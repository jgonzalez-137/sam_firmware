
#include "i2c_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "I2C_BUS";

I2cBus::I2cBus()
    : initialized_(false), sda_(21), scl_(22), freq_hz_(100000) {}

bool I2cBus::init(int sda, int scl, int freq_hz)
{
    sda_ = sda;
    scl_ = scl;
    freq_hz_ = freq_hz;

    i2c_config_t conf{};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)sda_;
    conf.scl_io_num = (gpio_num_t)scl_;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freq_hz_;

    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %d", err);
        return false;
    }

    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %d", err);
        return false;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "I2C init: SDA=%d SCL=%d Freq=%d", sda_, scl_, freq_hz_);
    return initialized_;
}

bool I2cBus::scan(std::vector<int>& devices)
{
    devices.clear();
    if (!initialized_) {
        ESP_LOGW(TAG, "scan() called but I2C not initialized");
        return false;
    }

    for (int addr = 0x03; addr <= 0x77; ++addr) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        if (ret == ESP_OK) {
            devices.push_back(addr);
        }
    }

    ESP_LOGI(TAG, "I2C scan finished, found %d device(s)", (int)devices.size());
    return true;
}

bool I2cBus::readRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out)
{
    if (!initialized_ || !out) return false;

    return readBytes(dev_addr, reg_addr, out, 1);
}

bool I2cBus::readBytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out, size_t len)
{
    if (!initialized_ || !out || len == 0) return false;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);

    if (len == 1) {
        i2c_master_read_byte(cmd, out, I2C_MASTER_NACK);
    } else {
        i2c_master_read(cmd, out, len, I2C_MASTER_LAST_NACK);
    }

    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(200));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "readBytes addr=0x%02x reg=0x%02x len=%d failed: %d", dev_addr, reg_addr, (int)len, ret);
        return false;
    }
    return true;
}

bool I2cBus::writeRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t value)
{
    if (!initialized_) return false;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "writeRegister addr=0x%02x reg=0x%02x failed: %d", dev_addr, reg_addr, ret);
        return false;
    }
    return true;
}

bool I2cBus::isInitialized() const { return initialized_; }
