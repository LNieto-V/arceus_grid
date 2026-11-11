#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "sensors_ina219.h"
#include "display_oled.h"
#include "relays.h"
#include "pilots.h"
#include "mqtt_manager.h"

// Instanciación de subsistemas modulares
SensorINA219Manager sensorINA;
DisplayOLEDManager  displayOLED;
RelayController     relays;
PilotController     pilots;
MQTTManager         mqtt(relays, pilots);

// Variables de estado y temporización no bloqueante
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
        // Desconectar cargas por proteccion local inmediata
        relays.allOff();
        mqtt.publishAlert("LOCAL_OVERCURRENT_TRIP", "Cargas desconectadas por sobrecorriente", "DANGER");
    } else {
        if (pilots.getMode() != PILOT_MODE_NORMAL) {
            // Retorno automatico a normal si no hay falla
            pilots.setMode(PILOT_MODE_NORMAL);
            currentAlertMsg = "";
        }
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1500);

    Serial.println(F("\n=================================================="));
    Serial.printf(" ARCEUS GRID — Firmware de Nodo: ESTACION %c\n", STATION_ID_CHAR);
    Serial.println(F("=================================================="));

    // 1. Inicializar bus I2C (SDA=21, SCL=22)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);  // 400 kHz Fast I2C

    // 2. Inicializar actuadores y periféricos
    relays.begin();
    pilots.begin();
    displayOLED.begin(I2C_ADDR_OLED);
    sensorINA.begin(I2C_ADDR_INA219);

    // 3. Inicializar conectividad MQTT y WiFi
    mqtt.begin();

    Serial.println(F("[SISTEMA] Nodo inicializado y en ejecucion no bloqueante."));
}

void loop() {
    unsigned long currentMillis = millis();

    // Tarea continua: loop de red MQTT y máquina de parpadeo de pilotos
    mqtt.loop();
    pilots.update(currentMillis);

    // Tarea 1: Adquisición de sensor INA219 (cada 200 ms)
    if (currentMillis - lastSensorReadMillis >= INTERVAL_SENSOR_MS) {
        lastSensorReadMillis = currentMillis;
        latestMetrics = sensorINA.readMetrics();
        evaluateLocalSafety(latestMetrics);
    }

    // Tarea 2: Refresco de Pantalla OLED (cada 300 ms)
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

    // Tarea 3: Publicación de Telemetría JSON vía MQTT (cada 1000 ms)
    if (currentMillis - lastTelemetryMillis >= INTERVAL_TELEMETRY_MS) {
        lastTelemetryMillis = currentMillis;

        const char* statusStr = "NORMAL";
        if (pilots.getMode() == PILOT_MODE_WARNING) statusStr = "WARNING";
        else if (pilots.getMode() == PILOT_MODE_DANGER) statusStr = "DANGER";
        else if (pilots.getMode() == PILOT_MODE_OFF) statusStr = "OFF";

        mqtt.publishTelemetry(latestMetrics, statusStr);
    }
}
