#include "mpu6050.h"
#include "esp_log.h"

static const char* TAG = "MPU6050";

Mpu6050Driver::Mpu6050Driver()
    : bus_(nullptr), initialized_(false)
{
    last_ = {0};
}

bool Mpu6050Driver::init(I2cBus* bus)
{
    bus_ = bus;
    if (!bus_ || !bus_->isInitialized()) {
        ESP_LOGW(TAG, "I2C bus not initialized for MPU6050");
        return false;
    }
    // Leer WHO_AM_I (registro 0x75) y verificar valor (esperado 0x68)
    uint8_t who = 0;
    if (!bus_->readRegister(0x68, 0x75, &who)) {
        ESP_LOGW(TAG, "MPU6050: failed to read WHO_AM_I");
        return false;
    }
    ESP_LOGI(TAG, "MPU6050 WHO_AM_I=0x%02x", who);
    if (who != 0x68) {
        ESP_LOGW(TAG, "MPU6050: unexpected WHO_AM_I value");
        // No asumir fallo absoluto; marcar no inicializado
        initialized_ = false;
        return false;
    }

    // Configuración mínima si aplica: salir del sleep (PWR_MGMT_1 = 0x6B <- 0x00)
    bus_->writeRegister(0x68, 0x6B, 0x00);

    initialized_ = true;
    ESP_LOGI(TAG, "MPU6050 init OK");
    return initialized_;
}

bool Mpu6050Driver::update()
{
    if (!initialized_) return false;

    uint8_t buf[14];
    // ACCEL_XOUT_H = 0x3B, lectura de 14 bytes: accel(6), temp(2), gyro(6)
    if (!bus_->readBytes(0x68, 0x3B, buf, sizeof(buf))) {
        ESP_LOGW(TAG, "MPU6050: failed to read sensor registers");
        last_.valid = false;
        return false;
    }

    auto to_s16 = [](uint8_t hi, uint8_t lo) -> int16_t { return (int16_t)((hi << 8) | lo); };

    int16_t ax = to_s16(buf[0], buf[1]);
    int16_t ay = to_s16(buf[2], buf[3]);
    int16_t az = to_s16(buf[4], buf[5]);
    int16_t gx = to_s16(buf[8], buf[9]);
    int16_t gy = to_s16(buf[10], buf[11]);
    int16_t gz = to_s16(buf[12], buf[13]);

    // Escalas por defecto: accel ±2g => 16384 LSB/g ; gyro ±250 dps => 131 LSB/dps
    const float A_SF = 16384.0f;
    const float G_SF = 131.0f;

    last_.ax = (float)ax / A_SF;
    last_.ay = (float)ay / A_SF;
    last_.az = (float)az / A_SF;
    last_.gx = (float)gx / G_SF;
    last_.gy = (float)gy / G_SF;
    last_.gz = (float)gz / G_SF;
    last_.valid = true;
    return true;
}

bool Mpu6050Driver::isConnected() const { return initialized_; }

ImuSample Mpu6050Driver::read() const { return last_; }
