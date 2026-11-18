# ⚡ ARCEUS GRID — Formulación y Planteamiento del Problema

Este documento define la base conceptual, técnica y académica del proyecto **ARCEUS GRID**, detallando el problema de ingeniería que aborda, la justificación de su diseño y sus objetivos formales para sustentación o informe de proyecto.

---

## 1. Contexto General del Problema

En la transición hacia redes eléctricas inteligentes (*Smart Grids*) y microrredes industriales de corriente continua (DC), la gestión eficiente de la energía exige una supervisión en tiempo real a nivel de carga individual y por ramales de distribución de baja tensión.

Tradicionalmente, las instalaciones eléctricas de baja tensión dependen de:
* Dispositivos de protección electromecánicos pasivos (interruptores termomagnéticos y fusibles) que únicamente actúan de forma destructiva o reactiva ante fallas catastróficas (cortocircuitos severos o sobrecargas térmicas prolongadas).
* Medidores de energía convencionales centralizados que registran consumos acumulados periódicos (lectura horaria o mensual), sin capacidad de telemetría de alta frecuencia ni discriminación por ramal.
* Topologías de control cableadas rígidas o basadas en PLCs industriales de alto costo, inaccesibles para microrredes experimentales o bancos didácticos.

---

## 2. Descripción de la Problemática Específica

El desarrollo e instrumentación de microrredes y bancos de distribución eléctrica enfrenta cuatro limitaciones críticas:

### 2.1. Falta de Visibilidad y Granularidad Operativa en Tiempo Real
Los operadores y sistemas de control carecen de información instantánea sobre el estado de cada estación o carga:
* No se detectan fluctuaciones sutiles de tensión (*voltage sags/swells*).
* No se identifican consumos fantasma ni degradación progresiva de cargas.
* Las decisiones de corte o conmutación no están vinculadas a la telemetría dinámica de la corriente y la potencia.

### 2.2. Cuello de Botella y Riesgos de la IA Centralizada Tradicional
Cuando se intenta aplicar analítica avanzada o Inteligencia Artificial en sistemas IoT eléctricos, el enfoque tradicional consiste en enviar todas las mediciones crudas de alta frecuencia a un servidor en la nube:
* **Saturación del ancho de banda:** Múltiples nodos transmitiendo muestras por segundo colapsan las redes inalámbricas locales.
* **Privacidad y seguridad:** La telemetría eléctrica detallada revela patrones de uso confidenciales, hábitos operativos y vulnerabilidades de la infraestructura.
* **Latencia y punto único de fallo:** Si el servidor central o la red caen, se pierde la capacidad de respuesta ante condiciones anómalas.

### 2.3. Interferencias Electromagnéticas y Riesgo de Tierra Común en Hardware
La coexistencia de lógica digital de 3.3V (microcontroladores ESP32), cargas inductivas/resistivas de 12V DC, relés de potencia y sensores I2C de alta sensibilidad suele generar:
* Ruido conducido y radiado por conmutación de cargas inductivas.
* Bucles de masa (*ground loops*) que falsean las lecturas analógicas del sensor de corriente (INA219).
* Reinicios espurios (*brownouts*) del microcontrolador durante la activación simultánea de bobinas de relé.

### 2.4. Limitaciones del Protocolo Wi-Fi en Ambientes Distribuidos
Hacer que cada nodo sensor dependa exclusivamente de una conexión Wi-Fi/IP individual hacia el router:
* Incrementa el tiempo de arranque y reasociación tras cortes de energía.
* Requiere gestionar credenciales y subredes IP en cada nodo terminal.
* Expone a los nodos sensores a congestión en la capa de transporte TCP/IP.

---

## 3. Pregunta de Investigación / Problema de Ingeniería

> **¿Cómo diseñar e implementar una plataforma integral de instrumentación eléctrica (AIoT) de bajo costo, que permita la adquisición en tiempo real de variables eléctricas, el desacople galvánico/eléctrico entre control y potencia, la actuación automatizada ante anomalías y el entrenamiento distribuido de modelos predictivos mediante Aprendizaje Federado sin comprometer la privacidad ni saturar la red?**

---

## 4. Objetivos del Proyecto

### 4.1. Objetivo General
Diseñar, construir y validar el sistema **ARCEUS GRID**, una plataforma experimental de instrumentación, monitoreo y control de cargas eléctricas en baja tensión (12V DC), integrada por nodos sensores basados en ESP32, comunicación inalámbrica híbrida (ESP-NOW + MQTT), control por relés optoacoplados, señalización visual industrial y un sistema coordinador con detección de anomalías y Aprendizaje Federado (**FedAvg**).

### 4.2. Objetivos Específicos

1. **Desarrollar la infraestructura de hardware:**
   * Diseñar una arquitectura eléctrica segura con fuente de 12V DC y reguladores reductores LM2596 (5V para control y 12V para potencia).
   * Implementar aislamiento galvánico en el módulo de relés mediante configuración `JD-VCC / VCC`.
   * Integrar una topología de conexión de tierra en estrella (*Star Ground*) para suprimir bucles de retorno y garantizar la precisión del sensor INA219.
   * Diseñar etapas de conmutación no bloqueantes para lámparas de 12V y pilotos de señalización industrial (Verde: Normal, Rojo: Peligro, Amarillo: Advertencia).

2. **Implementar el firmware embebido modular:**
   * Programar los nodos sensores en ESP32 para la adquisición continua de tensión, corriente y potencia mediante el sensor INA219 sobre bus I2C (0x40).
   * Incorporar visualización local en pantallas OLED SSD1306 (0x3C) con refresco optimizado y visualización de métricas y alarmas.
   * Diseñar un protocolo binario de alta velocidad sobre **ESP-NOW** para enlazar los nodos sensores con un ESP32 Gateway central sin saturar la red Wi-Fi.
   * Programar en el Gateway la traducción bidireccional entre la trama ESP-NOW y mensajes JSON sobre tópicos MQTT estandarizados.

3. **Construir el backend coordinador y analítica en Docker:**
   * Desplegar mediante Docker Compose un ecosistema de microservicios: Broker MQTT Eclipse Mosquitto, Daemon Colector en Python y Dashboard en Streamlit.
   * Gestionar la persistencia de series temporales de telemetría y eventos de alarma en SQLite con soporte de concurrencia thread-safe en modo WAL (*Write-Ahead Logging*).
   * Implementar un motor de detección determinista de anomalías (sobrecorriente >1.8A, subtensión <10.5V, sobretensión >13.5V, sobrepotencia >22W).

4. **Experimentar con Inteligencia Artificial Distribuida (Aprendizaje Federado):**
   * Implementar el algoritmo **FedAvg** (*Federated Averaging*) para entrenar colaborativamente un modelo de predicción de consumo eléctrico entre la Estación A y la Estación B.
   * Demostrar que los modelos locales aprenden sus propios patrones de carga sin compartir datos crudos, preservando el ancho de banda y la privacidad de cada estación.

---

## 5. Justificación del Proyecto

* **Técnica:** Demuestra la viabilidad de integrar protocolos de radio de capa 2 (ESP-NOW) con middlewares IoT industriales (MQTT), resolviendo los problemas típicos de ruido eléctrico, tierras cruzadas y picos inductivos mediante buenas prácticas de ingeniería de hardware.
* **Científica y de Vanguardia:** Acerca el concepto de **Aprendizaje Federado (Edge AI / AIoT)** al mundo de los sistemas embebidos de bajo costo, superando la limitación del procesamiento centralizado.
* **Económica y Didáctica:** Todo el sistema se construye con componentes comerciales accesibles (ESP32, INA219, relés, fuentes conmutadas estándar), proporcionando una plataforma replicable tanto para la docencia universitaria como para prototipos industriales.

---

## 6. Alcance y Delimitación

* **Tensión de Operación:** Banco experimental en corriente continua a 12V DC nominales, con cargas de prueba consistentes en 4 bombillas LED de 12V distribuidas en 2 estaciones.
* **Nodos Físicos:** 2 Estaciones sensoras (Estación A y Estación B) + 1 ESP32 Gateway central + 1 Servidor Coordinador en PC anfitrión.
* **Entorno de Red:** Red de área local privada con canal Wi-Fi sincronizado automáticamente entre Gateway y broker Docker.
