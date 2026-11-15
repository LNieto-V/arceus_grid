# 📡 Arquitectura ESP-NOW — ARCEUS GRID

Este documento detalla la topología de comunicación inalámbrica basada en **ESP-NOW** para una microrred de tres microcontroladores **ESP32**:
- **1x ESP32 Main (Gateway Central)**
- **2x ESP32 Nodos de Potencia y Sensórica (Estación A y Estación B)**

---

## 1. Topología del Sistema

```
   +------------------------------------+       +------------------------------------+
   |   ESP32 — Estación A (Sensor)      |       |   ESP32 — Estación B (Sensor)      |
   |   - INA219 (Tensión/Corriente/W)   |       |   - INA219 (Tensión/Corriente/W)   |
   |   - 2x Relés de Potencia 12V       |       |   - 2x Relés de Potencia 12V       |
   |   - 3x Pilotos LED 12V             |       |   - 3x Pilotos LED 12V             |
   |   - Pantalla OLED SSD1306          |       |   - Pantalla OLED SSD1306          |
   |   - Enlace: ESP-NOW (Canal Auto)   |       |   - Enlace: ESP-NOW (Canal Auto)   |
   +-----------------+------------------+       +-----------------+------------------+
                     |                                            |
                     |  (Paquetes binarios ESP-NOW ~25 bytes)     |
                     +---------------------+----------------------+
                                           |
                                           v
                        +------------------------------------+
                        |    ESP32 — Main Gateway Central    |
                        |    - Receptor ESP-NOW              |
                        |    - Cliente Wi-Fi (Punto Acceso)  |
                        |    - Publicador / Suscriptor MQTT  |
                        |    - Pantalla OLED (Opcional)      |
                        +------------------+-----------------+
                                           |
                                           | TCP/IP Wi-Fi (172.20.10.2:1884)
                                           v
                        +------------------------------------+
                        |      Docker Host PC (Mosquitto)    |
                        |  - arceus_collector (SQLite DB)    |
                        |  - arceus_dashboard (Streamlit)    |
                        +------------------------------------+
```

---

## 2. Ventajas Clave

1. **Aislamiento de Red Wi-Fi:** Solo el **ESP32 Main** necesita conectarse a la red Wi-Fi (`iPhone Owen`). Los dos nodos de medición no requieren contraseñas de red ni negociación DHCP.
2. **Auto-Sincronización de Canal Wi-Fi:** Los nodos sensores escanean automáticamente los canales Wi-Fi (1 a 13) hasta detectar la presencia del Gateway, bloqueando el canal correcto sin intervención manual.
3. **Control Bidireccional Transparente:**
   - **Subida (Uplink):** Telemetría de Estación A y B $\rightarrow$ Gateway $\rightarrow$ MQTT (`arceus/station_a/telemetry`, `arceus/station_b/telemetry`).
   - **Bajada (Downlink):** Comandos desde el Dashboard Streamlit $\rightarrow$ MQTT (`arceus/station_a/commands`) $\rightarrow$ Gateway $\rightarrow$ ESP-NOW $\rightarrow$ Conmutación de relés y pilotos en la estación física.
4. **Protección Eléctrica Local Inmediata:** Si ocurre una sobrecorriente (> 2.8 A), el nodo sensor corta los relés en microsegundos de forma autónoma sin depender de la latencia de radio.

---

## 3. Protocolo Binario de Datos

Definido en el archivo común [espnow_protocol.h](file:///C:/Users/owenm/proyectos/arceus_grid/firmware/espnow_protocol.h):

| Tipo de Mensaje | Código | Origen $\rightarrow$ Destino | Carga Útil (Payload) |
|---|---|---|---|
| `ARCEUS_MSG_TELEMETRY` | `0x01` | Estación $\rightarrow$ Gateway | Estación, Tensión ($V$), Corriente ($A$), Potencia ($W$), Relé 1, Relé 2, Modo Piloto, Secuencia |
| `ARCEUS_MSG_ALERT` | `0x02` | Estación $\rightarrow$ Gateway | Estación, Tipo de Alerta, Mensaje descriptivo, Severidad |
| `ARCEUS_MSG_CMD_RELAY` | `0x10` | Gateway $\rightarrow$ Estación | Estación Destino, Índice de Relé (1 o 2), Estado (ON/OFF) |
| `ARCEUS_MSG_CMD_PILOT` | `0x11` | Gateway $\rightarrow$ Estación | Estación Destino, Modo Piloto (NORMAL, WARNING, DANGER, OFF) |
| `ARCEUS_MSG_ACK` | `0x20` | Gateway $\rightarrow$ Estación | Confirmación de canal activo y sincronismo de paquetes |

---

## 4. Guía de Flasheo (Paso a Paso en Arduino IDE)

### Paso A: Flashear el ESP32 Main (Gateway)
1. Conecta el **ESP32 Main** a la PC por USB.
2. Abre en Arduino IDE el archivo:  
   👉 [espnow_gateway.ino](file:///C:/Users/owenm/proyectos/arceus_grid/firmware/arduino/espnow_gateway/espnow_gateway.ino)
3. Verifica las credenciales de red (líneas 32-38):
   ```cpp
   #define WIFI_SSID        "iPhone Owen"
   #define WIFI_PASSWORD    "0864297531"
   #define MQTT_BROKER_IP   "172.20.10.2"
   #define MQTT_BROKER_PORT 1884
   ```
4. Selecciona Placa: `ESP32 Dev Module` y el puerto COM correspondiente.
5. Haz clic en **Subir**.
6. En el Monitor Serie a `115200` verás:
   ```text
   [WIFI] Conectado en Canal Wi-Fi: X (IP: 172.20.10.x)
   [MQTT] ¡Conexion establecida exitosamente!
   [SISTEMA] Gateway inicializado y a la espera de paquetes.
   ```

---

### Paso B: Flashear el ESP32 de la Estación A
1. Conecta el ESP32 de la **Estación A** por USB.
2. Abre en Arduino IDE el archivo:  
   👉 [espnow_station_node.ino](file:///C:/Users/owenm/proyectos/arceus_grid/firmware/arduino/espnow_station_node/espnow_station_node.ino)
3. Verifica que en las líneas 27-28 esté configurado:
   ```cpp
   #define STATION_ID_CHAR 'A'
   #define STATION_NAME    "station_a"
   ```
4. Haz clic en **Subir**.
5. Al encender, su pantalla OLED mostrará:
   `ESTACION A  [CH:X LINK]` y transmitirá su telemetría cada segundo.

---

### Paso C: Flashear el ESP32 de la Estación B
1. Conecta el ESP32 de la **Estación B** por USB.
2. En el mismo archivo [espnow_station_node.ino](file:///C:/Users/owenm/proyectos/arceus_grid/firmware/arduino/espnow_station_node/espnow_station_node.ino), cambia las líneas 27-28 a:
   ```cpp
   #define STATION_ID_CHAR 'B'
   #define STATION_NAME    "station_b"
   ```
3. Haz clic en **Subir**.
4. ¡Listo! Ambas estaciones transmitirán en paralelo al Gateway sin interferencias.
