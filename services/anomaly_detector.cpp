#include "anomaly_detector.h"

#include "../config/anomaly_thresholds.h"
#include "../core/alarm_fsm.h"
#include "alarm_output.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <math.h>

static const char* TAG = "ANOMALY";

struct DetectorCtx {
    core::EventBus* bus;
    core::AlarmFsm fsm;
    AlarmOutput output;

    uint32_t last_ppg_ms;
    uint32_t last_beat_ms;
    uint32_t brady_since_ms;
    uint32_t tachy_since_ms;

    bool ppg_prev_high;
    float bpm;
    float accel_mag;
    bool sensor_fault;
};

static uint32_t nowMs()
{
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static void processImu(DetectorCtx* c, const core::ImuSampleRaw& s)
{
    if (!s.valid) return;
    c->accel_mag = sqrtf((s.ax * s.ax) + (s.ay * s.ay) + (s.az * s.az));
}

static void processPpg(DetectorCtx* c, const core::PpgSampleRaw& s, uint32_t ms)
{
    if (!s.valid) return;
    c->last_ppg_ms = ms;

    bool high = s.ir >= config::PPG_IR_BEAT_THRESHOLD;
    if (high && !c->ppg_prev_high) {
        if (c->last_beat_ms > 0U) {
            uint32_t dt = ms - c->last_beat_ms;
            if (dt >= config::BEAT_REFRACTORY_MS) {
                c->bpm = 60000.0f / (float)dt;
                c->last_beat_ms = ms;
            }
        } else {
            c->last_beat_ms = ms;
        }
    }
    c->ppg_prev_high = high;
}

static core::AlarmType evaluateAnomalies(DetectorCtx* c, uint32_t ms)
{
    bool accel_anomaly = c->accel_mag > config::ACCEL_ALERT_G;
    bool hr_pause = (c->last_beat_ms > 0U) && ((ms - c->last_beat_ms) > config::HR_PAUSE_MS);

    bool brady = false;
    bool tachy = false;
    if (c->bpm > 0.0f) {
        if (c->bpm < config::BRADY_BPM) {
            if (c->brady_since_ms == 0U) c->brady_since_ms = ms;
            if ((ms - c->brady_since_ms) >= config::HR_SUSTAIN_MS) brady = true;
        } else {
            c->brady_since_ms = 0U;
        }

        if (c->bpm > config::TACHY_BPM) {
            if (c->tachy_since_ms == 0U) c->tachy_since_ms = ms;
            if ((ms - c->tachy_since_ms) >= config::HR_SUSTAIN_MS) tachy = true;
        } else {
            c->tachy_since_ms = 0U;
        }
    }

    if (accel_anomaly) return core::AlarmType::ACCEL;
    if (hr_pause) return core::AlarmType::HR_PAUSE;
    if (brady) return core::AlarmType::BRADY;
    if (tachy) return core::AlarmType::TACHY;
    return core::AlarmType::NONE;
}

static void anomaly_task(void* pv)
{
    DetectorCtx* c = static_cast<DetectorCtx*>(pv);
    core::Event ev{};

    while (true) {
        uint32_t ms = nowMs();

        if (c->bus->consume(ev, pdMS_TO_TICKS(20))) {
            if (ev.type == core::EventType::IMU_SAMPLE) {
                processImu(c, ev.payload.imu);
            } else if (ev.type == core::EventType::PPG_SAMPLE) {
                processPpg(c, ev.payload.ppg, ms);
            }
        }

        // Sensor fault si no hay datos PPG recientes
        c->sensor_fault = (c->last_ppg_ms > 0U) && ((ms - c->last_ppg_ms) > 4000U);
        core::AlarmType type = evaluateAnomalies(c, ms);
        bool anomaly_active = type != core::AlarmType::NONE;

        c->fsm.update(c->sensor_fault, anomaly_active, type, ms);
        c->output.update(c->fsm.getState(), c->fsm.getAlarmType(), ms);

        if (anomaly_active) {
            ESP_LOGW(TAG, "[EVENT] anomaly=%d accel=%.2f bpm=%.1f", (int)type, c->accel_mag, c->bpm);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

bool start_anomaly_detector_task(core::EventBus* bus)
{
    if (!bus) return false;

    static DetectorCtx ctx{};
    ctx.bus = bus;
    ctx.last_ppg_ms = 0;
    ctx.last_beat_ms = 0;
    ctx.brady_since_ms = 0;
    ctx.tachy_since_ms = 0;
    ctx.ppg_prev_high = false;
    ctx.bpm = 0.0f;
    ctx.accel_mag = 1.0f;
    ctx.sensor_fault = false;

    if (!ctx.output.init()) {
        ESP_LOGE(TAG, "AlarmOutput init failed");
        return false;
    }

    BaseType_t r = xTaskCreate(anomaly_task, "anomaly_task", 6144, &ctx, 6, nullptr);
    if (r != pdPASS) {
        ESP_LOGE(TAG, "Failed to create anomaly task");
        return false;
    }

    ESP_LOGI(TAG, "Anomaly detector task started");
    return true;
}
