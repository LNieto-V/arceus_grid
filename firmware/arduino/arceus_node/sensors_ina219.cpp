#include "sensors_ina219.h"

SensorINA219Manager::SensorINA219Manager()
    : m_ina219(0x40), m_healthy(false) {}

bool SensorINA219Manager::begin(uint8_t i2c_address) {
    m_ina219 = Adafruit_INA219(i2c_address);
    if (!m_ina219.begin()) {
        m_healthy = false;
        Serial.printf("[INA219 ERROR] No se encontro sensor INA219 en direccion 0x%02X\n", i2c_address);
        return false;
    }
    // Calibración por defecto: 32V, 2A o hasta 3.2A con shunt estándar de 0.1 ohm
    m_ina219.setCalibration_32V_2A();
    m_healthy = true;
    Serial.printf("[INA219] Sensor inicializado correctamente en 0x%02X\n", i2c_address);
    return true;
}

ElectricalMetrics SensorINA219Manager::readMetrics() {
    ElectricalMetrics m;
    if (!m_healthy) {
        m.busVoltage_V = 0.0f;
        m.shuntVoltage_mV = 0.0f;
        m.loadVoltage_V = 0.0f;
        m.current_A = 0.0f;
        m.power_W = 0.0f;
        m.isValid = false;
        return m;
    }

    m.shuntVoltage_mV = m_ina219.getShuntVoltage_mV();
    m.busVoltage_V = m_ina219.getBusVoltage_V();
    m.loadVoltage_V = m.busVoltage_V + (m.shuntVoltage_mV / 1000.0f);
    m.current_A = m_ina219.getCurrent_mA() / 1000.0f;
    m.power_W = m_ina219.getPower_mW() / 1000.0f;
    m.isValid = true;

    // Filtro para corrientes negativas espurias por offset del amplificador
    if (m.current_A < 0.005f && m.current_A > -0.015f) {
        m.current_A = 0.0f;
        m.power_W = 0.0f;
    }

    return m;
}
