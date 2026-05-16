#pragma once

#include <vector>
#include <stdint.h>

#include "driver/i2c.h"

class I2cBus {
public:
    I2cBus();
    // Inicializa el bus I2C con pines y frecuencia (defaults para el proyecto)
    bool init(int sda = 21, int scl = 22, int freq_hz = 100000);

    // Escanea dispositivos en el bus; llena 'devices' con direcciones (0-127)
    // Esta implementación es un scaffold: realiza logs y devuelve true si init fue llamada.
    bool scan(std::vector<int>& devices);
    // Leer un registro de 8 bits desde un dispositivo I2C
    bool readRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out);
    bool writeRegister(uint8_t dev_addr, uint8_t reg_addr, uint8_t value);
    // Leer múltiples bytes a partir de un registro (secuencia)
    bool readBytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t* out, size_t len);

    bool isInitialized() const;

private:
    bool initialized_;
    int sda_;
    int scl_;
    int freq_hz_;
};
