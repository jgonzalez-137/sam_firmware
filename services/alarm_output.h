#pragma once

#include "../core/alarm_fsm.h"
#include <stdint.h>

class AlarmOutput {
public:
    bool init(int buzzer_gpio = 13, int led_r_gpio = 14, int led_g_gpio = 19, int led_b_gpio = 27);
    void update(core::AlarmState state, core::AlarmType type, uint32_t now_ms);

private:
    void setLed(bool r, bool g, bool b);
    void buzzerOn();
    void buzzerOff();

    bool initialized_;
    int buzzer_gpio_;
    int led_r_gpio_;
    int led_g_gpio_;
    int led_b_gpio_;
};
