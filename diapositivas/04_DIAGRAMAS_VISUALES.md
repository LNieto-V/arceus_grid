# 📐 ARCEUS GRID — Diagramas Visuales de Arquitectura e Ingeniería

Esta colección de diagramas en formato **Mermaid** modela los subsistemas físicos, eléctricos, de comunicaciones y de Inteligencia Artificial del proyecto. Pueden visualizarse en GitHub, Obsidian, VS Code o exportarse como imágenes para las diapositivas.

---

## 1. Arquitectura Eléctrica y Topología de Potencia (Star Ground + JD-VCC)

Este diagrama ilustra la separación de masas, el aislamiento de relés y la alimentación de los microcontroladores:

```mermaid
flowchart TD
    subgraph Fuente_Central ["FUENTE DE ALIMENTACIÓN 12V DC / 10A"]
        V12["+12V DC (Bus Principal)"]
        GND_CENTRAL["★ STAR GROUND (Tierra Central en Borne)"]
    end

    subgraph Regulacion_Control ["ETAPA DE REGULACIÓN DE CONTROL"]
        LM_A["LM2596 (Estación A)<br>12V ➔ 5.0V DC"]
        LM_B["LM2596 (Estación B)<br>12V ➔ 5.0V DC"]
    end

    subgraph Estacion_A ["ESTACIÓN A (NODO SENSOR)"]
        ESP32_A["ESP32 DevKit A<br>(Alimentado a 5V en VIN)"]
        INA_A["Sensor INA219 (0x40)<br>(En línea High-Side +12V)"]
        OLED_A["Display OLED SSD1306<br>(Bus I2C: 0x3C)"]
        PILOTS_A["Pilotos 12V (Verde/Amarillo/Rojo)<br>Drivers 2N2222"]
    end

    subgraph Aislamiento_Reles ["MÓDULO DE RELÉS OPTOACOPLADOS"]
        RELE_VCC["VCC Lógica (3.3V desde ESP32)<br>(Solo alimenta diodos optoacoplador)"]
        RELE_JDVCC["JD-VCC Potencia (5V desde LM2596)<br>(Alimenta bobinas electromagnéticas)"]
        CONTACTS["Contactos NO/COM<br>(Conmutan línea +12V hacia cargas)"]
    end

    subgraph Cargas_A ["CARGAS ESTACIÓN A"]
        LAMP1["Bombilla LED 1 (12V / 5W)"]
        LAMP2["Bombilla LED 2 (12V / 5W)"]
    end

    %% Conexiones de Alimentación
    V12 --> LM_A
    V12 --> LM_B
    V12 --> INA_A
    V12 --> PILOTS_A

    LM_A -->|"5V Regulado"| ESP32_A
    LM_A -->|"5V Aislado"| RELE_JDVCC

    %% Instrumentación
    INA_A -->|"12V Monitoreado"| CONTACTS
    CONTACTS --> LAMP1
    CONTACTS --> LAMP2

    %% Control Digital
    ESP32_A -->|"I2C (GPIO21/22)"| INA_A
    ESP32_A -->|"I2C (GPIO21/22)"| OLED_A
    ESP32_A -->|"GPIO Control (Activo LOW)"| RELE_VCC
    ESP32_A -->|"GPIO27/32/33 (Bases 2N2222)"| PILOTS_A

    %% Retornos de Masa a Tierra en Estrella
    LAMP1 --> GND_CENTRAL
    LAMP2 --> GND_CENTRAL
    LM_A --> GND_CENTRAL
    LM_B --> GND_CENTRAL
    ESP32_A -.->|"Referencia Señal"| GND_CENTRAL
```

---

## 2. Topología de Red Inalámbrica Híbrida (ESP-NOW + MQTT Broker)

Muestra cómo se comunican las estaciones de campo con el servidor sin colapsar el router Wi-Fi:

```mermaid
flowchart LR
    subgraph Campo_Sensor ["NODOS SENSORES DE CAMPO"]
        NodeA["Estación A (ESP32)<br>Canal Auto-Sincronizado"]
        NodeB["Estación B (ESP32)<br>Canal Auto-Sincronizado"]
    end

    subgraph Gateway_Bridge ["NODO CENTRAL GATEWAY"]
        GW["ESP32 Main Gateway<br>Radio Dual: ESP-NOW + Wi-Fi STA"]
    end

    subgraph Servidor_Docker ["SERVIDOR COORDINADOR (DOCKER)"]
        MOSQ["Broker MQTT Eclipse Mosquitto<br>Puerto 1883 / 1884"]
        COLLECTOR["Daemon Colector (Python)<br>Detección de Anomalías"]
        DB[(Base de Datos SQLite<br>Modo WAL Thread-Safe)]
        DASH["Dashboard Streamlit<br>Puerto 8501 (UI Operador)"]
    end

    %% Enlaces de Radio
    NodeA -->|"ESP-NOW Broadcast / Unicast<br>(Trama binaria 24 bytes, <5 ms)"| GW
    NodeB -->|"ESP-NOW Broadcast / Unicast<br>(Trama binaria 24 bytes, <5 ms)"| GW
    GW -->|"ACK y Comandos Inversos"| NodeA
    GW -->|"ACK y Comandos Inversos"| NodeB

    %% Enlace Wi-Fi / IP
    GW -->|"Publica JSON Telemetría / Suscribe Comandos<br>Wi-Fi 802.11 b/g/n"| MOSQ

    %% Ecosistema Docker
    MOSQ <-->|"Suscripción y Despacho"| COLLECTOR
    COLLECTOR -->|"Escritura de Métricas y Alertas"| DB
    DB <-->|"Lectura Concurrente a 60 FPS"| DASH
    DASH -.->|"Comandos de Operador (Relés ON/OFF)"| MOSQ
```

---

## 3. Flujo del Algoritmo de Aprendizaje Federado (FedAvg)

Ilustra la privacidad de datos: cómo aprenden las estaciones colaborativamente sin enviar sus lecturas crudas:

```mermaid
sequenceDiagram
    autonumber
    participant Coord as Coordinador FedAvg (Docker)
    participant EstA as Estación A (Local Trainer)
    participant EstB as Estación B (Local Trainer)

    Note over Coord: Ronda t = 0: Inicializa Modelo Base (θ_global)
    Coord->>EstA: Distribuye Pesos Iniciales (θ_0)
    Coord->>EstB: Distribuye Pesos Iniciales (θ_0)

    rect rgb(240, 248, 255)
        Note over EstA: Entrenamiento Local con lecturas de Estación A<br>(Los datos crudos nunca salen de A)
        Note over EstB: Entrenamiento Local con lecturas de Estación B<br>(Los datos crudos nunca salen de B)
    end

    EstA->>Coord: Envía únicamente Pesos Actualizados (θ_A, n_A muestras)
    EstB->>Coord: Envía únicamente Pesos Actualizados (θ_B, n_B muestras)

    rect rgb(255, 250, 240)
        Note over Coord: Agregación Matemática (FedAvg):<br>θ_global = (n_A*θ_A + n_B*θ_B) / (n_A + n_B)
    end

    Coord->>EstA: Despliega Modelo Global Optimizado (θ_global)
    Coord->>EstB: Despliega Modelo Global Optimizado (θ_global)
    Note over EstA, EstB: Ambos nodos ahora predicen con conocimiento combinado
```

---

## 4. Máquina de Estados de Seguridad y Protección Activa en el Borde

Muestra la lógica determinista no bloqueante ejecutada en cada ciclo de `loop()` del ESP32:

```mermaid
stateDiagram-v2
    [*] --> Estado_Normal: Sistema Encendido y Calibrado

    Estado_Normal --> Estado_Advertencia: V < 10.5V o V > 13.5V o I > 1.2A
    Estado_Normal --> Disparo_Emergencia: I > 1.8A (Sobrecorriente Crítica) o P > 22W

    state Estado_Normal {
        Piloto_Verde: Piloto Verde Fijo Encendido
        Reles_Activos: Relés en estado solicitado por usuario
        OLED_Ok: OLED muestra 'NORMAL - EN LÍNEA'
    }

    state Estado_Advertencia {
        Piloto_Amarillo: Piloto Amarillo Parpadeante
        Alerta_MQTT: Publica Warning en tópico /alerts
        OLED_Warn: OLED muestra mensaje de advertencia
    }

    Estado_Advertencia --> Estado_Normal: Variables regresan al rango seguro
    Estado_Advertencia --> Disparo_Emergencia: Corriente supera 1.8A

    state Disparo_Emergencia {
        Corte_Relés: CORTE FORZADO DE TODOS LOS RELÉS (INMEDIATO)
        Piloto_Rojo: Piloto Rojo Fijo Encendido
        OLED_Alarma: OLED en Alto Contraste '¡SOBRECORRIENTE!'
        Alerta_Critica: Publica Evento Prioritario a Mosquitto
    }

    Disparo_Emergencia --> Estado_Normal: Comando de rearme 'RESET_TRIP' desde Dashboard
```
