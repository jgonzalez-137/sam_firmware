#pragma once

#include "../drivers/mpu6050/mpu6050.h"
#include "../drivers/max30102/max30102.h"
#include "../core/event_bus.h"

// Crear y arrancar las tasks de sensores
bool start_sensor_tasks(Mpu6050Driver* imu, Max30102Driver* ppg, core::EventBus* bus);
