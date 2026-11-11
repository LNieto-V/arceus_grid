#pragma once

#include <Arduino.h>
#include "config.h"

class RelayController {
public:
    RelayController();
    void begin();
    void setRelay(uint8_t relayIndex, bool turnOn);
    bool getRelayState(uint8_t relayIndex) const;
    void allOff();

private:
    bool m_relay1State;
    bool m_relay2State;
};
