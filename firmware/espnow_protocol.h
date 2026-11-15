/**
 * ====================================================================
 * ARCEUS GRID — ESP-NOW PROTOCOL DEFINITIONS
 * Estructuras de datos binarias para comunicación ultrarrápida
 * entre Nodos Sensores y ESP32 Gateway Central.
 * ====================================================================
 */

#ifndef ARCEUS_ESPNOW_PROTOCOL_H
#define ARCEUS_ESPNOW_PROTOCOL_H

#include <Arduino.h>

// Tipos de mensaje del protocolo
#define ARCEUS_MSG_TELEMETRY   0x01
#define ARCEUS_MSG_ALERT       0x02
#define ARCEUS_MSG_CMD_RELAY   0x10
#define ARCEUS_MSG_CMD_PILOT   0x11
#define ARCEUS_MSG_ACK         0x20

// 1. Paquete de Telemetría (Estación -> Gateway)
typedef struct __attribute__((packed)) {
    uint8_t  msg_type;       // ARCEUS_MSG_TELEMETRY
    char     station_id;     // 'A' o 'B'
    float    voltage_v;      // Tensión de bus en Voltios
    float    current_a;      // Corriente en Amperios
    float    power_w;        // Potencia en Vatios
    bool     relay_1;        // Estado Relé 1 (true = ON)
    bool     relay_2;        // Estado Relé 2 (true = ON)
    uint8_t  pilot_mode;     // 0=OFF, 1=NORMAL, 2=WARNING, 3=DANGER
    uint32_t seq_num;        // Contador de secuencia
} ArceusEspNowTelemetry;

// 2. Paquete de Alertas Locales (Estación -> Gateway)
typedef struct __attribute__((packed)) {
    uint8_t  msg_type;       // ARCEUS_MSG_ALERT
    char     station_id;     // 'A' o 'B'
    char     alert_type[24]; // Ej: "LOCAL_OVERVOLTAGE"
    char     message[48];    // Descripción
    char     severity[12];   // "WARNING" o "DANGER"
} ArceusEspNowAlert;

// 3. Paquete de Comandos (Gateway -> Estación)
typedef struct __attribute__((packed)) {
    uint8_t  msg_type;       // ARCEUS_MSG_CMD_RELAY o ARCEUS_MSG_CMD_PILOT
    char     target_station; // 'A', 'B' o '*' (todas)
    uint8_t  param_index;    // Índice de relé (1 o 2) o modo de piloto (0..3)
    bool     state;          // true = ON, false = OFF (para relé)
} ArceusEspNowCommand;

// 4. Paquete de Sincronización y ACK (Gateway -> Estación)
typedef struct __attribute__((packed)) {
    uint8_t  msg_type;       // ARCEUS_MSG_ACK
    char     station_id;     // 'A' o 'B'
    uint8_t  wifi_channel;   // Canal Wi-Fi activo del Gateway
    uint32_t timestamp_ms;   // Millis del Gateway
} ArceusEspNowAck;

#endif // ARCEUS_ESPNOW_PROTOCOL_H
