#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "SAM";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Sistema SAM iniciado");

    while (true)
    {
        ESP_LOGI(TAG, "Running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}