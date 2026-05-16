#pragma once

#include <cstdint>
#include "../i2c_bus/i2c_bus.h"

struct ImuSample {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    bool valid;
};

class Mpu6050Driver {
public:
    Mpu6050Driver();
    bool init(I2cBus* bus);
    bool update();
    bool isConnected() const;
    ImuSample read() const;

private:
    I2cBus* bus_;
    bool initialized_;
    ImuSample last_;
};
