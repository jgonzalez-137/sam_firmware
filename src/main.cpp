#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "drivers/i2c_bus/i2c_bus.h"
#include "drivers/mpu6050/mpu6050.h"
#include "drivers/max30102/max30102.h"
#include "drivers/display_gc9a01/display_gc9a01.h"
#include "core/event_bus.h"
#include "services/sensor_tasks.h"

static const char *TAG = "SAM";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Sistema SAM iniciado");

    // Inicializar bus I2C
    I2cBus bus;
    if (!bus.init(21, 22, 100000)) {
        ESP_LOGE(TAG, "I2C bus init failed");
    }

    // Inicializar drivers
    Mpu6050Driver imu;
    if (!imu.init(&bus)) {
        ESP_LOGW(TAG, "MPU6050 init failed or not present");
    }

    Max30102Driver ppg;
    if (!ppg.init(&bus)) {
        ESP_LOGW(TAG, "MAX30102 init failed or not present");
    }

    // Inicializar display (scaffold)
    Gc9a01 display;
    display.init(23, 18, 5, 2, 4);

    // Event bus
    core::EventBus bus_events;
    bus_events.init(32);

    // Start sensor tasks
    if (!start_sensor_tasks(&imu, &ppg, &bus_events)) {
        ESP_LOGW(TAG, "Failed to start sensor tasks");
    }

    // Consumidor simple: loguear eventos
    core::Event ev;
    while (true) {
        if (bus_events.consume(ev, pdMS_TO_TICKS(1000))) {
            switch (ev.type) {
                case core::EventType::IMU_SAMPLE:
                    ESP_LOGI(TAG, "IMU: ax=%.3f ay=%.3f az=%.3f", ev.payload.imu.ax, ev.payload.imu.ay, ev.payload.imu.az);
                    break;
                case core::EventType::PPG_SAMPLE:
                    ESP_LOGI(TAG, "PPG: IR=%u RED=%u", (unsigned)ev.payload.ppg.ir, (unsigned)ev.payload.ppg.red);
                    break;
                default:
                    break;
            }
        }
    }
}