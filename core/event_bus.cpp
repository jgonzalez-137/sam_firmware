#include "event_bus.h"
#include "esp_log.h"

static const char* TAG = "EVENT_BUS";

namespace core {

EventBus::EventBus() : q_(nullptr) {}

EventBus::~EventBus()
{
    if (q_) vQueueDelete(q_);
}

bool EventBus::init(size_t capacity)
{
    if (q_) return true;
    q_ = xQueueCreate((UBaseType_t)capacity, sizeof(Event));
    if (!q_) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return false;
    }
    ESP_LOGI(TAG, "EventBus initialized (capacity=%d)", (int)capacity);
    return true;
}

bool EventBus::publish(const Event& e, TickType_t ticks_to_wait)
{
    if (!q_) return false;
    BaseType_t ok = xQueueSend(q_, &e, ticks_to_wait);
    return ok == pdTRUE;
}

bool EventBus::consume(Event& out, TickType_t ticks_to_wait)
{
    if (!q_) return false;
    BaseType_t ok = xQueueReceive(q_, &out, ticks_to_wait);
    return ok == pdTRUE;
}

} // namespace core
