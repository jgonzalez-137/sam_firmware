#include <stdio.h>
#include <vector>
#include "drivers/i2c_bus/i2c_bus.h"
#include "esp_log.h"

static const char* TAG = "TEST_I2C_SCANNER";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting I2C scanner test (scaffold)");
    I2cBus bus;
    if (!bus.init(21, 22, 100000)) {
        ESP_LOGE(TAG, "I2C init failed (scaffold)");
        return;
    }

    std::vector<int> devices;
    if (!bus.scan(devices)) {
        ESP_LOGW(TAG, "I2C scan returned false (stub)");
    } else {
        ESP_LOGI(TAG, "I2C scan completed (stub). Detected %d devices", (int)devices.size());
        for (int addr : devices) {
            ESP_LOGI(TAG, " device: 0x%02x", addr);
        }
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
