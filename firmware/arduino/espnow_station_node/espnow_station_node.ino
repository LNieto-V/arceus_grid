/**
 * ====================================================================
 * ARCEUS GRID — Firmware de Nodo Sensor (ESP-NOW)
 * Todo en Uno para Arduino IDE
 * Compatible con: ESP32 DevKit V1 (o DOIT ESP32 DevKit)
 * 
 * Rol: Nodo Sensor (Estación A o B).
 * Adquiere métricas eléctricas con INA219, gobierna relés y pilotos 12V,
 * gestiona display OLED SSD1306 y transmite/recibe vía ESP-NOW hacia
 * el ESP32 Gateway Central (sin necesidad de conectarse al Wi-Fi).
 * ====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Adafruit_INA219.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Compatibilidad con ESP32 Arduino Core v2.x y v3.x
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
    #define ESPNOW_RECV_V3
#endif

// ====================================================================
// 1. CONFIGURACIÓN DEL NODO
// ====================================================================
// Selecciona 'A' para Estación A, o 'B' para Estación B:
#define STATION_ID_CHAR 'A'
#define STATION_NAME    "station_a"

// Canal Wi-Fi inicial para ESP-NOW (se auto-sincroniza con el Gateway)
#define INITIAL_ESPNOW_CHANNEL 1

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
#define INTERVAL_TELEMETRY_MS 1000  // Envío ESP-NOW cada 1000 ms
#define INTERVAL_DISPLAY_MS    300  // Refresco OLED cada 300 ms
#define INTERVAL_SENSOR_MS     200  // Muestreo sensor INA219 cada 200 ms
#define CHANNEL_SCAN_TIMEOUT_MS 4000 // Si no hay ACK en 4s, prueba otro canal

// Umbrales locales de seguridad eléctrica
#define LOCAL_VOLTAGE_MIN_V 10.5f
#define LOCAL_VOLTAGE_MAX_V 13.5f
#define LOCAL_CURRENT_MAX_A  2.8f

// Dirección Broadcast para ESP-NOW (todos los dispositivos en el canal la reciben)
static uint8_t s_broadcast_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ====================================================================
// 2. PROTOCOLO BINARIO ESP-NOW
// ====================================================================
#define ARCEUS_MSG_TELEMETRY   0x01
#define ARCEUS_MSG_ALERT       0x02
#define ARCEUS_MSG_CMD_RELAY   0x10
#define ARCEUS_MSG_CMD_PILOT   0x11
#define ARCEUS_MSG_ACK         0x20

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;
    char     station_id;
    float    voltage_v;
    float    current_a;
    float    power_w;
    bool     relay_1;
    bool     relay_2;
    uint8_t  pilot_mode;
    uint32_t seq_num;
} ArceusEspNowTelemetry;

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;
    char     station_id;
    char     alert_type[24];
    char     message[48];
    char     severity[12];
} ArceusEspNowAlert;

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;
    char     target_station;
    uint8_t  param_index;
    bool     state;
} ArceusEspNowCommand;

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;
    char     station_id;
    uint8_t  wifi_channel;
    uint32_t timestamp_ms;
} ArceusEspNowAck;

// ====================================================================
// 3. ESTRUCTURAS Y CONTROLADOR DE SENSORES (INA219)
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
// 4. CONTROLADOR DE RELÉS
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
// 5. CONTROLADOR DE PILOTOS 12V (MÁQUINA DE ESTADOS MILLIS)
// ====================================================================
enum PilotMode {
    PILOT_MODE_OFF = 0,
    PILOT_MODE_NORMAL = 1,   // Verde fijo
    PILOT_MODE_WARNING = 2,  // Amarillo intermitente (700 ms)
    PILOT_MODE_DANGER = 3    // Rojo intermitente rapido (200 ms)
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
// 6. PANTALLA OLED SSD1306 (128x64)
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
        m_display.printf("Nodo ESP-NOW [%c]\n", STATION_ID_CHAR);
        m_display.display();
        return true;
    }

    void render(char stationId,
                const ElectricalMetrics& metrics,
                bool relay1,
                bool relay2,
                bool gatewayLinked,
                uint8_t currentChannel,
                PilotMode pilotMode,
                const char* alertMsg = nullptr) {
        if (!m_initialized) return;

        m_display.clearDisplay();
        m_display.setTextColor(SSD1306_WHITE);

        // Barra superior
        m_display.setTextSize(1);
        m_display.setCursor(0, 0);
        m_display.printf("ESTACION %c  [CH:%d %s]", 
                         stationId, 
                         currentChannel, 
                         gatewayLinked ? "LINK" : "SCAN");
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
// 7. INSTANCIAS GLOBALES Y LÓGICA DE CONTROL
// ====================================================================
SensorINA219Manager sensorINA;
DisplayOLEDManager  displayOLED;
RelayController     relays;
PilotController     pilots;

unsigned long lastSensorReadMillis = 0;
unsigned long lastDisplayMillis = 0;
unsigned long lastTelemetryMillis = 0;
unsigned long lastAckMillis = 0;
unsigned long lastChannelScanMillis = 0;

uint32_t telemetrySeqNum = 0;
uint8_t currentWifiChannel = INITIAL_ESPNOW_CHANNEL;
bool gatewayLinked = false;

ElectricalMetrics latestMetrics = {0};
String currentAlertMsg = "";

// Forward declarations
void sendEspNowTelemetry();
void sendEspNowAlert(const char* alertType, const char* message, const char* severity);

// Callback de recepción ESP-NOW
#if defined(ESPNOW_RECV_V3)
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
    const uint8_t *mac_addr = recv_info->src_addr;
#else
void onDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
#endif
    if (len <= 0) return;

    uint8_t msgType = incomingData[0];

    // Procesar ACK de sincronización recibido del Gateway
    if (msgType == ARCEUS_MSG_ACK && len >= sizeof(ArceusEspNowAck)) {
        ArceusEspNowAck ack;
        memcpy(&ack, incomingData, sizeof(ArceusEspNowAck));
        if (ack.station_id == STATION_ID_CHAR || ack.station_id == '*') {
            gatewayLinked = true;
            lastAckMillis = millis();
            // Si el Gateway nos reporta su canal, confirmamos que coincidimos
            if (ack.wifi_channel != 0 && ack.wifi_channel != currentWifiChannel) {
                currentWifiChannel = ack.wifi_channel;
                esp_wifi_set_channel(currentWifiChannel, WIFI_SECOND_CHAN_NONE);
            }
        }
    }
    // Procesar Comandos recibidos desde el Gateway (originados en MQTT/Streamlit)
    else if ((msgType == ARCEUS_MSG_CMD_RELAY || msgType == ARCEUS_MSG_CMD_PILOT) && len >= sizeof(ArceusEspNowCommand)) {
        ArceusEspNowCommand cmd;
        memcpy(&cmd, incomingData, sizeof(ArceusEspNowCommand));

        if (cmd.target_station == STATION_ID_CHAR || cmd.target_station == '*') {
            lastAckMillis = millis();
            gatewayLinked = true;

            if (cmd.msg_type == ARCEUS_MSG_CMD_RELAY) {
                Serial.printf("[ESPNOW CMD] Set Relay %d -> %s\n", cmd.param_index, cmd.state ? "ON" : "OFF");
                relays.setRelay(cmd.param_index, cmd.state);
            } else if (cmd.msg_type == ARCEUS_MSG_CMD_PILOT) {
                PilotMode pMode = (PilotMode)cmd.param_index;
                Serial.printf("[ESPNOW CMD] Set Pilot Mode -> %d\n", pMode);
                pilots.setMode(pMode);
            }
        }
    }
}

void registerBroadcastPeer() {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, s_broadcast_mac, 6);
    peerInfo.channel = 0; // 0 = sigue el canal actual del adaptador
    peerInfo.encrypt = false;

    if (esp_now_is_peer_exist(s_broadcast_mac)) {
        esp_now_del_peer(s_broadcast_mac);
    }
    esp_err_t res = esp_now_add_peer(&peerInfo);
    if (res != ESP_OK) {
        Serial.printf("[ESPNOW ERROR] No se pudo anadir peer broadcast: 0x%X\n", res);
    }
}

void sendEspNowTelemetry() {
    ArceusEspNowTelemetry packet;
    packet.msg_type   = ARCEUS_MSG_TELEMETRY;
    packet.station_id = STATION_ID_CHAR;
    packet.voltage_v  = latestMetrics.busVoltage_V;
    packet.current_a  = latestMetrics.current_A;
    packet.power_w    = latestMetrics.power_W;
    packet.relay_1    = relays.getRelayState(1);
    packet.relay_2    = relays.getRelayState(2);
    packet.pilot_mode = (uint8_t)pilots.getMode();
    packet.seq_num    = ++telemetrySeqNum;

    esp_err_t res = esp_now_send(s_broadcast_mac, (uint8_t*)&packet, sizeof(packet));
    if (res != ESP_OK) {
        Serial.printf("[ESPNOW ERROR] Error enviando telemetria: 0x%X\n", res);
    }
}

void sendEspNowAlert(const char* alertType, const char* message, const char* severity) {
    ArceusEspNowAlert alert;
    alert.msg_type   = ARCEUS_MSG_ALERT;
    alert.station_id = STATION_ID_CHAR;
    strncpy(alert.alert_type, alertType, sizeof(alert.alert_type) - 1);
    strncpy(alert.message, message, sizeof(alert.message) - 1);
    strncpy(alert.severity, severity, sizeof(alert.severity) - 1);
    alert.alert_type[sizeof(alert.alert_type) - 1] = '\0';
    alert.message[sizeof(alert.message) - 1] = '\0';
    alert.severity[sizeof(alert.severity) - 1] = '\0';

    esp_now_send(s_broadcast_mac, (uint8_t*)&alert, sizeof(alert));
}

void evaluateLocalSafety(const ElectricalMetrics& m) {
    if (!m.isValid) return;

    if (m.busVoltage_V < LOCAL_VOLTAGE_MIN_V && m.busVoltage_V > 1.0f) {
        currentAlertMsg = "SUBTENSION!";
        pilots.setMode(PILOT_MODE_WARNING);
        sendEspNowAlert("LOCAL_UNDERVOLTAGE", "Tension en bus inferior a 10.5V", "WARNING");
    } else if (m.busVoltage_V > LOCAL_VOLTAGE_MAX_V) {
        currentAlertMsg = "SOBRETENSION!";
        pilots.setMode(PILOT_MODE_DANGER);
        sendEspNowAlert("LOCAL_OVERVOLTAGE", "Tension en bus superior a 13.5V", "DANGER");
    } else if (m.current_A > LOCAL_CURRENT_MAX_A) {
        currentAlertMsg = "SOBRECORRIENTE!";
        pilots.setMode(PILOT_MODE_DANGER);
        relays.allOff();
        sendEspNowAlert("LOCAL_OVERCURRENT_TRIP", "Cargas desconectadas por sobrecorriente", "DANGER");
    } else {
        if (pilots.getMode() != PILOT_MODE_NORMAL) {
            pilots.setMode(PILOT_MODE_NORMAL);
            currentAlertMsg = "";
        }
    }
}

void checkChannelAutoSync(unsigned long now) {
    // Si han pasado más de CHANNEL_SCAN_TIMEOUT_MS sin recibir ACK del Gateway, probamos otro canal Wi-Fi
    if (now - lastAckMillis > CHANNEL_SCAN_TIMEOUT_MS) {
        gatewayLinked = false;
        if (now - lastChannelScanMillis >= CHANNEL_SCAN_TIMEOUT_MS) {
            lastChannelScanMillis = now;
            currentWifiChannel = (currentWifiChannel % 13) + 1; // Cicla canales 1..13
            esp_wifi_set_channel(currentWifiChannel, WIFI_SECOND_CHAN_NONE);
            registerBroadcastPeer();
            Serial.printf("[ESPNOW SCAN] Buscando Gateway en Canal Wi-Fi %d...\n", currentWifiChannel);
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500);

    Serial.println(F("\n=================================================="));
    Serial.printf(" ARCEUS GRID — Nodo Sensor ESP-NOW: ESTACION %c\n", STATION_ID_CHAR);
    Serial.println(F("=================================================="));

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    relays.begin();
    pilots.begin();
    displayOLED.begin(I2C_ADDR_OLED);
    sensorINA.begin(I2C_ADDR_INA219);

    // Inicializar Wi-Fi en modo Estación para el stack ESP-NOW (sin conectarse a ningún AP)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(currentWifiChannel, WIFI_SECOND_CHAN_NONE);

    Serial.print(F("[WIFI MAC Local]: "));
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println(F("[ESPNOW ERROR] Fallo critico iniciando ESP-NOW!"));
        return;
    }

    esp_now_register_recv_cb(onDataRecv);
    registerBroadcastPeer();

    lastAckMillis = millis();
    Serial.println(F("[SISTEMA] Nodo ESP-NOW inicializado y listo."));
}

void loop() {
    unsigned long currentMillis = millis();

    pilots.update(currentMillis);
    checkChannelAutoSync(currentMillis);

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
            gatewayLinked,
            currentWifiChannel,
            pilots.getMode(),
            currentAlertMsg.c_str()
        );
    }

    // Publicación de telemetría ESP-NOW al Gateway (cada 1000 ms)
    if (currentMillis - lastTelemetryMillis >= INTERVAL_TELEMETRY_MS) {
        lastTelemetryMillis = currentMillis;
        sendEspNowTelemetry();
    }
}
