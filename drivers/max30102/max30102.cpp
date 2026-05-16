#include "max30102.h"
#include "esp_log.h"

static const char* TAG = "MAX30102";

Max30102Driver::Max30102Driver()
    : bus_(nullptr), initialized_(false)
{
    last_ = {0,0,false};
}

bool Max30102Driver::init(I2cBus* bus)
{
    bus_ = bus;
    if (!bus_ || !bus_->isInitialized()) {
        ESP_LOGW(TAG, "I2C bus not initialized for MAX30102");
        return false;
    }
    // Verificar presencia leyendo FIFO pointers
    uint8_t wr = 0, rd = 0;
    if (!bus_->readRegister(0x57, 0x04, &wr) || !bus_->readRegister(0x57, 0x06, &rd)) {
        ESP_LOGW(TAG, "MAX30102: no responde en 0x57 (no pudo leer pointers)");
        return false;
    }

    ESP_LOGI(TAG, "MAX30102 pointers WR=0x%02x RD=0x%02x", wr, rd);

    // Configuración básica: poner en modo SpO2 (MODE_REG = 0x09 <- 0x03)
    bus_->writeRegister(0x57, 0x09, 0x03);
    // FIFO configuración: samples to average = 4 (FIFO_CONFIG = 0x08)
    bus_->writeRegister(0x57, 0x08, 0x4); // value chosen as safe default

    initialized_ = true;
    ESP_LOGI(TAG, "MAX30102 init OK");
    return initialized_;
}

bool Max30102Driver::update()
{
    if (!initialized_) return false;

    uint8_t wr = 0, rd = 0;
    if (!bus_->readRegister(0x57, 0x04, &wr) || !bus_->readRegister(0x57, 0x06, &rd)) {
        ESP_LOGW(TAG, "MAX30102: failed to read FIFO pointers");
        last_.valid = false;
        return false;
    }

    int count = (int)( (wr - rd) & 0x1F );
    if (count <= 0) {
        last_.valid = false;
        return true; // no new samples
    }

    // Leer solo una muestra (6 bytes: 3 IR + 3 RED)
    uint8_t buf[6];
    if (!bus_->readBytes(0x57, 0x07, buf, sizeof(buf))) {
        ESP_LOGW(TAG, "MAX30102: failed to read FIFO_DATA");
        last_.valid = false;
        return false;
    }

    auto to_18 = [](uint8_t b0, uint8_t b1, uint8_t b2) -> uint32_t {
        uint32_t v = ((uint32_t)b0 << 16) | ((uint32_t)b1 << 8) | (uint32_t)b2;
        return v & 0x3FFFF; // 18-bit
    };

    uint32_t ir = to_18(buf[0], buf[1], buf[2]);
    uint32_t red = to_18(buf[3], buf[4], buf[5]);

    last_.ir = ir;
    last_.red = red;
    last_.valid = true;
    return true;
}

bool Max30102Driver::isConnected() const { return initialized_; }

PpgSample Max30102Driver::read() const { return last_; }
