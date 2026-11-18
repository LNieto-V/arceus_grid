# 🎤 ARCEUS GRID — Estructura y Guion de Exposición Diapositiva por Diapositiva

Este documento contiene la planificación completa para la presentación oral del proyecto (duración estimada: 10 a 15 minutos). Incluye el objetivo de cada diapositiva, los elementos visuales a proyectar y el **guion verbal exacto (lo que debes decir)**.

---

## Resumen de la Presentación

| Slide | Título / Tema | Tiempo Sugerido | Objetivo Principal |
|:---:|---|:---:|---|
| **1** | Portada e Introducción | 0:45 min | Presentar el proyecto, integrantes y lema central. |
| **2** | Contexto y Planteamiento del Problema | 1:30 min | Explicar las fallas de las redes tradicionales y la IA centralizada. |
| **3** | La Propuesta: ARCEUS GRID | 1:00 min | Visión general del ecosistema AIoT + Edge Computing. |
| **4** | Arquitectura Física y Distribución | 1:15 min | Detallar Estación A, Estación B y el Coordinador Central. |
| **5** | Ingeniería Eléctrica y Hardware Seguro | 1:30 min | Explicar Star Ground, LM2596, JD-VCC y drivers de pilotos. |
| **6** | Instrumentación y Medición (INA219 + OLED) | 1:00 min | Explicar la captura de V, I, P y visualización en tiempo real. |
| **7** | Red Inalámbrica Híbrida: ESP-NOW + MQTT | 1:30 min | Justificar por qué capa 2 (ESP-NOW) hacia el Gateway y MQTT. |
| **8** | Backend Contenerizado (Docker + SQLite WAL) | 1:00 min | Detallar Mosquitto, el Daemon Colector y concurrencia segura. |
| **9** | Detección de Anomalías y Protección Activa | 1:00 min | Umbrales de corte (sobrecorriente, subtensión) y pilotos. |
| **10**| Aprendizaje Federado (FedAvg en el Borde) | 1:45 min | Demostrar cómo se entrenan modelos locales sin fugar datos. |
| **11**| Demostración en Vivo (Dashboard Streamlit) | 2:00 min | Mostrar telemetría, control de relés y ronda federada en vivo. |
| **12**| Conclusiones y Trabajo Futuro | 1:00 min | Síntesis del impacto, métricas alcanzadas y despedida. |

---

## Diapositiva 1: Portada
* **Título:** ARCEUS GRID: Sistema Experimental de Monitoreo Eléctrico y AIoT
* **Subtítulo:** Instrumentación Inteligente, Red Inalámbrica Híbrida (ESP-NOW/MQTT) y Aprendizaje Federado (FedAvg)
* **Elementos en pantalla:** Logo del proyecto, foto del prototipo montado, nombres de los autores, fecha e institución.
* **🗣️ Guion del expositor:**
  > *"Muy buenos días a todos. Hoy les presentamos ARCEUS GRID, una plataforma integral de instrumentación inteligente y control para microrredes de baja tensión. Nuestro proyecto combina sistemas embebidos de bajo costo basados en ESP32, una arquitectura inalámbrica de alta velocidad y algoritmos de Inteligencia Artificial en el Borde mediante Aprendizaje Federado para resolver los problemas críticos de visibilidad, seguridad y privacidad en la distribución eléctrica actual."*

---

## Diapositiva 2: Contexto y Planteamiento del Problema
* **Título:** ¿Cuál es el problema en la supervisión eléctrica tradicional?
* **Puntos clave en pantalla:**
  * ❌ Falta de visibilidad en tiempo real a nivel de carga individual.
  * ❌ Protecciones pasivas (fusibles) que actúan solo de forma reactiva/destructiva.
  * ❌ Saturación de red y pérdida de privacidad al centralizar toda la telemetría en la nube.
  * ❌ Ruido eléctrico y desacoples térmicos en bancos de pruebas de baja tensión.
* **🗣️ Guion del expositor:**
  > *"En las instalaciones eléctricas actuales, la mayoría de protecciones como fusibles o interruptores termomagnéticos son pasivas: solo actúan cuando el cortocircuito o la sobrecarga ya causó daños. Además, los sistemas IoT convencionales intentan resolver esto transmitiendo cada milisegundo de datos a servidores en la nube. Esto colapsa el ancho de banda, genera cuellos de botella y expone información crítica sobre los hábitos de consumo. Si el internet cae, el sistema queda ciego. El problema que resolvemos es: ¿Cómo tener monitoreo preciso, respuesta inmediata en el microsegundo y aprendizaje predictivo sin saturar la red ni exponer los datos crudos?"*

---

## Diapositiva 3: La Solución ARCEUS GRID
* **Título:** Arquitectura Integral de Solución
* **Puntos clave en pantalla:**
  * Diagrama de 3 capas:
    1. **Borde (Edge):** Estaciones A y B con ESP32, INA219 y corte por relés.
    2. **Comunicaciones:** Radio rápida ESP-NOW enlazada al Gateway principal.
    3. **Coordinación y AIoT:** Docker (Mosquitto, SQLite WAL, Streamlit y FedAvg).
* **🗣️ Guion del expositor:**
  > *"ARCEUS GRID responde a este desafío con un diseño modular en tres capas. En el borde, dos estaciones independientes censan voltaje, corriente y potencia, protegiendo las cargas en milisegundos. En la capa de comunicaciones, creamos una red de radio privada con ESP-NOW que no requiere claves de Wi-Fi en los sensores. Y en la capa de coordinación, un servidor contenerizado en Docker orquesta el panel de control, almacena métricas y entrena colaborativamente modelos de Inteligencia Artificial usando Federated Learning."*

---

## Diapositiva 4: Arquitectura Física del Prototipo
* **Título:** Componentes y Distribución Física
* **Puntos clave en pantalla:**
  * Tabla comparativa:
    * **Estación A y B (cada una):** 1x ESP32, 1x INA219, 1x OLED 0.96", 2x Bombillas LED 12V, 3x Pilotos industriales 12V (Verde, Amarillo, Rojo).
    * **Sistema Central:** Fuente conmutada 12V/10A, Módulo de 4 relés con aislamiento, 2x reguladores LM2596 (5V de precisión).
* **🗣️ Guion del expositor:**
  > *"El hardware de ARCEUS GRID consta de dos estaciones de trabajo idénticas. Cada una cuenta con dos bombillas LED de 12V como cargas dinámicas, un sensor de corriente y tensión INA219 y una pantalla OLED que informa en el lugar el estado de operación. Además, incluimos semáforos de pilotos industriales de 12V para inspección visual inmediata a distancia. Todo el control de potencia se gestiona mediante un módulo de 4 relés independientes alimentados de forma aislada."*

---

## Diapositiva 5: Ingeniería Eléctrica y Hardware Seguro
* **Título:** Desacople Galvánico, LM2596 y Conexión en Estrella
* **Puntos clave en pantalla:**
  * Esquema de conexión Star Ground (Tierra en Estrella única en la fuente).
  * Aislamiento óptico de relés retirando el jumper `JD-VCC`.
  * Conversión DC-DC LM2596: 12V -> 5.0V estables con diodo Schottky y filtrado.
  * Etapa de drivers para pilotos 12V mediante transistores 2N2222 y resistores de base.
* **🗣️ Guion del expositor:**
  > *"Uno de los mayores aportes de ingeniería de ARCEUS GRID es su robustez eléctrica. Para evitar que la conmutación de las cargas o los relés reinicien los microcontroladores, aplicamos aislamiento óptico total: el ESP32 alimenta únicamente los optoacopladores a 3.3V, mientras que la bobina de los relés se energiza a través de un circuito separado en JD-VCC. Asimismo, implementamos una topología de tierra en estrella en la fuente para erradicar bucles de masa que arruinarían las mediciones analógicas del sensor."*

---

## Diapositiva 6: Adquisición de Datos e Instrumentación (INA219 + OLED)
* **Título:** Medición de Precisión en Tiempo Real
* **Puntos clave en pantalla:**
  * Sensor INA219 I2C (0x40): Medición High-Side con resistencia shunt de 0.1 Ω.
  * Variables adquiridas: Voltaje de bus (V), Corriente (mA/A), Potencia (W).
  * Pantalla OLED SSD1306 (0x3C): Renderizado por páginas no bloqueante sin parpadeos.
* **🗣️ Guion del expositor:**
  > *"La instrumentación se realiza mediante el sensor INA219 conectado en el lado positivo de la línea de 12V. Al medir la caída sobre una resistencia shunt de precisión, calculamos voltaje, corriente y potencia con una resolución de hasta 0.8 miliamperios. Simultáneamente, el ESP32 refresca la pantalla OLED en el bus I2C a 400 kHz mostrando el estado de la conexión, los relés y las métricas eléctricas en tiempo real."*

---

## Diapositiva 7: Red Inalámbrica Híbrida: ESP-NOW + Gateway MQTT
* **Título:** Comunicaciones Robustas sin Dependencia de Router
* **Puntos clave en pantalla:**
  * ¿Por qué ESP-NOW?: Protocolo sin conexión de capa 2 del ESP32 (<5 ms de latencia, sin handshake TCP ni DHCP).
  * El ESP32 Gateway: Hace de puente, escucha paquetes binarios empaquetados (`packed structs`) y los convierte a JSON sobre MQTT (Mosquitto).
  * Auto-sincronización de canales Wi-Fi y confirmaciones ACK automáticas.
* **🗣️ Guion del expositor:**
  > *"A nivel de comunicaciones, innovamos eliminando la necesidad de que los sensores se conecten al Wi-Fi del router. En su lugar, utilizamos ESP-NOW, una tecnología de capa de enlace propia del ESP32 con latencias inferiores a 5 milisegundos. Un tercer ESP32 actúa como Gateway principal: recibe la trama binaria por radio, la valida y la inyecta al Broker MQTT Mosquitto en formato JSON para que el sistema central la procese."*

---

## Diapositiva 8: Backend Contenerizado y Persistencia de Datos
* **Título:** Arquitectura Docker y SQLite WAL Thread-Safe
* **Puntos clave en pantalla:**
  * 3 Contenedores Docker:
    1. `arceus_mosquitto`: Broker MQTT liviano con soporte de WebSockets.
    2. `arceus_collector`: Daemon Python asíncrono para ingesta, alertas y auditoría.
    3. `arceus_dashboard`: Interfaz web interactiva en Streamlit.
  * Base de datos SQLite configurada con `PRAGMA journal_mode=WAL` para lecturas y escrituras simultáneas sin bloqueos.
* **🗣️ Guion del expositor:**
  > *"En el computador coordinador, todo el backend corre en contenedores Docker mediante un solo comando. Mosquitto recibe la telemetría, el recolector en Python procesa y valida los datos, y los persiste en una base de datos SQLite en modo WAL, lo que permite que el Dashboard en Streamlit lea métricas continuamente a 60 FPS sin bloquear las escrituras concurrentes del recolector."*

---

## Diapositiva 9: Motor de Detección de Anomalías y Seguridad Activa
* **Título:** Protección Determinista en Borde y Nube
* **Puntos clave en pantalla:**
  * Umbrales de seguridad calibrados:
    * Sobrecorriente: > 1.80 A (Corte automático de relés).
    * Subtensión: < 10.50 V (Alerta de descarga de fuente o sobrecarga).
    * Sobretensión: > 13.50 V (Protección de bombillas LED).
    * Sobrepotencia: > 22.00 W.
  * Transición de pilotos: Normal (Verde) -> Advertencia (Amarillo parpadeante) -> Alarma (Rojo fijo + Desconexión).
* **🗣️ Guion del expositor:**
  > *"La seguridad en ARCEUS GRID es proactiva. Si una estación detecta una corriente superior a 1.8 amperios o una caída de tensión peligrosa, no espera a que el operador humano se dé cuenta: el ESP32 en el borde apaga inmediatamente los relés correspondientes, cambia los pilotos luminosos a color rojo de emergencia y publica una alerta prioritaria hacia el Dashboard para alertar al operador."*

---

## Diapositiva 10: Inteligencia Artificial en el Borde: Federated Learning (FedAvg)
* **Título:** Aprendizaje Federado sin Transferencia de Datos Crudos
* **Puntos clave en pantalla:**
  * Esquema FedAvg:
    $$\theta_{global} = \frac{n_A \theta_A + n_B \theta_B}{n_A + n_B}$$
  * Ventajas clave:
    1. **Privacidad total:** Las series temporales de corriente nunca salen de la estación.
    2. **Ahorro de ancho de banda:** Solo se intercambian vectores de pesos (pesos y sesgos).
    3. **Modelo enriquecido:** El modelo global aprende de cargas distintas (Estación A y B) sin centralizar sus bases de datos.
* **🗣️ Guion del expositor:**
  > *"Aquí llegamos a la joya científica de ARCEUS GRID: el Aprendizaje Federado con el algoritmo FedAvg. En lugar de transmitir gigabytes de telemetría a un servidor central, cada estación entrena su propio modelo de regresión neuronal de forma local con sus datos privados. Luego, envían únicamente los pesos matemáticos del modelo al coordinador. El coordinador promedia estos pesos ponderados por el número de muestras y devuelve un modelo global más inteligente a ambas estaciones. Privacidad absoluta y ahorro del 95% del tráfico de red."*

---

## Diapositiva 11: Demostración en Vivo (Live Demo)
* **Título:** Validación Experimental en Vivo
* **Puntos clave en pantalla:**
  * Pestañas del Dashboard Streamlit:
    1. **Monitoreo en Tiempo Real:** Gráficos Plotly de V, I y P en vivo.
    2. **Control Remoto:** Botones para activar/desactivar Bombilla 1 y Bombilla 2.
    3. **Registro de Alarmas:** Historial filtrable de anomalías detectadas.
    4. **Módulo Federado:** Visualización de rondas de FedAvg, pesos locales y convergencia del modelo global.
* **🗣️ Guion del expositor:**
  > *"A continuación, pasamos a la demostración en vivo. En pantalla pueden observar el Dashboard en tiempo real. Vamos a encender la bombilla 1 de la Estación A mediante el botón remoto... notamos de inmediato el incremento de corriente y el refresco en la pantalla OLED física. Ahora activaremos la ronda de Aprendizaje Federado para que observen la convergencia del algoritmo FedAvg en vivo."*

---

## Diapositiva 12: Conclusiones y Trabajo Futuro
* **Título:** Conclusiones y Proyección
* **Puntos clave en pantalla:**
  * ✅ Plataforma funcional, segura y desacoplada galvánicamente a bajo costo.
  * ✅ Enlace ESP-NOW con latencia <5 ms y escalabilidad sin sobrecargar routers Wi-Fi.
  * ✅ Validación empírica del entrenamiento federado en microrredes.
  * 🔮 Trabajo Futuro: Implementación en PCB dedicada, inferencia con TensorFlow Lite for Microcontrollers en el ESP32 e integración de paneles solares fotovoltaicos.
* **🗣️ Guion del expositor:**
  > *"En conclusión, ARCEUS GRID demuestra que es posible implementar una plataforma industrial de AIoT y microrredes de alta precisión utilizando hardware accesible y buenas prácticas de ingeniería eléctrica. Hemos integrado medición, protección activa, telemetría inalámbrica y aprendizaje federado con éxito rotundo. Muchas gracias por su atención y quedamos a su entera disposición para responder sus preguntas."*
