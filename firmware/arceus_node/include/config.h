#pragma once

#include <Arduino.h>

// ====================================================================
// IDENTIFICACIÓN DE LA ESTACIÓN (Configurable por build_flags o manual)
// ====================================================================
#ifndef STATION_ID_CHAR
#define STATION_ID_CHAR 'A'  // 'A' para Estación A, 'B' para Estación B
#endif

#ifndef STATION_NAME
#define STATION_NAME "station_a"
#endif

// ====================================================================
// ASIGNACIÓN DE PINES GPIO (ESP32)
// ====================================================================
// Bus I2C Compartido (OLED + INA219)
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// Módulo de Relés (Control de Bombillas de 12V DC)
#define PIN_RELAY_1 25
#define PIN_RELAY_2 26

// Nivel lógico de disparo del relé (La mayoría de módulos de relés son activos en LOW)
#define RELAY_ACTIVE_LEVEL LOW
#define RELAY_INACTIVE_LEVEL HIGH

// Control de Pilotos Indicadores de 12V DC (Etapa MOSFET/Transistor)
#define PIN_PILOT_GREEN  27  // Funcionamiento Normal
#define PIN_PILOT_RED    32  // Alarma / Peligro
#define PIN_PILOT_YELLOW 33  // Advertencia

// ====================================================================
// DIRECCIONES I2C
// ====================================================================
#define I2C_ADDR_OLED   0x3C
#define I2C_ADDR_INA219 0x40

// ====================================================================
// CONFIGURACIÓN DE RED Y MQTT
// ====================================================================
#define WIFI_SSID     "ARCEUS_NETWORK"
#define WIFI_PASSWORD "arceus_secure_key"

#define MQTT_BROKER_IP   "192.168.1.100"  // IP del Host/Raspberry/PC con Mosquitto
#define MQTT_BROKER_PORT 1883

// Tópicos MQTT estructurados
#define MQTT_TOPIC_TELEMETRY "arceus/" STATION_NAME "/telemetry"
#define MQTT_TOPIC_STATUS    "arceus/" STATION_NAME "/status"
#define MQTT_TOPIC_COMMANDS  "arceus/" STATION_NAME "/commands"
#define MQTT_TOPIC_ALERTS    "arceus/" STATION_NAME "/alerts"

// ====================================================================
// INTERVALOS DE TIEMPO (no bloqueantes, millis)
// ====================================================================
#define INTERVAL_TELEMETRY_MS  1000  // Envío MQTT cada 1 segundo
#define INTERVAL_DISPLAY_MS     300  // Refresco de pantalla OLED cada 300 ms
#define INTERVAL_SENSOR_MS      200  // Adquisición del INA219 cada 200 ms

// Umbrales locales de seguridad eléctrica para detección en el nodo
#define LOCAL_VOLTAGE_MIN_V 10.5f
#define LOCAL_VOLTAGE_MAX_V 13.5f
#define LOCAL_CURRENT_MAX_A  2.8f
