#pragma once

#include <stdint.h>

namespace core {

enum class AlarmType : uint8_t {
    NONE,
    ACCEL,
    HR_PAUSE,
    BRADY,
    TACHY,
    SENSOR_FAULT,
};

enum class AlarmState : uint8_t {
    NORMAL,
    PRE_ALERT,
    ALERT_ACTIVE,
    ALERT_HOLD,
    FAULT_SENSOR,
};

class AlarmFsm {
public:
    AlarmFsm();

    // Actualiza estado FSM segun inputs ya procesados
    void update(bool sensor_fault, bool anomaly_active, AlarmType anomaly_type, uint32_t now_ms);

    AlarmState getState() const;
    AlarmType getAlarmType() const;

private:
    void transitionTo(AlarmState new_state, uint32_t now_ms, const char* reason);

    AlarmState state_;
    AlarmType type_;
    uint32_t state_since_ms_;
};

} // namespace core
