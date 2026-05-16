#pragma once

#include <cstdint>
#include "../i2c_bus/i2c_bus.h"

struct PpgSample {
    uint32_t ir;
    uint32_t red;
    bool valid;
};

class Max30102Driver {
public:
    Max30102Driver();
    bool init(I2cBus* bus);
    bool update();
    bool isConnected() const;
    PpgSample read() const;

private:
    I2cBus* bus_;
    bool initialized_;
    PpgSample last_;
};
