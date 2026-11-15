/**
 * ====================================================================
 * ARCEUS GRID — Firmware de Gateway Central (ESP-NOW + Wi-Fi + MQTT)
 * Todo en Uno para Arduino IDE
 * Compatible con: ESP32 DevKit V1 (o DOIT ESP32 DevKit)
 * 
 * Rol: Nodo Central / Gateway Principal (ESP32 Main).
 * Se conecta a la red Wi-Fi y al Broker MQTT (Docker).
 * Recibe inalámbricamente por ESP-NOW los datos de las Estaciones A y B,
 * los publica a Mosquitto en formato JSON y reenvía comandos de control
 * desde el Dashboard hacia las estaciones por radio ESP-NOW.
 * ====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Compatibilidad con ESP32 Arduino Core v2.x y v3.x
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
    #define ESPNOW_RECV_V3
#endif

// ====================================================================
// 1. CONFIGURACIÓN DE RED Y MQTT
// ====================================================================
#define GATEWAY_NAME     "arceus_main_gateway"

// Credenciales Wi-Fi (Punto de acceso iPhone / Router)
#define WIFI_SSID        "iPhone Owen"
#define WIFI_PASSWORD    "0864297531"

// Broker MQTT (PC anfitrión corriendo Docker)
#define MQTT_BROKER_IP   "172.20.10.2"
#define MQTT_BROKER_PORT 1884

// Tópicos MQTT
#define MQTT_TOPIC_GATEWAY_STATUS "arceus/gateway/status"
#define MQTT_TOPIC_CMD_SUB        "arceus/+/commands"

// Pines opcionales para pantalla OLED en el Gateway (I2C)
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22
#define I2C_ADDR_OLED 0x3C

// Dirección Broadcast ESP-NOW
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

// Estructura de estado en memoria por estación
struct StationCache {
    float voltage;
    float current;
    float power;
    bool relay1;
    bool relay2;
    uint8_t pilotMode;
    uint32_t lastSeq;
    unsigned long lastSeenMillis;
    bool online;
};

static StationCache cacheStationA = {0};
static StationCache cacheStationB = {0};

// ====================================================================
// 3. CONTROLADOR DE PANTALLA OLED OPCIONAL
// ====================================================================
class GatewayOLEDManager {
public:
    GatewayOLEDManager() : m_display(128, 64, &Wire, -1), m_available(false) {}

    bool begin() {
        Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
        Wire.beginTransmission(I2C_ADDR_OLED);
        if (Wire.endTransmission() != 0) {
            Serial.println(F("[OLED] No se detecto pantalla OLED en el Gateway (operando en modo headless)."));
            m_available = false;
            return false;
        }

        if (!m_display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDR_OLED)) {
            m_available = false;
            return false;
        }

        m_available = true;
        m_display.clearDisplay();
        m_display.setTextColor(SSD1306_WHITE);
        m_display.setTextSize(1);
        m_display.setCursor(10, 15);
        m_display.println(F("ARCEUS GATEWAY"));
        m_display.setCursor(10, 30);
        m_display.println(F("Iniciando Main..."));
        m_display.display();
        return true;
    }

    void render(bool wifiOk, bool mqttOk, uint8_t channel, const StationCache& a, const StationCache& b) {
        if (!m_available) return;

        m_display.clearDisplay();
        m_display.setTextColor(SSD1306_WHITE);

        // Barra superior
        m_display.setTextSize(1);
        m_display.setCursor(0, 0);
        m_display.printf("GATEWAY CH:%d [%s|%s]", 
                         channel, 
                         wifiOk ? "WIFI" : "!WF", 
                         mqttOk ? "MQTT" : "!MQ");
        m_display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

        // Estación A
        m_display.setCursor(0, 13);
        if (a.online) {
            m_display.printf("A: %4.1fV %5.2fA R:%d%d", a.voltage, a.current, a.relay1 ? 1 : 0, a.relay2 ? 1 : 0);
        } else {
            m_display.println(F("A: [OFFLINE]"));
        }

        // Estación B
        m_display.setCursor(0, 26);
        if (b.online) {
            m_display.printf("B: %4.1fV %5.2fA R:%d%d", b.voltage, b.current, b.relay1 ? 1 : 0, b.relay2 ? 1 : 0);
        } else {
            m_display.println(F("B: [OFFLINE]"));
        }

        m_display.drawFastHLine(0, 38, 128, SSD1306_WHITE);

        // Resumen de enlaces
        m_display.setCursor(0, 42);
        m_display.printf("Broker: %s:%d", MQTT_BROKER_IP, MQTT_BROKER_PORT);
        m_display.setCursor(0, 53);
        m_display.println(F("ESP-NOW: Receptor ON"));

        m_display.display();
    }

private:
    Adafruit_SSD1306 m_display;
    bool m_available;
};

// ====================================================================
// 4. INSTANCIAS Y GESTIÓN DE COMUNICACIONES
// ====================================================================
WiFiClient      espClient;
PubSubClient    mqttClient(espClient);
GatewayOLEDManager gatewayOLED;

unsigned long lastWiFiAttempt = 0;
unsigned long lastMQTTAttempt = 0;
unsigned long lastDisplayMillis = 0;
uint8_t activeWifiChannel = 1;

void sendEspNowAck(char stationId) {
    ArceusEspNowAck ack;
    ack.msg_type     = ARCEUS_MSG_ACK;
    ack.station_id   = stationId;
    ack.wifi_channel = activeWifiChannel;
    ack.timestamp_ms = millis();

    esp_now_send(s_broadcast_mac, (uint8_t*)&ack, sizeof(ack));
}

void publishTelemetryToMQTT(const ArceusEspNowTelemetry& t) {
    if (!mqttClient.connected()) return;

    char topic[64];
    snprintf(topic, sizeof(topic), "arceus/station_%c/telemetry", tolower(t.station_id));

    const char* statusStr = "NORMAL";
    if (t.pilot_mode == 2) statusStr = "WARNING";
    else if (t.pilot_mode == 3) statusStr = "DANGER";
    else if (t.pilot_mode == 0) statusStr = "OFF";

    StaticJsonDocument<256> doc;
    doc["station"] = String(t.station_id);
    doc["voltage"] = serialized(String(t.voltage_v, 2));
    doc["current"] = serialized(String(t.current_a, 3));
    doc["power"]   = serialized(String(t.power_w, 2));
    doc["relay_1"] = t.relay_1;
    doc["relay_2"] = t.relay_2;
    doc["status"]  = statusStr;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    mqttClient.publish(topic, (const uint8_t*)buffer, n, false);
}

void publishAlertToMQTT(const ArceusEspNowAlert& a) {
    if (!mqttClient.connected()) return;

    char topic[64];
    snprintf(topic, sizeof(topic), "arceus/station_%c/alerts", tolower(a.station_id));

    StaticJsonDocument<256> doc;
    doc["station"]  = String(a.station_id);
    doc["type"]     = a.alert_type;
    doc["message"]  = a.message;
    doc["severity"] = a.severity;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    mqttClient.publish(topic, (const uint8_t*)buffer, n, false);
}

// Callback de recepción ESP-NOW
#if defined(ESPNOW_RECV_V3)
void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
    const uint8_t *mac_addr = recv_info->src_addr;
#else
void onDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
#endif
    if (len <= 0) return;

    uint8_t msgType = incomingData[0];

    if (msgType == ARCEUS_MSG_TELEMETRY && len >= sizeof(ArceusEspNowTelemetry)) {
        ArceusEspNowTelemetry t;
        memcpy(&t, incomingData, sizeof(ArceusEspNowTelemetry));

        // Actualizar cache local
        StationCache* cache = (t.station_id == 'A') ? &cacheStationA : ((t.station_id == 'B') ? &cacheStationB : nullptr);
        if (cache) {
            cache->voltage = t.voltage_v;
            cache->current = t.current_a;
            cache->power   = t.power_w;
            cache->relay1  = t.relay_1;
            cache->relay2  = t.relay_2;
            cache->pilotMode = t.pilot_mode;
            cache->lastSeq = t.seq_num;
            cache->lastSeenMillis = millis();
            cache->online = true;
        }

        Serial.printf("[ESPNOW RX %c] V:%.2fV I:%.3fA P:%.2fW | Relays:%d%d Pilot:%d (Seq #%u)\n",
                      t.station_id, t.voltage_v, t.current_a, t.power_w, t.relay_1, t.relay_2, t.pilot_mode, t.seq_num);

        // Enviar confirmación inmediata de canal/ACK a la estación
        sendEspNowAck(t.station_id);

        // Retransmitir al broker MQTT (Docker)
        publishTelemetryToMQTT(t);
    }
    else if (msgType == ARCEUS_MSG_ALERT && len >= sizeof(ArceusEspNowAlert)) {
        ArceusEspNowAlert a;
        memcpy(&a, incomingData, sizeof(ArceusEspNowAlert));
        Serial.printf("[ESPNOW ALERT %c] Tipo:%s Msg:%s (%s)\n",
                      a.station_id, a.alert_type, a.message, a.severity);
        publishAlertToMQTT(a);
    }
}

// Callback de comandos MQTT desde el Coordinador/Streamlit
void handleIncomingMQTTCommand(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.printf("[MQTT CMD ERROR] JSON invalido: %s\n", error.c_str());
        return;
    }

    // Determinar la estación destino a partir del tópico
    char targetStation = '*';
    if (strstr(topic, "station_a")) targetStation = 'A';
    else if (strstr(topic, "station_b")) targetStation = 'B';

    const char* command = doc["command"] | "";
    Serial.printf("[MQTT CMD] Estacion:%c Comando:%s\n", targetStation, command);

    ArceusEspNowCommand cmdPacket;
    cmdPacket.target_station = targetStation;

    if (strcmp(command, "SET_RELAY") == 0) {
        cmdPacket.msg_type    = ARCEUS_MSG_CMD_RELAY;
        cmdPacket.param_index = doc["relay"] | 1;
        cmdPacket.state       = doc["state"] | false;

        esp_now_send(s_broadcast_mac, (uint8_t*)&cmdPacket, sizeof(cmdPacket));
        Serial.printf("[ESPNOW TX CMD] Reenviando SET_RELAY %d=%d a Estacion %c\n",
                      cmdPacket.param_index, cmdPacket.state, targetStation);
    } 
    else if (strcmp(command, "SET_PILOT") == 0) {
        cmdPacket.msg_type = ARCEUS_MSG_CMD_PILOT;
        const char* modeStr = doc["mode"] | "NORMAL";
        uint8_t pMode = 1; // NORMAL
        if (strcasecmp(modeStr, "WARNING") == 0) pMode = 2;
        else if (strcasecmp(modeStr, "DANGER") == 0) pMode = 3;
        else if (strcasecmp(modeStr, "OFF") == 0) pMode = 0;

        cmdPacket.param_index = pMode;
        cmdPacket.state = false;

        esp_now_send(s_broadcast_mac, (uint8_t*)&cmdPacket, sizeof(cmdPacket));
        Serial.printf("[ESPNOW TX CMD] Reenviando SET_PILOT %d (%s) a Estacion %c\n",
                      pMode, modeStr, targetStation);
    }
}

void registerBroadcastPeer() {
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, s_broadcast_mac, 6);
    peerInfo.channel = 0; // Seguir canal Wi-Fi activo
    peerInfo.encrypt = false;

    if (esp_now_is_peer_exist(s_broadcast_mac)) {
        esp_now_del_peer(s_broadcast_mac);
    }
    esp_now_add_peer(&peerInfo);
}

void reconnectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;

    unsigned long now = millis();
    if (lastWiFiAttempt != 0 && (now - lastWiFiAttempt < 10000)) return;

    lastWiFiAttempt = now;
    Serial.printf("[WIFI] Conectando a %s...\n", WIFI_SSID);
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void reconnectMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (mqttClient.connected()) return;

    unsigned long now = millis();
    if (lastMQTTAttempt == 0 || (now - lastMQTTAttempt > 5000)) {
        lastMQTTAttempt = now;
        Serial.printf("[MQTT] Conectando a broker %s:%d...\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);

        String clientId = "ArceusGateway_" + String(random(0xffff), HEX);
        const char* willTopic = MQTT_TOPIC_GATEWAY_STATUS;
        const char* willMsg = "{\"gateway\": \"OFFLINE\"}";

        if (mqttClient.connect(clientId.c_str(), willTopic, 1, true, willMsg)) {
            Serial.println(F("[MQTT] ¡Conexion establecida exitosamente!"));
            mqttClient.publish(willTopic, "{\"gateway\": \"ONLINE\"}", true);
            mqttClient.subscribe(MQTT_TOPIC_CMD_SUB, 1);
            Serial.printf("[MQTT] Suscrito a comandos: %s\n", MQTT_TOPIC_CMD_SUB);
        } else {
            Serial.printf("[MQTT ERROR] Fallo con codigo rc=%d. Reintento en 5s.\n", mqttClient.state());
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500);

    Serial.println(F("\n=================================================="));
    Serial.println(F(" ARCEUS GRID — ESP32 Main Gateway (ESP-NOW -> MQTT)"));
    Serial.println(F("=================================================="));

    gatewayOLED.begin();

    // Modo Wi-Fi dual (AP_STA o STA) para operar simultáneamente Wi-Fi y ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    Serial.print(F("[WIFI MAC Gateway]: "));
    Serial.println(WiFi.macAddress());

    // Iniciar conexión Wi-Fi inicial
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Inicializar stack ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println(F("[ESPNOW ERROR] Fallo critico iniciando ESP-NOW en el Gateway!"));
        return;
    }

    esp_now_register_recv_cb(onDataRecv);
    registerBroadcastPeer();

    mqttClient.setServer(MQTT_BROKER_IP, MQTT_BROKER_PORT);
    mqttClient.setCallback(handleIncomingMQTTCommand);
    mqttClient.setBufferSize(512);

    Serial.println(F("[SISTEMA] Gateway inicializado y a la espera de paquetes."));
}

void loop() {
    unsigned long currentMillis = millis();

    // Gestión de Wi-Fi y canal
    if (WiFi.status() == WL_CONNECTED) {
        uint8_t ch = WiFi.channel();
        if (ch != activeWifiChannel) {
            activeWifiChannel = ch;
            registerBroadcastPeer();
            Serial.printf("[WIFI] Conectado en Canal Wi-Fi: %d (IP: %s)\n",
                          activeWifiChannel, WiFi.localIP().toString().c_str());
        }
    } else {
        reconnectWiFi();
    }

    // Gestión MQTT
    if (!mqttClient.connected()) {
        reconnectMQTT();
    } else {
        mqttClient.loop();
    }

    // Control de timeout de estaciones (si no transmiten en 5s, marcarlas OFFLINE)
    if (cacheStationA.online && (currentMillis - cacheStationA.lastSeenMillis > 5000)) {
        cacheStationA.online = false;
    }
    if (cacheStationB.online && (currentMillis - cacheStationB.lastSeenMillis > 5000)) {
        cacheStationB.online = false;
    }

    // Refresco de Pantalla OLED local (cada 400 ms si está conectada)
    if (currentMillis - lastDisplayMillis >= 400) {
        lastDisplayMillis = currentMillis;
        gatewayOLED.render(
            WiFi.status() == WL_CONNECTED,
            mqttClient.connected(),
            activeWifiChannel,
            cacheStationA,
            cacheStationB
        );
    }
}
