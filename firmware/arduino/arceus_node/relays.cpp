#include "relays.h"

RelayController::RelayController()
    : m_relay1State(false), m_relay2State(false) {}

void RelayController::begin() {
    pinMode(PIN_RELAY_1, OUTPUT);
    pinMode(PIN_RELAY_2, OUTPUT);
    allOff();
    Serial.println(F("[RELAYS] Controlador de reles inicializado (Estado inicial: APAGADO)."));
}

void RelayController::setRelay(uint8_t relayIndex, bool turnOn) {
    uint8_t level = turnOn ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL;
    if (relayIndex == 1) {
        digitalWrite(PIN_RELAY_1, level);
        m_relay1State = turnOn;
        Serial.printf("[RELAY 1] %s\n", turnOn ? "ACTIVADO (ON)" : "DESACTIVADO (OFF)");
    } else if (relayIndex == 2) {
        digitalWrite(PIN_RELAY_2, level);
        m_relay2State = turnOn;
        Serial.printf("[RELAY 2] %s\n", turnOn ? "ACTIVADO (ON)" : "DESACTIVADO (OFF)");
    }
}

bool RelayController::getRelayState(uint8_t relayIndex) const {
    if (relayIndex == 1) return m_relay1State;
    if (relayIndex == 2) return m_relay2State;
    return false;
}

void RelayController::allOff() {
    digitalWrite(PIN_RELAY_1, RELAY_INACTIVE_LEVEL);
    digitalWrite(PIN_RELAY_2, RELAY_INACTIVE_LEVEL);
    m_relay1State = false;
    m_relay2State = false;
}
