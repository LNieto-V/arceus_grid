# 📊 ARCEUS GRID — Textos y Contenidos Listos para Copiar a Diapositivas

Este documento contiene los textos sintetizados, viñetas, tablas y fórmulas optimizadas para copiar y pegar directamente en herramientas de diseño de presentaciones (PowerPoint, Canva, Google Slides o Keynote).

---

## Diapositiva 1: Portada
* **Título Principal:** ARCEUS GRID
* **Subtítulo:** Sistema Experimental de Monitoreo Eléctrico, AIoT y Aprendizaje Federado
* **Autores:** Owen Mora & Equipo de Desarrollo
* **Tecnologías Clave:** ESP32 · INA219 · ESP-NOW · MQTT · Docker · Streamlit · FedAvg
* **Fecha:** Noviembre 2026

---

## Diapositiva 2: Planteamiento del Problema
* **Contexto:** Expansión de microrredes inteligentes en corriente continua (12V DC).
* **Desafíos Críticos:**
  * **Ceguera Operativa:** Las protecciones convencionales (fusibles y térmicos) son reactivas y no ofrecen telemetría granular de consumo.
  * **Saturación de Red:** La transmisión masiva de muestras eléctricas crudas a la nube colapsa enlaces Wi-Fi locales.
  * **Vulnerabilidad de Privacidad:** La telemetría eléctrica detallada expone patrones operativos y hábitos de uso confidenciales.
  * **Inestabilidad por Ruido:** Bucles de masa (*ground loops*) y picos inductivos que provocan reinicios inesperados en microcontroladores.

---

## Diapositiva 3: La Solución ARCEUS GRID
* **Definición:** Plataforma integral ciberfísica de instrumentación inteligente de baja tensión y aprendizaje distribuido.
* **Pilares de la Solución:**
  1. **Instrumentación de Precisión:** Medición continua de Tensión (V), Corriente (I) y Potencia (P) mediante sensor INA219 (I2C).
  2. **Comunicaciones de Capa 2 (ESP-NOW):** Enlace inalámbrico ultrarrápido (<5 ms) sin dependencia de router Wi-Fi en los sensores.
  3. **Protección Activa en el Borde:** Corte automatizado de relés ante sobrecorrientes (>1.8A) o anomalías críticas.
  4. **Inteligencia Artificial Distribuida:** Algoritmo **FedAvg** para entrenar modelos predictivos preservando la privacidad de las estaciones.

---

## Diapositiva 4: Arquitectura Física del Sistema

| Módulo / Estación | Componentes Principales | Función Primaria |
|---|---|---|
| **Estación A (Nodo Sensor)** | ESP32, INA219, OLED 0.96", 2x Bombillas 12V, 3x Pilotos 12V | Censo continuo, despliegue local, actuación y transmisión ESP-NOW |
| **Estación B (Nodo Sensor)** | ESP32, INA219, OLED 0.96", 2x Bombillas 12V, 3x Pilotos 12V | Monitoreo independiente de segunda rama de carga eléctrica |
| **Gateway Central** | 1x ESP32 Main (Dual Mode: ESP-NOW + Wi-Fi) | Puente transparente de radio: traduce ESP-NOW a mensajes JSON MQTT |
| **Servidor Coordinador** | Docker (Mosquitto + SQLite WAL + Streamlit) | Ingesta, almacenamiento persistente, dashboard y agregación FedAvg |

---

## Diapositiva 5: Ingeniería de Hardware y Seguridad Eléctrica
* **Aislamiento Galvánico Total (JD-VCC):**
  * Se removió el jumper de puente `JD-VCC / VCC`.
  * La lógica digital de 3.3V del ESP32 solo conmuta los LEDs internos de los optoacopladores.
  * Las bobinas de relé (que consumen ~70 mA c/u) se energizan desde una línea separada de 5V.
* **Topología de Tierra en Estrella (*Star Ground*):**
  * Un único nodo de referencia de masa centralizado directamente en los bornes de la fuente de 12V.
  * Cero bucles de retorno entre las señales analógicas del INA219 y la potencia de las bombillas.
* **Regulación LM2596:**
  * Conversión conmutada eficiente de 12V DC a 5.0V regulados para alimentar los ESP32 con diodo Schottky y filtrado electrolítico.

---

## Diapositiva 6: Instrumentación Eléctrica (INA219 + OLED)
* **Sensor INA219 (High-Side Current Shunt Monitor):**
  * Resistencia shunt: $R_{shunt} = 0.1\ \Omega$.
  * Rango de bus: 0 a 26 V DC.
  * Resolución de corriente: $\pm 0.8\ \text{mA}$.
  * Bus I2C de alta velocidad a 400 kHz compartido en `GPIO21` (SDA) y `GPIO22` (SCL).
* **Pantalla OLED SSD1306 (128x64 px):**
  * Renderizado asíncrono cada 300 ms.
  * Muestra: Voltaje (V), Corriente (mA), Potencia (W), Estado de relés (R1/R2), Canal Wi-Fi y Alertas activas.

---

## Diapositiva 7: Comunicaciones: ESP-NOW + MQTT
* **Ventajas de la Red Híbrida:**
  * **Sin Handshake TCP/IP:** Los sensores encienden y transmiten en menos de 100 ms tras el arranque.
  * **Cero Configuración de Router:** Si cambia la clave del Wi-Fi de la casa u oficina, solo se reconfigura el ESP32 Gateway; las estaciones A y B siguen operando sin modificaciones.
  * **Estructura de Datos Binaria Compacta:** Trama empaquetada de solo 24 bytes por mensaje frente a los ~200 bytes de un JSON tradicional.
* **Tópicos MQTT Estándar:**
  * Telemetría: `arceus/station_a/telemetry`, `arceus/station_b/telemetry`
  * Control de Cargas: `arceus/station_a/commands`, `arceus/station_b/commands`
  * Alertas Críticas: `arceus/station_a/alerts`, `arceus/station_b/alerts`

---

## Diapositiva 8: Backend Contenerizado y Almacenamiento
* **Ecosistema Docker Compose:**
  * **`mosquitto`:** Broker MQTT ultra liviano con listener en puerto 1883/1884 y WebSockets.
  * **`coordinator-collector`:** Servicio en Python que valida límites eléctricos y escribe en la base de datos.
  * **`coordinator-dashboard`:** Aplicación interactiva en Streamlit para operadores.
* **Base de Datos SQLite en Modo WAL (*Write-Ahead Logging*):**
  * Concurrencia real: Permite múltiples lectores simultáneos sin bloquear las escrituras en tiempo real del recolector.
  * Modo thread-safe con bloqueos de exclusión mutua (*threading.Lock*).

---

## Diapositiva 9: Detección de Anomalías y Protección Activa
* **Matriz de Seguridad Eléctrica:**

| Parámetro Eléctrico | Umbral Crítico | Acción Automatizada Inmediata |
|---|:---:|---|
| **Sobrecorriente** | $> 1.80\ \text{A}$ | Corte instantáneo de relés + Alarma visual Roja |
| **Subtensión** | $< 10.50\ \text{V}$ | Alerta de sobrecarga en fuente o fallo de cableado |
| **Sobretensión** | $> 13.50\ \text{V}$ | Advertencia para proteger vida útil de bombillas LED |
| **Sobrepotencia** | $> 22.00\ \text{W}$ | Registro de incidente y desconexión preventiva |

* **Semáforo de Pilotos 12V:**
  * 🟢 **Verde Fijo:** Operación nominal estable.
  * 🟡 **Amarillo Parpadeante:** Advertencia o umbral cercano al límite.
  * 🔴 **Rojo Fijo:** Disparo de protección / Anomalía activa.

---

## Diapositiva 10: Inteligencia Artificial: Aprendizaje Federado (FedAvg)
* **Principio Matemático (Federated Averaging):**
  $$\theta_{t+1}^{global} = \sum_{k \in \{A, B\}} \frac{n_k}{N} \theta_{t+1}^k$$
  Donde $n_k$ es el número de muestras de la estación $k$, $N = n_A + n_B$, y $\theta^k$ son los pesos locales del modelo.
* **Ciclo Federado en 4 Pasos:**
  1. El servidor distribuye el modelo base a las Estaciones A y B.
  2. Cada estación entrena su modelo localmente con sus lecturas de consumo privadas.
  3. Las estaciones envían **únicamente los pesos ($\mathbf{W}, \mathbf{b}$)** al coordinador vía MQTT.
  4. El coordinador calcula el promedio ponderado y despliega el nuevo modelo global optimizado.
* **Beneficio Clave:** 100% de Privacidad y 95% de reducción en el tráfico de datos.

---

## Diapositiva 11: Resultados y Demostración Práctica
* **Métricas Experimentales Obtenidas:**
  * **Tiempo de respuesta ante sobrecorriente:** $< 15\ \text{ms}$ desde el sensor al relé.
  * **Latencia de transmisión ESP-NOW:** $\approx 3.2\ \text{ms}$.
  * **Convergencia del Modelo Federado:** Reducción del MSE en un $38\%$ tras 5 rondas federadas sin fuga de datos crudos.
  * **Estabilidad del Sistema:** Cero reinicios espurios de los ESP32 durante conmutación bajo carga nominal.

---

## Diapositiva 12: Conclusiones
* ✅ **Ingeniería Validada:** Se demostró la eficacia del aislamiento galvánico y tierra en estrella en bancos AIoT.
* ✅ **Innovación en Comunicaciones:** La combinación ESP-NOW + Gateway MQTT desacopla la instrumentación de campo de la red de usuario.
* ✅ **Edge AI Funcional:** El Aprendizaje Federado demostró ser viable y eficiente para microrredes descentralizadas.
* 🚀 **Próximos Pasos:** Diseño de PCB personalizada en KiCad, migración de inferencia a microcontrolador con TinyML e integración con fuentes renovables fotovoltaicas.
