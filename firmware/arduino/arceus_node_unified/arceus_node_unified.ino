/**
 * ====================================================================
 * ARCEUS GRID — Firmware Completo de Nodo (Todo en Uno para Arduino IDE)
 * Sistema de Monitoreo Eléctrico y AIoT
 * Compatible con: ESP32 DevKit V1 (o DOIT ESP32 DevKit)
 * ====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ====================================================================
// 1. CONFIGURACIÓN Y PARÁMETROS DE RED
// ====================================================================
#define STATION_ID_CHAR 'A'          // 'A' para Estación A, 'B' para Estación B
#define STATION_NAME    "station_a"  // "station_a" o "station_b"

// Credenciales Wi-Fi (Punto de acceso iPhone)
#define WIFI_SSID       "iPhone Owen"
#define WIFI_PASSWORD   "0864297531"

// Broker MQTT (PC anfitrión corriendo Docker)
#define MQTT_BROKER_IP   "172.20.10.2"
#define MQTT_BROKER_PORT 1884

// Tópicos MQTT
#define MQTT_TOPIC_TELEMETRY "arceus/" STATION_NAME "/telemetry"
#define MQTT_TOPIC_STATUS    "arceus/" STATION_NAME "/status"
#define MQTT_TOPIC_COMMANDS  "arceus/" STATION_NAME "/commands"
#define MQTT_TOPIC_ALERTS    "arceus/" STATION_NAME "/alerts"

// Pines GPIO del ESP32
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

#define PIN_RELAY_1 25
#define PIN_RELAY_2 26
#define RELAY_ACTIVE_LEVEL   LOW
#define RELAY_INACTIVE_LEVEL HIGH

#define PIN_PILOT_GREEN  27  // Estado Normal (12V)
#define PIN_PILOT_RED    32  // Alarma / Peligro (12V)
#define PIN_PILOT_YELLOW 33  // Advertencia (12V)

#define I2C_ADDR_OLED   0x3C
#define I2C_ADDR_INA219 0x40

// Intervalos de temporización no bloqueante (millis)
#define INTERVAL_TELEMETRY_MS 1000  // Envío MQTT cada 1 segundo
#define INTERVAL_DISPLAY_MS    300  // Refresco de pantalla cada 300 ms
#define INTERVAL_SENSOR_MS     200  // Muestreo del sensor cada 200 ms

// Umbrales locales de seguridad eléctrica
#define LOCAL_VOLTAGE_MIN_V 10.5f
#define LOCAL_VOLTAGE_MAX_V 13.5f
#define LOCAL_CURRENT_MAX_A  2.8f

// ====================================================================
// 2. ESTRUCTURAS Y CONTROLADOR DE SENSORES (INA219)
// ====================================================================
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
    SensorINA219Manager() : m_ina219(0x40), m_healthy(false) {}

    bool begin(uint8_t i2c_address = 0x40) {
        m_ina219 = Adafruit_INA219(i2c_address);
        if (!m_ina219.begin()) {
            m_healthy = false;
            Serial.printf("[INA219 ERROR] No se encontro sensor INA219 en 0x%02X\n", i2c_address);
            return false;
        }
        m_ina219.setCalibration_32V_2A();
        m_healthy = true;
        Serial.printf("[INA219] Sensor inicializado en 0x%02X\n", i2c_address);
        return true;
    }

    ElectricalMetrics readMetrics() {
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

        if (m.current_A < 0.005f && m.current_A > -0.015f) {
            m.current_A = 0.0f;
            m.power_W = 0.0f;
        }
        return m;
    }

private:
    Adafruit_INA219 m_ina219;
    bool m_healthy;
};

// ====================================================================
// 3. CONTROLADOR DE RELÉS
// ====================================================================
class RelayController {
public:
    RelayController() : m_relay1State(false), m_relay2State(false) {}

    void begin() {
        pinMode(PIN_RELAY_1, OUTPUT);
        pinMode(PIN_RELAY_2, OUTPUT);
        allOff();
        Serial.println(F("[RELAYS] Controlador de reles inicializado (APAGADOS)."));
    }

    void setRelay(uint8_t relayIndex, bool turnOn) {
        uint8_t level = turnOn ? RELAY_ACTIVE_LEVEL : RELAY_INACTIVE_LEVEL;
        if (relayIndex == 1) {
            digitalWrite(PIN_RELAY_1, level);
            m_relay1State = turnOn;
            Serial.printf("[RELAY 1] %s\n", turnOn ? "ON" : "OFF");
        } else if (relayIndex == 2) {
            digitalWrite(PIN_RELAY_2, level);
            m_relay2State = turnOn;
            Serial.printf("[RELAY 2] %s\n", turnOn ? "ON" : "OFF");
        }
    }

    bool getRelayState(uint8_t relayIndex) const {
        if (relayIndex == 1) return m_relay1State;
        if (relayIndex == 2) return m_relay2State;
        return false;
    }

    void allOff() {
        digitalWrite(PIN_RELAY_1, RELAY_INACTIVE_LEVEL);
        digitalWrite(PIN_RELAY_2, RELAY_INACTIVE_LEVEL);
        m_relay1State = false;
        m_relay2State = false;
    }

private:
    bool m_relay1State;
    bool m_relay2State;
};

// ====================================================================
// 4. CONTROLADOR DE PILOTOS 12V (MÁQUINA DE ESTADOS MILLIS)
// ====================================================================
enum PilotMode {
    PILOT_MODE_OFF,
    PILOT_MODE_NORMAL,   // Verde fijo
    PILOT_MODE_WARNING,  // Amarillo intermitente (700 ms)
    PILOT_MODE_DANGER    // Rojo intermitente rapido (200 ms)
};

class PilotController {
public:
    PilotController() : m_mode(PILOT_MODE_NORMAL), m_lastBlinkMillis(0), m_blinkState(false) {}

    void begin() {
        pinMode(PIN_PILOT_GREEN, OUTPUT);
        pinMode(PIN_PILOT_RED, OUTPUT);
        pinMode(PIN_PILOT_YELLOW, OUTPUT);
        setMode(PILOT_MODE_NORMAL);
        Serial.println(F("[PILOTS] Controlador de pilotos inicializado (NORMAL)."));
    }

    void applyOutputs(bool green, bool red, bool yellow) {
        digitalWrite(PIN_PILOT_GREEN, green ? HIGH : LOW);
        digitalWrite(PIN_PILOT_RED, red ? HIGH : LOW);
        digitalWrite(PIN_PILOT_YELLOW, yellow ? HIGH : LOW);
    }

    void setMode(PilotMode mode) {
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

    void setModeFromString(const String& modeStr) {
        if (modeStr.equalsIgnoreCase("NORMAL")) setMode(PILOT_MODE_NORMAL);
        else if (modeStr.equalsIgnoreCase("WARNING")) setMode(PILOT_MODE_WARNING);
        else if (modeStr.equalsIgnoreCase("DANGER")) setMode(PILOT_MODE_DANGER);
        else if (modeStr.equalsIgnoreCase("OFF")) setMode(PILOT_MODE_OFF);
    }

    PilotMode getMode() const { return m_mode; }

    void update(unsigned long currentMillis) {
        if (m_mode == PILOT_MODE_WARNING) {
            if (currentMillis - m_lastBlinkMillis >= 700) {
                m_lastBlinkMillis = currentMillis;
                m_blinkState = !m_blinkState;
                applyOutputs(false, false, m_blinkState);
            }
        } else if (m_mode == PILOT_MODE_DANGER) {
            if (currentMillis - m_lastBlinkMillis >= 200) {
                m_lastBlinkMillis = currentMillis;
                m_blinkState = !m_blinkState;
                applyOutputs(false, m_blinkState, false);
            }
        }
    }

private:
    PilotMode m_mode;
    unsigned long m_lastBlinkMillis;
    bool m_blinkState;
};

// ====================================================================
// 5. PANTALLA OLED SSD1306 (128x64)
// ====================================================================
class DisplayOLEDManager {
public:
    DisplayOLEDManager() : m_display(128, 64, &Wire, -1), m_initialized(false) {}

    bool begin(uint8_t i2c_address = 0x3C) {
        if (!m_display.begin(SSD1306_SWITCHCAPVCC, i2c_address)) {
            Serial.printf("[OLED ERROR] No se pudo iniciar SSD1306 en 0x%02X\n", i2c_address);
            m_initialized = false;
            return false;
        }
        m_initialized = true;
        m_display.clearDisplay();
        m_display.setTextColor(SSD1306_WHITE);
        m_display.setTextSize(1);
        m_display.setCursor(15, 20);
        m_display.println(F("ARCEUS GRID"));
        m_display.setCursor(15, 35);
        m_display.println(F("Iniciando nodo..."));
        m_display.display();
        return true;
    }

    void render(char stationId,
                const ElectricalMetrics& metrics,
                bool relay1,
                bool relay2,
                bool mqttConnected,
                PilotMode pilotMode,
                const char* alertMsg = nullptr) {
        if (!m_initialized) return;

        m_display.clearDisplay();
        m_display.setTextColor(SSD1306_WHITE);

        // Barra superior
        m_display.setTextSize(1);
        m_display.setCursor(0, 0);
        m_display.printf("ESTACION %c  [%s]", stationId, mqttConnected ? "MQTT" : "DISC");
        m_display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

        // Mediciones eléctricas
        m_display.setCursor(0, 13);
        m_display.printf("V: %5.2f V", metrics.busVoltage_V);
        m_display.setCursor(70, 13);
        m_display.printf("P: %4.1f W", metrics.power_W);

        m_display.setCursor(0, 25);
        m_display.printf("I: %5.3f A (%4.0f mA)", metrics.current_A, metrics.current_A * 1000.0f);

        // Estado de Relés
        m_display.drawFastHLine(0, 36, 128, SSD1306_WHITE);
        m_display.setCursor(0, 40);
        m_display.printf("RELES: B1:%s B2:%s", relay1 ? "ON " : "OFF", relay2 ? "ON " : "OFF");

        // Estado Operativo o Alerta
        m_display.setCursor(0, 52);
        if (alertMsg && strlen(alertMsg) > 0) {
            m_display.printf("! %s", alertMsg);
        } else {
            const char* stStr = "NORMAL";
            if (pilotMode == PILOT_MODE_WARNING) stStr = "WARNING";
            else if (pilotMode == PILOT_MODE_DANGER) stStr = "DANGER!";
            else if (pilotMode == PILOT_MODE_OFF) stStr = "OFF";
            m_display.printf("ESTADO: %s", stStr);
        }

        m_display.display();
    }

private:
    Adafruit_SSD1306 m_display;
    bool m_initialized;
};

// ====================================================================
// 6. GESTOR DE COMUNICACIÓN MQTT Y WIFI
// ====================================================================
class MQTTManager;
static MQTTManager* s_instance = nullptr;

class MQTTManager {
public:
    MQTTManager(RelayController& relays, PilotController& pilots)
        : m_relays(relays),
          m_pilots(pilots),
          m_mqttClient(m_espClient),
          m_lastWiFiAttempt(0),
          m_lastMQTTAttempt(0),
          m_wasWiFiConnected(false) {
        s_instance = this;
    }

    void begin() {
        m_mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
        m_mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
            if (s_instance) s_instance->handleIncomingCommand(topic, payload, length);
        });
        m_mqttClient.setBufferSize(512);

        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        reconnectWiFi();
    }

    void reconnectWiFi() {
        if (WiFi.status() == WL_CONNECTED) {
            if (!m_wasWiFiConnected) {
                m_wasWiFiConnected = true;
                Serial.printf("[WIFI] Conectado exitosamente. IP asignada: %s\n", WiFi.localIP().toString().c_str());
            }
            return;
        }

        m_wasWiFiConnected = false;
        unsigned long now = millis();
        if (m_lastWiFiAttempt != 0 && (now - m_lastWiFiAttempt < 10000)) return;

        m_lastWiFiAttempt = now;
        Serial.printf("[WIFI] Conectando a %s...\n", WIFI_SSID);
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }

    void reconnectMQTT() {
        if (WiFi.status() != WL_CONNECTED) {
            reconnectWiFi();
            return;
        }

        if (m_mqttClient.connected()) return;

        unsigned long now = millis();
        if (m_lastMQTTAttempt == 0 || (now - m_lastMQTTAttempt > 5000)) {
            m_lastMQTTAttempt = now;
            Serial.printf("[MQTT] Conectando a broker %s:%d...\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);

            String clientId = "ArceusNode_" + String(STATION_NAME) + "_" + String(random(0xffff), HEX);
            const char* willTopic = MQTT_TOPIC_STATUS;
            const char* willMsg = "{\"status\": \"OFFLINE\"}";

            if (m_mqttClient.connect(clientId.c_str(), willTopic, 1, true, willMsg)) {
                Serial.println(F("[MQTT] ¡Conexion establecida exitosamente!"));
                m_mqttClient.publish(willTopic, "{\"status\": \"ONLINE\"}", true);
                m_mqttClient.subscribe(MQTT_TOPIC_COMMANDS, 1);
                Serial.printf("[MQTT] Suscrito a comandos: %s\n", MQTT_TOPIC_COMMANDS);
            } else {
                Serial.printf("[MQTT ERROR] Fallo con codigo rc=%d. Reintento en 5s.\n", m_mqttClient.state());
            }
        }
    }

    void loop() {
        if (!m_mqttClient.connected()) {
            reconnectMQTT();
        } else {
            m_mqttClient.loop();
        }
    }

    bool isConnected() { return m_mqttClient.connected(); }

    bool publishTelemetry(const ElectricalMetrics& metrics, const char* status) {
        if (!m_mqttClient.connected()) return false;

        StaticJsonDocument<256> doc;
        doc["station"] = String(STATION_ID_CHAR);
        doc["voltage"] = serialized(String(metrics.busVoltage_V, 2));
        doc["current"] = serialized(String(metrics.current_A, 3));
        doc["power"]   = serialized(String(metrics.power_W, 2));
        doc["relay_1"] = m_relays.getRelayState(1);
        doc["relay_2"] = m_relays.getRelayState(2);
        doc["status"]  = status;

        char buffer[256];
        size_t n = serializeJson(doc, buffer);
        return m_mqttClient.publish(MQTT_TOPIC_TELEMETRY, (const uint8_t*)buffer, n, false);
    }

    bool publishAlert(const char* type, const char* message, const char* severity) {
        if (!m_mqttClient.connected()) return false;

        StaticJsonDocument<256> doc;
        doc["station"]  = String(STATION_ID_CHAR);
        doc["type"]     = type;
        doc["message"]  = message;
        doc["severity"] = severity;

        char buffer[256];
        size_t n = serializeJson(doc, buffer);
        return m_mqttClient.publish(MQTT_TOPIC_ALERTS, (const uint8_t*)buffer, n, false);
    }

    void handleIncomingCommand(char* topic, byte* payload, unsigned int length) {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
            Serial.printf("[MQTT CMD] Error JSON: %s\n", error.c_str());
            return;
        }

        const char* command = doc["command"] | "";
        Serial.printf("[MQTT CMD] Ejecutando: %s\n", command);

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

private:
    RelayController& m_relays;
    PilotController& m_pilots;
    WiFiClient m_espClient;
    PubSubClient m_mqttClient;
    unsigned long m_lastWiFiAttempt;
    unsigned long m_lastMQTTAttempt;
    bool m_wasWiFiConnected;
};

// ====================================================================
// 7. INSTANCIAS GLOBALES Y LÓGICA PRINCIPAL
// ====================================================================
SensorINA219Manager sensorINA;
DisplayOLEDManager  displayOLED;
RelayController     relays;
PilotController     pilots;
MQTTManager         mqtt(relays, pilots);

unsigned long lastSensorReadMillis = 0;
unsigned long lastDisplayMillis = 0;
unsigned long lastTelemetryMillis = 0;

ElectricalMetrics latestMetrics = {0};
String currentAlertMsg = "";

void evaluateLocalSafety(const ElectricalMetrics& m) {
    if (!m.isValid) return;

    if (m.busVoltage_V < LOCAL_VOLTAGE_MIN_V && m.busVoltage_V > 1.0f) {
        currentAlertMsg = "SUBTENSION!";
        pilots.setMode(PILOT_MODE_WARNING);
        mqtt.publishAlert("LOCAL_UNDERVOLTAGE", "Tension en bus inferior a 10.5V", "WARNING");
    } else if (m.busVoltage_V > LOCAL_VOLTAGE_MAX_V) {
        currentAlertMsg = "SOBRETENSION!";
        pilots.setMode(PILOT_MODE_DANGER);
        mqtt.publishAlert("LOCAL_OVERVOLTAGE", "Tension en bus superior a 13.5V", "DANGER");
    } else if (m.current_A > LOCAL_CURRENT_MAX_A) {
        currentAlertMsg = "SOBRECORRIENTE!";
        pilots.setMode(PILOT_MODE_DANGER);
        relays.allOff();
        mqtt.publishAlert("LOCAL_OVERCURRENT_TRIP", "Cargas desconectadas por sobrecorriente", "DANGER");
    } else {
        if (pilots.getMode() != PILOT_MODE_NORMAL) {
            pilots.setMode(PILOT_MODE_NORMAL);
            currentAlertMsg = "";
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500);

    Serial.println(F("\n=================================================="));
    Serial.printf(" ARCEUS GRID — Firmware Unificado: ESTACION %c\n", STATION_ID_CHAR);
    Serial.println(F("=================================================="));

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    relays.begin();
    pilots.begin();
    displayOLED.begin(I2C_ADDR_OLED);
    sensorINA.begin(I2C_ADDR_INA219);

    mqtt.begin();
    Serial.println(F("[SISTEMA] Nodo inicializado y en ejecucion."));
}

void loop() {
    unsigned long currentMillis = millis();

    mqtt.loop();
    pilots.update(currentMillis);

    // Lectura del sensor INA219 (cada 200 ms)
    if (currentMillis - lastSensorReadMillis >= INTERVAL_SENSOR_MS) {
        lastSensorReadMillis = currentMillis;
        latestMetrics = sensorINA.readMetrics();
        evaluateLocalSafety(latestMetrics);
    }

    // Refresco de Pantalla OLED (cada 300 ms)
    if (currentMillis - lastDisplayMillis >= INTERVAL_DISPLAY_MS) {
        lastDisplayMillis = currentMillis;
        displayOLED.render(
            STATION_ID_CHAR,
            latestMetrics,
            relays.getRelayState(1),
            relays.getRelayState(2),
            mqtt.isConnected(),
            pilots.getMode(),
            currentAlertMsg.c_str()
        );
    }

    // Publicación de telemetría MQTT (cada 1000 ms)
    if (currentMillis - lastTelemetryMillis >= INTERVAL_TELEMETRY_MS) {
        lastTelemetryMillis = currentMillis;

        const char* statusStr = "NORMAL";
        if (pilots.getMode() == PILOT_MODE_WARNING) statusStr = "WARNING";
        else if (pilots.getMode() == PILOT_MODE_DANGER) statusStr = "DANGER";
        else if (pilots.getMode() == PILOT_MODE_OFF) statusStr = "OFF";

        mqtt.publishTelemetry(latestMetrics, statusStr);
    }
}
