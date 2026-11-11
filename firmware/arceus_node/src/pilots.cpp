#include "pilots.h"

PilotController::PilotController()
    : m_mode(PILOT_MODE_NORMAL), m_lastBlinkMillis(0), m_blinkState(false) {}

void PilotController::begin() {
    pinMode(PIN_PILOT_GREEN, OUTPUT);
    pinMode(PIN_PILOT_RED, OUTPUT);
    pinMode(PIN_PILOT_YELLOW, OUTPUT);
    setMode(PILOT_MODE_NORMAL);
    Serial.println(F("[PILOTS] Controlador de pilotos 12V inicializado en modo NORMAL."));
}

void PilotController::applyOutputs(bool green, bool red, bool yellow) {
    digitalWrite(PIN_PILOT_GREEN, green ? HIGH : LOW);
    digitalWrite(PIN_PILOT_RED, red ? HIGH : LOW);
    digitalWrite(PIN_PILOT_YELLOW, yellow ? HIGH : LOW);
}

void PilotController::setMode(PilotMode mode) {
    m_mode = mode;
    m_lastBlinkMillis = millis();
    m_blinkState = true;

    if (m_mode == PILOT_MODE_OFF) {
        applyOutputs(false, false, false);
    } else if (m_mode == PILOT_MODE_NORMAL) {
        applyOutputs(true, false, false);
    } else if (m_mode == PILOT_MODE_WARNING) {
        applyOutputs(false, false, true);
    } else if (m_mode == PILOT_MODE_DANGER) {
        applyOutputs(false, true, false);
    }
}

void PilotController::setModeFromString(const String& modeStr) {
    if (modeStr.equalsIgnoreCase("NORMAL")) {
        setMode(PILOT_MODE_NORMAL);
    } else if (modeStr.equalsIgnoreCase("WARNING")) {
        setMode(PILOT_MODE_WARNING);
    } else if (modeStr.equalsIgnoreCase("DANGER")) {
        setMode(PILOT_MODE_DANGER);
    } else if (modeStr.equalsIgnoreCase("OFF")) {
        setMode(PILOT_MODE_OFF);
    }
}

void PilotController::update(unsigned long currentMillis) {
    if (m_mode == PILOT_MODE_WARNING) {
        // Intermitencia de 700 ms
        if (currentMillis - m_lastBlinkMillis >= 700) {
            m_lastBlinkMillis = currentMillis;
            m_blinkState = !m_blinkState;
            applyOutputs(false, false, m_blinkState);
        }
    } else if (m_mode == PILOT_MODE_DANGER) {
        // Intermitencia rapida de 200 ms
        if (currentMillis - m_lastBlinkMillis >= 200) {
            m_lastBlinkMillis = currentMillis;
            m_blinkState = !m_blinkState;
            applyOutputs(false, m_blinkState, false);
        }
    }
}
