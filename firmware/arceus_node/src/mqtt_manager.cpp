#include "mqtt_manager.h"

static MQTTManager* s_instance = nullptr;

static void staticMqttCallback(char* topic, byte* payload, unsigned int length) {
    if (s_instance) {
        s_instance->handleIncomingCommand(topic, payload, length);
    }
}

MQTTManager::MQTTManager(RelayController& relays, PilotController& pilots)
    : m_relays(relays), m_pilots(pilots), m_mqttClient(m_espClient), m_lastReconnectAttempt(0) {
    s_instance = this;
}

void MQTTManager::begin() {
    m_mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    m_mqttClient.setCallback(staticMqttCallback);
    m_mqttClient.setBufferSize(512);
    reconnectWiFi();
}

void MQTTManager::reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    Serial.printf("[WIFI] Conectando a %s...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void MQTTManager::reconnectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        reconnectWiFi();
        return;
    }

    if (m_mqttClient.connected()) return;

    unsigned long now = millis();
    if (now - m_lastReconnectAttempt > 5000) {
        m_lastReconnectAttempt = now;
        Serial.printf("[MQTT] Intentando conexion al broker %s:%d...\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);

        String clientId = "ArceusNode_" + String(STATION_NAME) + "_" + String(random(0xffff), HEX);
        const char* willTopic = MQTT_TOPIC_STATUS;
        const char* willMsg = "{\"status\": \"OFFLINE\"}";

        if (m_mqttClient.connect(clientId.c_str(), willTopic, 1, true, willMsg)) {
            Serial.println(F("[MQTT] Conectado exitosamente."));
            // Publicar estado ONLINE con retencion
            m_mqttClient.publish(willTopic, "{\"status\": \"ONLINE\"}", true);
            // Suscribirse al canal de comandos
            m_mqttClient.subscribe(MQTT_TOPIC_COMMANDS, 1);
            Serial.printf("[MQTT] Suscrito al topico de comandos: %s\n", MQTT_TOPIC_COMMANDS);
        } else {
            Serial.printf("[MQTT ERROR] Fallo de conexion, rc=%d. Reintento en 5s.\n", m_mqttClient.state());
        }
    }
}

void MQTTManager::loop() {
    if (!m_mqttClient.connected()) {
        reconnectMQTT();
    } else {
        m_mqttClient.loop();
    }
}

bool MQTTManager::isConnected() {
    return m_mqttClient.connected();
}

bool MQTTManager::publishTelemetry(const ElectricalMetrics& metrics, const char* status) {
    if (!m_mqttClient.connected()) return false;

    StaticJsonDocument<256> doc;
    doc["station"] = String(STATION_ID_CHAR);
    doc["voltage"] = serialized(String(metrics.busVoltage_V, 2));
    doc["current"] = serialized(String(metrics.current_A, 3));
    doc["power"] = serialized(String(metrics.power_W, 2));
    doc["relay_1"] = m_relays.getRelayState(1);
    doc["relay_2"] = m_relays.getRelayState(2);
    doc["status"] = status;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    return m_mqttClient.publish(MQTT_TOPIC_TELEMETRY, (const uint8_t*)buffer, n, false);
}

bool MQTTManager::publishAlert(const char* type, const char* message, const char* severity) {
    if (!m_mqttClient.connected()) return false;

    StaticJsonDocument<256> doc;
    doc["station"] = String(STATION_ID_CHAR);
    doc["type"] = type;
    doc["message"] = message;
    doc["severity"] = severity;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    return m_mqttClient.publish(MQTT_TOPIC_ALERTS, (const uint8_t*)buffer, n, false);
}

void MQTTManager::handleIncomingCommand(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.printf("[MQTT CMD] Error parseando JSON: %s\n", error.c_str());
        return;
    }

    const char* command = doc["command"] | "";
    Serial.printf("[MQTT CMD] Comando recibido: %s\n", command);

    if (strcmp(command, "SET_RELAY") == 0) {
        int relay = doc["relay"] | 0;
        bool state = doc["state"] | false;
        if (relay == 1 || relay == 2) {
            m_relays.setRelay(relay, state);
        }
    } else if (strcmp(command, "SET_PILOT") == 0) {
        const char* mode = doc["mode"] | "NORMAL";
        m_pilots.setModeFromString(String(mode));
    }
}
