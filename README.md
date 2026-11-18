# ⚡ ARCEUS GRID — Sistema Experimental de Monitoreo Eléctrico y AIoT

[![CI](https://github.com/LNieto-V/arceus_grid/actions/workflows/ci.yml/badge.svg)](https://github.com/LNieto-V/arceus_grid/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python: 3.11+](https://img.shields.io/badge/Python-3.11+-blue.svg)](https://www.python.org/)
[![Docker Compose](https://img.shields.io/badge/Docker-Compose-2496ED?logo=docker&logoColor=white)](https://www.docker.com/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange.svg?logo=platformio)](https://platformio.org/)
[![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow.svg)](https://conventionalcommits.org)

**ARCEUS GRID** es una plataforma de instrumentación inteligente de consumo eléctrico compuesta por dos estaciones independientes (Estación A y Estación B) basadas en **ESP32** y un **Coordinador Central** orquestado en Docker que integra comunicación MQTT, base de datos SQLite, detección de anomalías eléctricas, panel de control en Streamlit y **Aprendizaje Federado (Federated Learning mediante FedAvg)**.

---

## 📁 Estructura del Repositorio y Enlaces Directos

```
arceus_grid/
├── diapositivas/                   # Material Exclusivo de Exposición y Sustentación
│   ├── 01_PLANTEAMIENTO_DEL_PROBLEMA.md  # Planteamiento formal del problema de ingeniería
│   ├── 02_ESTRUCTURA_Y_GUION_PRESENTACION.md # Guion y speech diapositiva por diapositiva
│   ├── 03_CONTENIDO_DIAPOSITIVAS.md      # Textos y viñetas listos para PowerPoint/Canva
│   ├── 04_DIAGRAMAS_VISUALES.md          # Diagramas Mermaid (Hardware, ESP-NOW, FedAvg)
│   ├── 05_GUIA_DEMO_EN_VIVO.md           # Procedimiento paso a paso para la demo en vivo
│   └── diapositivas_marp.md              # Presentación ejecutable en Marp / Markdown
├── docker-compose.yml              # Orquestador multi-contenedor (Broker, Colector, Dashboard)
├── mosquitto/                      # Broker MQTT Eclipse Mosquitto
│   ├── Dockerfile
│   └── config/mosquitto.conf
├── coordinator/                    # Sistema Central de Coordinación (Python)
│   ├── Dockerfile
│   ├── requirements.txt
│   ├── config.py                   # Configuración y umbrales eléctricos de alarma
│   ├── main.py                     # Servicio daemon de ingesta MQTT y anomalías
│   ├── database/
│   │   └── db_manager.py           # Gestor SQLite thread-safe con modo WAL
│   ├── ai/
│   │   ├── anomaly_detector.py     # Detección determinista y estadística de anomalías
│   │   └── federated_learning.py   # Algoritmo FedAvg y modelo local de regresión/red
│   ├── dashboard/
│   │   └── app.py                  # Panel interactivo Streamlit con gráficos Plotly
│   └── mqtt/
│       └── client.py               # Cliente MQTT Paho con reconexión y dispatch de comandos
├── firmware/                       # Código embebido ESP32 (PlatformIO)
│   ├── platformio.ini              # Entornos: station_a, station_b, stage1_didactic_test
│   ├── stage1_test/                # Etapa 1: Banco didáctico 3 LEDs + OLED + Serial
│   │   └── src/main.cpp
│   └── arceus_node/                # Firmware modular de producción para Estaciones A y B
│       ├── include/
│       │   ├── config.h            # Asignación de pines, tópicos MQTT, credenciales
│       │   ├── sensors_ina219.h    # Driver del sensor de corriente/tensión/potencia
│       │   ├── display_oled.h      # Driver de pantalla OLED SSD1306 128x64
│       │   ├── relays.h            # Controlador no bloqueante de relés
│       │   ├── pilots.h            # Máquina de estados de pilotos 12V (millis)
│       │   └── mqtt_manager.h      # Cliente MQTT embebido (PubSubClient + ArduinoJson)
│       └── src/
│           ├── main.cpp
│           ├── sensors_ina219.cpp
│           ├── display_oled.cpp
│           ├── relays.cpp
│           ├── pilots.cpp
│           └── mqtt_manager.cpp
├── hardware/                       # Ingeniería Eléctrica y Memorias de Cálculo
│   ├── schematics/
│   │   ├── electrical_architecture.md      # Topología de potencia, LM2596 y Star GND
│   │   ├── pilots_driver_analysis.md       # Análisis técnico de etapa 2N2222 / MOSFET
│   │   ├── relays_isolation_guide.md       # Aislamiento JD-VCC / VCC y lógica activa
│   │   └── wiring_diagram_station_a_b.md   # Tablas completas de cableado pin-a-pin
│   └── calculations/
│       └── power_and_thermal_sizing.md     # Balance de potencia, fusibles, AWG y calor
└── scripts/
    ├── simulate_stations.py        # Simulador hardware completo para pruebas inmediatas
    └── setup_local.sh              # Despliegue en entorno virtual local sin Docker
```

---

## 🚀 Puesta en Marcha Rápida (Despliegue)

### Opción 1: Despliegue con Docker Compose (Recomendado)
Para levantar el Broker MQTT Mosquitto, el recolector de telemetría y el Dashboard Streamlit en un solo paso:

```bash
cd /home/tensei/Proyectos/arceus_grid
docker compose up --build -d
```

Una vez levantado:
- **Dashboard Streamlit:** [http://localhost:8501](http://localhost:8501)
- **Broker MQTT:** `localhost:1883` (y WebSockets en `9001`)

### Opción 2: Despliegue Local sin Docker (Script automatizado)
Si prefieres ejecutar directamente en el sistema operativo anfitrión:

```bash
cd /home/tensei/Proyectos/arceus_grid
./scripts/setup_local.sh
```

---

## 🧪 Validación Inmediata con el Simulador de Estaciones

Para verificar todo el pipeline (MQTT, SQLite, gráficos en tiempo real, conmutación de relés y rondas de FedAvg) sin necesidad de tener conectadas físicamente las placas ESP32:

1. Inicia el broker y los servicios (vía Docker o local).
2. En una terminal ejecuta:
   ```bash
   python scripts/simulate_stations.py
   ```
3. El simulador publicará telemetría real de las Estaciones A y B cada segundo y responderá en vivo cuando enciendas o apagues las bombillas desde el Dashboard de Streamlit.

---

## 🎤 Material Exclusivo para Sustentación y Diapositivas

Todo el material para la exposición oral y presentación del proyecto se encuentra organizado en la carpeta [`diapositivas/`](diapositivas/):

1. 📄 [Formulación y Planteamiento del Problema](diapositivas/01_PLANTEAMIENTO_DEL_PROBLEMA.md) — Base formal, contexto, problema central y objetivos de ingeniería.
2. 🗣️ [Estructura y Guion de Exposición](diapositivas/02_ESTRUCTURA_Y_GUION_PRESENTACION.md) — Planificación diapositiva por diapositiva con speech verbal del expositor.
3. 📊 [Textos Listos para Diapositivas](diapositivas/03_CONTENIDO_DIAPOSITIVAS.md) — Viñetas, tablas y fórmulas optimizadas para PowerPoint o Canva.
4. 📐 [Diagramas Visuales Mermaid](diapositivas/04_DIAGRAMAS_VISUALES.md) — Diagramas de potencia, comunicaciones, FedAvg y máquina de estados.
5. 🚀 [Guía Paso a Paso para la Demo en Vivo](diapositivas/05_GUIA_DEMO_EN_VIVO.md) — Procedimiento de demostración práctica ante jurados y evaluadores.
6. 📽️ [Presentación Marp / Markdown](diapositivas/diapositivas_marp.md) — Deck de diapositivas listo para proyectar o exportar a PDF/HTML.

---

## 📑 Enlaces Rápidos a la Documentación de Hardware e Ingeniería

1. [Arquitectura Eléctrica y Tierra en Estrella](file:///home/tensei/Proyectos/arceus_grid/hardware/schematics/electrical_architecture.md)
2. [Estudio y Análisis de la Etapa de Pilotos 12V (2N2222 + MOSFET)](file:///home/tensei/Proyectos/arceus_grid/hardware/schematics/pilots_driver_analysis.md)
3. [Guía de Aislamiento Óptico de Relés (JD-VCC / VCC)](file:///home/tensei/Proyectos/arceus_grid/hardware/schematics/relays_isolation_guide.md)
4. [Diagrama de Cableado Pin-a-Pin Estaciones A y B](file:///home/tensei/Proyectos/arceus_grid/hardware/schematics/wiring_diagram_station_a_b.md)
5. [Memoria de Cálculo de Potencia, Calibres AWG y Térmica](file:///home/tensei/Proyectos/arceus_grid/hardware/calculations/power_and_thermal_sizing.md)

---

## 📜 Historial de Chat y Especificaciones del Proyecto

- [Prompt Maestro Canónico de ARCEUS GRID](file:///home/tensei/Proyectos/arceus_grid/docs/chat_history/PROMPT_MAESTRO_ARCEUS_GRID.md)
- [Transcripción Completa de la Sesión de Chat](file:///home/tensei/Proyectos/arceus_grid/docs/chat_history/CHAT_TRANSCRIPT.md)
- [Log de Telemetría del Chat en JSONL](file:///home/tensei/Proyectos/arceus_grid/docs/chat_history/transcript.jsonl)


---

## 📡 Topología Inalámbrica con ESP-NOW (1x Gateway Main + 2x Nodos Sensores)

El proyecto incluye soporte nativo para operar con **3 placas ESP32** mediante **ESP-NOW**:
- **ESP32 Main / Gateway:** Se conecta al Wi-Fi y a Mosquitto; hace de puente bidireccional entre la radio ESP-NOW y MQTT.
  - Sketch Arduino IDE: [espnow_gateway.ino](firmware/arduino/espnow_gateway/espnow_gateway.ino)
- **ESP32 Nodos Sensores (Estación A y B):** Miden tensión/corriente con INA219, gobiernan relés y pilotos 12V, y transmiten a alta velocidad por ESP-NOW (sin necesidad de conectarse a la red Wi-Fi).
  - Sketch Arduino IDE: [espnow_station_node.ino](firmware/arduino/espnow_station_node/espnow_station_node.ino)
- **Documentación Completa y Protocolo:**
  - [Guía de Arquitectura y Flasheo ESP-NOW](docs/ESPNOW_ARCHITECTURE.md)
  - [Definición del Protocolo Binario](firmware/espnow_protocol.h)

---

## 🔧 Compilación y Flasheo del Firmware ESP32

El proyecto incluye configuración para **PlatformIO**:

- **Flashear Estación A:**
  ```bash
  pio run -e station_a -t upload
  ```
- **Flashear Estación B:**
  ```bash
  pio run -e station_b -t upload
  ```
- **Flashear Banco Didáctico Inicial (Etapa 1 - LEDs + OLED):**
  ```bash
  pio run -e stage1_didactic_test -t upload
  ```
- **Monitor Serial:**
  ```bash
  pio device monitor -b 115200
  ```

---

## 🌐 Publicación y Vinculación con GitHub

Para vincular este repositorio local con tu cuenta de GitHub (`LNieto-V`):

1. Crea un nuevo repositorio en [GitHub](https://github.com/new) con el nombre **`arceus_grid`** (dejarlo vacío, sin README inicial).
2. Ejecuta en la terminal:
   ```bash
   ./scripts/publish_to_github.sh
   ```
   *O manualmente:*
   ```bash
   git remote add origin git@github.com:LNieto-V/arceus_grid.git
   git branch -M main
   git push -u origin main
   ```

