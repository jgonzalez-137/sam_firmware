#pragma once

#include "../core/event_bus.h"

// Inicia task de deteccion de anomalias y gestiona FSM/salidas de alarma
bool start_anomaly_detector_task(core::EventBus* bus);
