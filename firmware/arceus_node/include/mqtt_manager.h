#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors_ina219.h"
#include "relays.h"
#include "pilots.h"

class MQTTManager {
public:
    MQTTManager(RelayController& relays, PilotController& pilots);
    void begin();
    void loop();
    bool isConnected();
    bool publishTelemetry(const ElectricalMetrics& metrics, const char* status);
    bool publishAlert(const char* type, const char* message, const char* severity);
    void handleIncomingCommand(char* topic, byte* payload, unsigned int length);

private:
    void reconnectWiFi();
    void reconnectMQTT();

    RelayController& m_relays;
    PilotController& m_pilots;
    WiFiClient m_espClient;
    PubSubClient m_mqttClient;
    unsigned long m_lastReconnectAttempt;
};
