#pragma once

#include <Arduino.h>
#include <Adafruit_INA219.h>

struct ElectricalMetrics {
    float busVoltage_V;
    float shuntVoltage_mV;
    float loadVoltage_V;
    float current_A;
    float power_W;
    bool isValid;
};

class SensorINA219Manager {
public:
    SensorINA219Manager();
    bool begin(uint8_t i2c_address = 0x40);
    ElectricalMetrics readMetrics();
    bool isHealthy() const { return m_healthy; }

private:
    Adafruit_INA219 m_ina219;
    bool m_healthy;
};
