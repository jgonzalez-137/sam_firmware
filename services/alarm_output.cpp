#include "alarm_output.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "ALARM_OUTPUT";

bool AlarmOutput::init(int buzzer_gpio, int led_r_gpio, int led_g_gpio, int led_b_gpio)
{
    buzzer_gpio_ = buzzer_gpio;
    led_r_gpio_ = led_r_gpio;
    led_g_gpio_ = led_g_gpio;
    led_b_gpio_ = led_b_gpio;

    gpio_set_direction((gpio_num_t)led_r_gpio_, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)led_g_gpio_, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)led_b_gpio_, GPIO_MODE_OUTPUT);

    // LED de anodo comun: HIGH apaga, LOW enciende
    setLed(false, false, false);

    ledc_timer_config_t tcfg{};
    tcfg.speed_mode = LEDC_LOW_SPEED_MODE;
    tcfg.duty_resolution = LEDC_TIMER_10_BIT;
    tcfg.timer_num = LEDC_TIMER_0;
    tcfg.freq_hz = 3800;
    tcfg.clk_cfg = LEDC_AUTO_CLK;
    if (ledc_timer_config(&tcfg) != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed");
        return false;
    }

    ledc_channel_config_t ccfg{};
    ccfg.gpio_num = buzzer_gpio_;
    ccfg.speed_mode = LEDC_LOW_SPEED_MODE;
    ccfg.channel = LEDC_CHANNEL_0;
    ccfg.intr_type = LEDC_INTR_DISABLE;
    ccfg.timer_sel = LEDC_TIMER_0;
    ccfg.duty = 0;
    ccfg.hpoint = 0;
    if (ledc_channel_config(&ccfg) != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed");
        return false;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "AlarmOutput initialized");
    return true;
}

void AlarmOutput::setLed(bool r, bool g, bool b)
{
    gpio_set_level((gpio_num_t)led_r_gpio_, r ? 0 : 1);
    gpio_set_level((gpio_num_t)led_g_gpio_, g ? 0 : 1);
    gpio_set_level((gpio_num_t)led_b_gpio_, b ? 0 : 1);
}

void AlarmOutput::buzzerOn()
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void AlarmOutput::buzzerOff()
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void AlarmOutput::update(core::AlarmState state, core::AlarmType type, uint32_t now_ms)
{
    if (!initialized_) return;

    bool blink = ((now_ms / 250U) % 2U) != 0U;
    bool slow_blink = ((now_ms / 600U) % 2U) != 0U;

    switch (state) {
        case core::AlarmState::NORMAL:
            setLed(false, true, false);
            buzzerOff();
            break;

        case core::AlarmState::PRE_ALERT:
            setLed(false, false, true);
            if ((now_ms % 1200U) < 80U) buzzerOn(); else buzzerOff();
            break;

        case core::AlarmState::ALERT_ACTIVE:
            if (type == core::AlarmType::HR_PAUSE) {
                setLed(true, false, true);
            } else {
                setLed(true, false, false);
            }
            if (blink) buzzerOn(); else buzzerOff();
            break;

        case core::AlarmState::ALERT_HOLD:
            setLed(true, true, false);
            if (slow_blink) buzzerOn(); else buzzerOff();
            break;

        case core::AlarmState::FAULT_SENSOR:
            setLed(false, false, true);
            if (slow_blink) buzzerOn(); else buzzerOff();
            break;
    }
}
