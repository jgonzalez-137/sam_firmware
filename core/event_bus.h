#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <cstdint>

namespace core {

enum class EventType : uint8_t {
    IMU_SAMPLE,
    PPG_SAMPLE,
};

struct ImuSampleRaw {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    bool valid;
};

struct PpgSampleRaw {
    uint32_t ir;
    uint32_t red;
    bool valid;
};

struct Event {
    EventType type;
    union {
        ImuSampleRaw imu;
        PpgSampleRaw ppg;
    } payload;
};

class EventBus {
public:
    EventBus();
    ~EventBus();

    // Inicializa la cola con tamaño 'capacity'
    bool init(size_t capacity = 16);

    // Publica un evento (bloqueante con timeout)
    bool publish(const Event& e, TickType_t ticks_to_wait = portMAX_DELAY);

    // Consume un evento; espera hasta 'ticks_to_wait'
    bool consume(Event& out, TickType_t ticks_to_wait = portMAX_DELAY);

private:
    QueueHandle_t q_;
};

} // namespace core
