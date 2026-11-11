#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sensors_ina219.h"
#include "pilots.h"

class DisplayOLEDManager {
public:
    DisplayOLEDManager();
    bool begin(uint8_t i2c_address = 0x3C);
    void render(char stationId,
                const ElectricalMetrics& metrics,
                bool relay1,
                bool relay2,
                bool mqttConnected,
                PilotMode pilotMode,
                const char* alertMsg = nullptr);

private:
    Adafruit_SSD1306 m_display;
    bool m_initialized;
};
