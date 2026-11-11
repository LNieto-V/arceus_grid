#pragma once

#include <Arduino.h>
#include "config.h"

enum PilotMode {
    PILOT_MODE_OFF,
    PILOT_MODE_NORMAL,   // Verde fijo
    PILOT_MODE_WARNING,  // Amarillo intermitente (700 ms)
    PILOT_MODE_DANGER    // Rojo intermitente rapido (200 ms)
};

class PilotController {
public:
    PilotController();
    void begin();
    void setMode(PilotMode mode);
    void setModeFromString(const String& modeStr);
    PilotMode getMode() const { return m_mode; }
    void update(unsigned long currentMillis);

private:
    void applyOutputs(bool green, bool red, bool yellow);

    PilotMode m_mode;
    unsigned long m_lastBlinkMillis;
    bool m_blinkState;
};
