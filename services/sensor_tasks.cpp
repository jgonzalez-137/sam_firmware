#include "sensor_tasks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "SENSOR_TASKS";

struct TaskParams {
    Mpu6050Driver* imu;
    Max30102Driver* ppg;
    core::EventBus* bus;
};

static void imu_task(void* pv)
{
    TaskParams* p = (TaskParams*)pv;
    const TickType_t period = pdMS_TO_TICKS(10); // 100 Hz
    while (true) {
        if (p->imu && p->imu->isConnected()) {
            p->imu->update();
            ImuSample s = p->imu->read();
            core::Event e{};
            e.type = core::EventType::IMU_SAMPLE;
            e.payload.imu.ax = s.ax;
            e.payload.imu.ay = s.ay;
            e.payload.imu.az = s.az;
            e.payload.imu.gx = s.gx;
            e.payload.imu.gy = s.gy;
            e.payload.imu.gz = s.gz;
            e.payload.imu.valid = s.valid;
            p->bus->publish(e, 0);
        }
        vTaskDelay(period);
    }
}

static void ppg_task(void* pv)
{
    TaskParams* p = (TaskParams*)pv;
    const TickType_t period = pdMS_TO_TICKS(20); // 50 Hz
    while (true) {
        if (p->ppg && p->ppg->isConnected()) {
            p->ppg->update();
            PpgSample s = p->ppg->read();
            core::Event e{};
            e.type = core::EventType::PPG_SAMPLE;
            e.payload.ppg.ir = s.ir;
            e.payload.ppg.red = s.red;
            e.payload.ppg.valid = s.valid;
            p->bus->publish(e, 0);
        }
        vTaskDelay(period);
    }
}

bool start_sensor_tasks(Mpu6050Driver* imu, Max30102Driver* ppg, core::EventBus* bus)
{
    if (!bus) return false;
    TaskParams* params = new TaskParams();
    params->imu = imu;
    params->ppg = ppg;
    params->bus = bus;

    BaseType_t r1 = xTaskCreate(imu_task, "imu_task", 4096, params, 5, nullptr);
    BaseType_t r2 = xTaskCreate(ppg_task, "ppg_task", 4096, params, 5, nullptr);
    if (r1 != pdPASS || r2 != pdPASS) {
        ESP_LOGE(TAG, "Failed to create sensor tasks");
        return false;
    }
    ESP_LOGI(TAG, "Sensor tasks started");
    return true;
}
