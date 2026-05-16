#include "alarm_fsm.h"
#include "esp_log.h"

static const char* TAG = "FSM";

namespace core {

static const char* stateName(AlarmState s)
{
    switch (s) {
        case AlarmState::NORMAL: return "NORMAL";
        case AlarmState::PRE_ALERT: return "PRE_ALERT";
        case AlarmState::ALERT_ACTIVE: return "ALERT_ACTIVE";
        case AlarmState::ALERT_HOLD: return "ALERT_HOLD";
        case AlarmState::FAULT_SENSOR: return "FAULT_SENSOR";
        default: return "UNKNOWN";
    }
}

AlarmFsm::AlarmFsm() : state_(AlarmState::NORMAL), type_(AlarmType::NONE), state_since_ms_(0) {}

void AlarmFsm::transitionTo(AlarmState new_state, uint32_t now_ms, const char* reason)
{
    if (new_state == state_) return;
    ESP_LOGI(TAG, "[FSM] %s -> %s (%s)", stateName(state_), stateName(new_state), reason);
    state_ = new_state;
    state_since_ms_ = now_ms;
}

void AlarmFsm::update(bool sensor_fault, bool anomaly_active, AlarmType anomaly_type, uint32_t now_ms)
{
    if (sensor_fault) {
        type_ = AlarmType::SENSOR_FAULT;
        transitionTo(AlarmState::FAULT_SENSOR, now_ms, "sensor fault");
        return;
    }

    switch (state_) {
        case AlarmState::NORMAL:
            if (anomaly_active) {
                type_ = anomaly_type;
                transitionTo(AlarmState::PRE_ALERT, now_ms, "anomaly detected");
            }
            break;

        case AlarmState::PRE_ALERT:
            if (anomaly_active) {
                type_ = anomaly_type;
                if ((now_ms - state_since_ms_) >= 500U) {
                    transitionTo(AlarmState::ALERT_ACTIVE, now_ms, "confirmed anomaly");
                }
            } else {
                type_ = AlarmType::NONE;
                transitionTo(AlarmState::NORMAL, now_ms, "anomaly cleared");
            }
            break;

        case AlarmState::ALERT_ACTIVE:
            if (!anomaly_active) {
                transitionTo(AlarmState::ALERT_HOLD, now_ms, "alarm hold");
            }
            break;

        case AlarmState::ALERT_HOLD:
            if (anomaly_active) {
                type_ = anomaly_type;
                transitionTo(AlarmState::ALERT_ACTIVE, now_ms, "anomaly resumed");
            } else if ((now_ms - state_since_ms_) >= 2000U) {
                type_ = AlarmType::NONE;
                transitionTo(AlarmState::NORMAL, now_ms, "hold elapsed");
            }
            break;

        case AlarmState::FAULT_SENSOR:
            if (!sensor_fault) {
                type_ = AlarmType::NONE;
                transitionTo(AlarmState::NORMAL, now_ms, "sensor recovered");
            }
            break;
    }
}

AlarmState AlarmFsm::getState() const { return state_; }

AlarmType AlarmFsm::getAlarmType() const { return type_; }

} // namespace core
