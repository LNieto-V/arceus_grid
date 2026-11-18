# 👥 GUÍA DE EXPOSICIÓN Y ROLES PARA 5 PERSONAS
## Detección Federada de Anomalías en Maquinaria Industrial mediante Arquitecturas IIoT y AIoT (ARCEUS GRID)

* **Institución:** Universidad Cooperativa de Colombia (UCC)
* **Duración total recomendada:** 12 a 15 minutos (2.5 a 3 minutos por persona)
* **Objetivo:** Demostrar dominio de ingeniería multidisciplinar (Electrónica, Firmware, Redes, Ciberseguridad, Inteligencia Artificial y Normativa Industrial).

---

## 📋 MAPA GENERAL DE LA PRESENTACIÓN Y TRANSICIONES

```
[PERSONA 1] ──► [PERSONA 2] ──► [PERSONA 3] ──► [PERSONA 4] ──► [PERSONA 5]
 Problema &      Hardware &      Seguridad &      Firmware &      IA, FedAvg
 Arquitectura    Sensores IIoT   Aislamiento      ESP-NOW/MQTT    & Métricas
 (0:00 - 3:00)   (3:00 - 6:00)   (6:00 - 9:00)   (9:00 - 12:00)  (12:00 - 15:00)
```

---

## 👤 PERSONA 1: LÍDER DE PROYECTO Y ARQUITECTURA GENERAL
**Especialidad:** Contexto industrial, planteamiento del problema, Computación Ubicua y arquitectura de capas.

### 🎯 Lo que debe saber y dominar:
* Los 3 cuellos de botella del mantenimiento predictivo tradicional en la nube: saturación de ancho de banda, latencias inadmisibles y riesgos de privacidad (IP industrial).
* La definición de **Computación Ubicua**: inteligencia invisible embebida directamente en el entorno de producción (máquinas que se autodiagnostican).
* El concepto general de **Aprendizaje Federado (Federated Learning)**: entrenar modelos locales en el borde sin enviar datos crudos a la nube.
* La topología en 2 capas del paper: Capa de Adquisición Física (IIoT) y Capa de Inferencia/Aprendizaje en el Borde (AIoT).

### 🎙️ Guion de Exposición (Qué decir):
> *"Buenos días, miembros del jurado. En la automatización industrial moderna, el monitoreo del estado de la maquinaria rotativa —como motores, bombas y abanicos— es vital para evitar paradas no programadas. Tradicionalmente, la industria ha recurrido al Cloud Computing: colocar sensores que envían millones de muestras crudas de aceleración y corriente a la nube.*
>
> *Sin embargo, este enfoque colapsa por tres razones críticas:*
> 1. *Genera cuellos de botella masivos en el ancho de banda de la planta.*
> 2. *Introduce latencias de cientos de milisegundos, inaceptables para disparar una protección antes de que un devanado se destruya.*
> 3. *Expone secretos operacionales y perfiles de producción a riesgos de ciberseguridad.*
>
> *Para resolver esto, nuestro proyecto propone un sistema descentralizado basado en Computación Ubicua, IIoT y AIoT. En lugar de transmitir telemetría cruda, cada máquina cuenta con un nodo inteligente basado en ESP32 que procesa localmente las anomalías y utiliza **Aprendizaje Federado** para compartir únicamente parámetros matemáticos actualizados hacia un coordinador central.*
>
> *Nuestra arquitectura se divide en dos grandes capas: la capa de adquisición física e instrumentación IIoT, y la capa de inferencia y coordinación AIoT. Para explicar cómo capturamos las variables físicas y diseñamos el hardware bajo normativas internacionales, los dejo con mi compañero [Nombre Persona 2]."*

### 🛡️ Preguntas del Jurado que DEBE responder esta persona:
* **P: ¿Por qué no usar simplemente una plataforma como AWS IoT o Azure?**  
  *R:* "Porque centralizar datos crudos a 100 Hz de cientos de motores cuesta miles de dólares en ancho de banda y almacenamiento en la nube, y ante una falla catastrófica inmediata la latencia de internet no garantiza una respuesta en menos de 10 milisegundos. Nuestro sistema resuelve la inferencia en el borde y protege la privacidad de la planta."
* **P: ¿Por qué eligieron motores de abanico industriales como banco de pruebas?**  
  *R:* "Porque los motores de abanico representan la carga inductiva rotativa más común en la industria. Permiten simular tanto fallas mecánicas por desbalance en las aspas como sobrecargas térmicas y eléctricas en devanados reales de corriente alterna."

---

## 👤 PERSONA 2: INGENIERO DE HARDWARE, SENSORES Y NORMATIVA IPC
**Especialidad:** Selección de componentes, acondicionamiento de señales y diseño de PCB según IPC-2221B e IPC-2222.

### 🎯 Lo que debe saber y dominar:
* **Sensores:**
  * **ADXL345 (I2C):** Acelerómetro triaxial digital ($\pm 2g$ a $\pm 16g$, 13 bits), para vibraciones en rodamientos y desbalance dinámico.
  * **DS18B20 (OneWire):** Sensor térmico digital para temperatura del devanado crítico del estator ($-55^\circ\text{C}$ a $+125^\circ\text{C}$).
  * **INA219 (I2C `0x40`):** Shunt $0.1\,\Omega$, medición High-Side de corriente, tensión de bus y potencia.
  * **Transformadores de Corriente AC:** Sensores split-core sin contacto en las líneas de fase.
* **Normativas IPC:**
  * **IPC-2221B:** Norma genérica de diseño de PCB. Cálculo de ancho de pista según corriente y elevación térmica ($\Delta T \le 10^\circ\text{C}$). Aislamiento dieléctrico (*clearance* y *creepage*).
  * **IPC-2222:** Norma seccional para circuitos rígidos (cobre de 1 oz/ft² $\approx 35\,\mu\text{m}$, planos continuos de tierra para inmunidad electromagnética EMI frente al ruido de los motores).

### 🎙️ Guion de Exposición (Qué decir):
> *"Gracias. Para llevar la teoría a la práctica, construimos un banco experimental con 4 motores de abanico industriales AC, gobernados por 3 unidades microcontroladoras ESP32 de arquitectura Xtensa dual-core a 240 MHz.*
>
> *La adquisición física de datos se estructuró de manera síncrona mediante tres tipos de transductores:*
> 1. *Para la dinámica mecánica: acelerómetros triaxiales digitales **ADXL345** comunicados por bus I2C, capaces de muestrear a alta resolución para capturar las firmas espectrales de desbalance y fallas en pistas de rodamientos.*
> 2. *Para la supervisión térmica: sensores digitales **DS18B20** con protocolo OneWire, acoplados térmicamente a los devanados del estator.*
> 3. *Para las variables eléctricas: sensores de corriente AC no invasivos y módulos **INA219** en configuración High-Side con resistencia shunt de 0.1 ohmios para medir tensión, corriente y potencia consumida.*
>
> *Un aspecto diferenciador de nuestro proyecto es que no usamos protoboards en la versión final. Diseñamos y fabricamos tarjetas de circuito impreso (PCB) dedicadas bajo normativas internacionales:*
> * Aplicamos la **norma IPC-2221B** para calcular analíticamente el ancho de las pistas de cobre, asegurando que ante corrientes de régimen continuo la elevación térmica no supere los 10 °C, y respetando las distancias dieléctricas de aislamiento para evitar arcos eléctricos.*
> * Cumplimos la **norma IPC-2222** para placas rígidas con cobre de 1 oz, incorporando planos de masa extensos que blindan al ESP32 del ruido electromagnético radiado por los motores.*
>
> *Sin embargo, mezclar electrónica digital sensible de 3.3V con conmutación de motores y bobinas de potencia de 12V y 110V plantea riesgos destructivos. A continuación, [Nombre Persona 3] explicará cómo resolvimos la seguridad eléctrica y el aislamiento."*

### 🛡️ Preguntas del Jurado que DEBE responder esta persona:
* **P: ¿Por qué seleccionaron el sensor térmico DS18B20 con protocolo OneWire en vez de un termopar o LM35?**  
  *R:* "Porque el DS18B20 entrega una señal digital calibrada de fábrica con resolución configurable de hasta 12 bits sobre un solo cable de datos. Los sensores analógicos como el LM35 sufren atenuación y captan ruido electromagnético severo en cables largos cercanos a motores AC."
* **P: ¿Qué fórmula usaron para calcular las pistas de la PCB?**  
  *R:* "Aplicamos la ecuación empírica de la norma IPC-2221B: $A = (I / (k \cdot \Delta T^b))^{1/c}$, donde especificamos un $\Delta T$ máximo admisible de 10 °C y $k=0.048$ para capas externas con cobre de 1 oz ($35\,\mu\text{m}$), garantizando que las pistas soporten picos de corriente sin degradación térmica."

---

## 👤 PERSONA 3: INGENIERO DE SEGURIDAD ELÉCTRICA, AISLAMIENTO Y POTENCIA
**Especialidad:** Aislamiento de relés (JD-VCC), tierra en estrella (*Star Ground*), drivers de potencia y balance térmico.

### 🎯 Lo que debe saber y dominar:
* **El Jumper JD-VCC / VCC en relés:** Por qué es obligatorio quitarlo. Las bobinas son de 5V inductivas ($72\,\text{mA}$ c/u $\approx 290\,\text{mA}$ total); el ESP32 es de 3.3V. Si no se quita el jumper, hay una tensión residual de $1.7\,\text{V}$ que impide apagar el optoacoplador y transmite picos de fuerza contraelectromotriz (*Back-EMF*) al silicio del MCU.
* **Tierra en Estrella (*Star Ground*):** Punto común de masa exclusivamente en el borne negativo de la fuente de 12V. Jamás retornar corriente de motores o bombillas por los pines GND del microcontrolador.
* **Driver de Pilotos 12V:** Uso de MOSFETs de nivel lógico (*Logic-Level*, como IRLZ44N o AO3400A) que saturan con $V_{GS} = 3.3\,\text{V}$, frente al BJT 2N2222 que invierte la lógica.
* **Balance de Potencia:** Corriente nominal total de $2.41\,\text{A}$. Fuente dimensionada a 12V @ 5A (60W) con factor de seguridad del 40%. Fusible principal de 5A y fusibles por estación de 2A.

### 🎙️ Guion de Exposición (Qué decir):
> *"Gracias. En cualquier entorno industrial donde conviven señales lógicas débiles de 3.3 voltios con actuadores de potencia, la seguridad eléctrica es el factor determinante entre un sistema robusto y un circuito destruido.*
>
> *En ARCEUS GRID implementamos tres principios de diseño rigurosos:*
>
> 1. * **Aislamiento Óptico Real en Relés:** Los módulos comerciales de 4 relés utilizan bobinas electromecánicas de 5V que demandan casi 300 mA. **El error común en ingeniería es dejar puesto el jumper entre VCC y JD-VCC**. Si se deja ese jumper, las bobinas inductivas comparten tensión con los optoacopladores, generando una diferencia residual de 1.7V que impide que el optoacoplador se apague con la lógica de 3.3V del ESP32, además de inyectar ruido inductivo. En nuestro sistema **retiramos el jumper**: alimentamos `JD-VCC` desde una línea independiente de 5V y alimentamos `VCC` a 3.3V, logrando desacoplo galvánico total.
> 2. * **Topología de Tierra en Estrella (Star Ground):** Evitamos los bucles de masa haciendo que todas las corrientes de retorno de potencia y los retornos de control converjan físicamente en un único borne en la fuente de 12V. Jamás permitimos que la corriente de las cargas regrese por las patillas del ESP32, erradicando reseteos por Brownout y ruido en el bus I2C.
> 3. * **Etapa de Pilotos 12V:** Analizamos el uso de transistores 2N2222 frente a MOSFETs de nivel lógico. Seleccionamos MOSFETs tipo Logic-Level que entran en saturación completa con los 3.3V del microcontrolador, eliminando componentes intermedios e inversiones lógicas indeseadas.
>
> *El balance de potencia en el peor escenario operativo suma 2.41 amperios. Siguiendo estándares IEEE, aplicamos un factor de seguridad del 40%, seleccionando una fuente industrial conmutada de 12V a 5A (60W), protegida por un fusible general de 5A y fusibles rápidos de 2A por estación.*
>
> *Habiendo garantizado la estabilidad del hardware, [Nombre Persona 4] explicará la arquitectura de red y el firmware embebido no bloqueante."*

### 🛡️ Preguntas del Jurado que DEBE responder esta persona:
* **P: ¿Por qué el ESP32 se resetea por "Brownout" si no se utiliza una topología en estrella?**  
  *R:* "Porque si los retornos de corriente de los motores o relés pasan por el cable o la pista de masa del ESP32, la ley de Ohm ($V = I \times R$) genera una elevación del potencial de masa. Esto hace que la tensión neta entre 3V3 y GND caiga por debajo de 2.43V, disparando el circuito detector interno de Brownout del silicio y reiniciando el microcontrolador."
* **P: ¿Qué función cumple el diodo en paralelo con la bobina del relé?**  
  *R:* "Es un diodo de rueda libre o Flyback. Cuando se desenergiza la bobina, la ley de Faraday-Lenz genera un pico inductivo de polaridad inversa (Back-EMF) que puede superar los 100 voltios. El diodo recircula esa corriente y disipa la energía, protegiendo al transistor driver de perforación dieléctrica."

---

## 👤 PERSONA 4: INGENIERO DE FIRMWARE EMBEBIDO, REDES Y PROTOCOLOS
**Especialidad:** Programación no bloqueante con `millis()`, protocolo binario ESP-NOW, broker Mosquitto MQTT y protecciones locales de borde.

### 🎯 Lo que debe saber y dominar:
* **Firmware no bloqueante:** Prohibición absoluta de `delay()`. Máquinas de estado finitas gobernadas por diferencias de tiempo con `millis()`.
* **Guardrail Local Autónomo:** Si la corriente supera **2.8 A**, el microcontrolador corta los relés por software en menos de 1 ms, sin depender de la red ni del coordinador.
* **Topología Híbrida de Red:**
  * **Nivel de Campo (ESP-NOW):** Tramas binarias directas de capa de enlace MAC (~25 bytes). Auto-escaneo de canales RF (1 al 13). Cero necesidad de IP, SSID o contraseñas en los sensores (Ciberseguridad por aislamiento).
  * **Gateway ESP32 Central:** Hace de puente (*Bridge*), desempaqueta la trama binaria y publica en JSON hacia el broker MQTT Mosquitto en Docker.
* **Estructura MQTT:** Tópicos jerárquicos (`arceus/station_a/telemetry`, `commands`, `alerts`).

### 🎙️ Guion de Exposición (Qué decir):
> *"Gracias. A nivel de firmware y comunicaciones, el desafío principal era garantizar tiempo real estricto y eliminar la saturación del tráfico de red.*
>
> *Para ello, implementamos dos decisiones de ingeniería fundamentales:*
>
> *Primero, en el firmware del ESP32 **eliminamos por completo la función `delay()`**. Todo el código está estructurado como máquinas de estado temporizadas mediante diferenciales de `millis()`. Esto garantiza que el microcontrolador jamás se quede pausado esperando un sensor, permitiendo que la rutina de protección evalúe continuamente la corriente: **si el consumo supera los 2.8 amperios, el nodo corta los relés de forma autónoma en microsegundos**, sin depender de la conectividad externa.*
>
> *Segundo, diseñamos una **topología de comunicación híbrida** de dos niveles:*
> 1. * **Nivel de Campo con ESP-NOW:** Las estaciones de sensórica no se conectan a la red Wi-Fi convencional. Utilizan el protocolo ESP-NOW a nivel de trama MAC. Esto reduce el payload a una estructura binaria compacta en C de apenas **25 bytes** (frente a los más de 200 bytes de un JSON sobre TCP/IP) y elimina el proceso de negociación DHCP. Además, los nodos sensores implementan un algoritmo de auto-escaneo que detecta dinámicamente el canal de radio del Gateway (del canal 1 al 13). Desde el punto de vista de ciberseguridad, los nodos sensores son invisibles para atacantes externos porque carecen de dirección IP.
> 2. * **Nivel de Coordinación con MQTT:** Un ESP32 configurado como Gateway Central recibe las tramas ESP-NOW, las traduce a formato JSON y las publica a través de Wi-Fi hacia un broker **Eclipse Mosquitto** contenerizado en Docker, bajo tópicos jerárquicos estandarizados.*
>
> *Con esta base de datos e infraestructura lista, [Nombre Persona 5] presentará el núcleo de inteligencia artificial: cómo implementamos el Aprendizaje Federado y los resultados obtenidos en el paper."*

### 🛡️ Preguntas del Jurado que DEBE responder esta persona:
* **P: ¿Qué sucede si se apaga el router Wi-Fi o se cae el servidor central?**  
  *R:* "El nodo sensor de borde opera con total autonomía. Dado que se comunica por ESP-NOW directamente con la radio del Gateway y su firmware ejecuta la protección física por sobrecorriente de forma local, la máquina sigue monitoreada y protegida. Los datos se almacenan en búfer local para sincronizarse tan pronto como se restablezca el enlace."
* **P: ¿Por qué usar tramas binarias en ESP-NOW en lugar de enviar texto JSON directo?**  
  *R:* "Porque un entero de 16 bits ocupa 2 bytes en binario, mientras que en texto JSON `"corriente": 1.25` ocupa más de 17 bytes de caracteres ASCII. El protocolo binario reduce el tiempo de transmisión por radio en más de un 80%, disminuyendo el consumo de batería y la probabilidad de colisiones en el espectro de 2.4 GHz."

---

## 👤 PERSONA 5: INGENIERO DE INTELIGENCIA ARTIFICIAL, APRENDIZAJE FEDERADO Y RESULTADOS
**Especialidad:** Detección dual de anomalías, algoritmo FedAvg, red neuronal ligera de borde y las 4 métricas del paper.

### 🎯 Lo que debe saber y dominar:
* **Detección Dual de Anomalías:** Capa 1 (Guardrails deterministas de tensión, corriente y potencia) + Capa 2 (Z-Score estadístico para derivas lentas e impedancia).
* **Algoritmo FedAvg:** La fórmula matemática y su ciclo:
  $$\theta_{global} = \sum_{k=1}^K \frac{n_k}{n_{total}} \theta_k$$
* **Red Neuronal Ligera (Edge MLP):** Entrada (3: $V, \text{relé}_1, \text{relé}_2$) $\rightarrow$ Oculta (8 neuronas, ReLU) $\rightarrow$ Salida (1 lineal, corriente predicha $\hat{I}$).
* **Software Central:** Docker Compose con Mosquitto, SQLite en modo WAL (lecturas y escrituras concurrentes sin bloqueo) y Dashboard Streamlit interactivo.
* **Las 4 Métricas de Evaluación de la Sección IV del Paper:**
  1. *Precisión de Detección:* Tasa de acierto ante anomalías inducidas de vibración (ADXL345) y temperatura (DS18B20).
  2. *Tráfico de Red:* Reducción $> 95\%$ en bytes transmitidos al enviar solo pesos periódicos vs telemetría continua.
  3. *Heterogeneidad de Datos (Non-IID):* Capacidad del modelo global de converger aunque los motores tengan diferentes regímenes operativos.
  4. *Resistencia a Desconexiones:* Estabilidad operativa ante pérdidas intermitentes de red.

### 🎙️ Guion de Exposición (Qué decir):
> *"Gracias. Llegamos al núcleo innovador de nuestra investigación: la detección descentralizada de anomalías mediante **Aprendizaje Federado (Federated Learning)**.*
>
> *En lugar de aplicar IA a ciegas, diseñamos un sistema de detección en dos capas:*
> 1. * **Capa Determinista Inmediata:** Monitorea reglas físicas estrictas: subtensión (<10V), sobretensión (>14V), corrientes de fuga sin carga y circuitos abiertos por devanado roto.*
> 2. * **Capa Estadística / Machine Learning:** Aplica una distancia normalizada por Z-score para alertar derivas térmicas y descalibraciones sutiles antes de que ocurra la falla física.*
>
> *Para el aprendizaje colaborativo, cada nodo entrena un modelo neuronal ligero de 2 capas que predice la corriente e impedancia nominal esperada según el estado operativo. El entrenamiento se ejecuta localmente mediante Descenso de Gradiente Estocástico (SGD). Los datos crudos jamás salen de la máquina.*
>
> *El coordinador central ejecuta el algoritmo **FedAvg (Federated Averaging)**. Al finalizar una ronda de entrenamiento, el servidor recibe los pesos de cada estación y calcula el modelo global mediante el promedio ponderado por el número de muestras:*
> $$\theta_{global} = \sum_{k=1}^K \frac{n_k}{n_{total}} \theta_k$$
>
> *En la Sección IV del artículo científico validamos el sistema bajo cuatro métricas fundamentales:*
> 1. * **Precisión de Detección:** Logramos una sensibilidad superior al 95% ante anomalías inducidas de vibración mecánica y calentamiento del estator.*
> 2. * **Tráfico de Red:** Demostramos una **reducción de más del 95% en los bytes transmitidos**. Enviar una matriz de pesos de unos pocos bytes por ronda ahorra gigabytes de telemetría continua en comparación con la nube tradicional.*
> 3. * **Heterogeneidad de Datos (Non-IID):** Comprobamos que el modelo global converge con estabilidad aun cuando los 4 motores operan con perfiles de carga y desgastes distintos.*
> 4. * **Resistencia a Desconexiones:** Ante cortes forzados de enlace Wi-Fi, los nodos mantuvieron la inferencia y las protecciones activas sin interrupción.*
>
> *Toda esta arquitectura está respaldada por un coordinador contenerizado en Docker con SQLite en modo WAL y un panel en Streamlit que visualiza la telemetría en tiempo real. En conclusión, demostramos que la combinación de IIoT, normativas IPC y Aprendizaje Federado es viable, eficiente y lista para la industria moderna. Quedamos a disposición del jurado para sus preguntas."*

### 🛡️ Preguntas del Jurado que DEBE responder esta persona:
* **P: ¿Qué es el problema de datos Non-IID y cómo lo soluciona su sistema?**  
  *R:* "Non-IID significa que los datos entre estaciones no son independientes ni idénticamente distribuidos. En una planta real, un motor opera al 50% y otro al 90%. Si se promedian simplemente sin control, los gradientes divergen. FedAvg lo resuelve ponderando cada modelo por su volumen de datos $n_k / n_{total}$ y aumentando las épocas locales de entrenamiento antes de la agregación, logrando que el modelo aprenda la firma común de falla sin sesgarse."
* **P: ¿Por qué usan SQLite con modo WAL en el backend en lugar de MySQL o Postgres?**  
  *R:* "Porque para un coordinador de borde industrial necesitamos ligereza y mínima sobrecarga de memoria. El modo WAL (Write-Ahead Logging) de SQLite permite que el daemon MQTT inserte telemetría a alta velocidad mientras el Dashboard de Streamlit lee datos concurrentemente para renderizar gráficos, sin generar bloqueos de base de datos (`database locked`)."

---

## 🎯 RESUMEN DE RESPONSABILIDADES ANTE PREGUNTAS DEL JURADO

| Si el Jurado pregunta sobre... | La persona que debe responder es... |
|---|---|
| Justificación, costos, privacidad en la nube, por qué computación ubicua | **Persona 1** |
| Cálculos de PCB, norma IPC-2221B, ADXL345, DS18B20, INA219 | **Persona 2** |
| Jumper JD-VCC, relés, Star Ground, Brownout, drivers de pilotos, fusibles | **Persona 3** |
| Firmware, `millis()`, ESP-NOW, tramas binarias, tópicos MQTT, Docker | **Persona 4** |
| FedAvg, red neuronal, Z-score, Non-IID, las 4 métricas del paper, Streamlit | **Persona 5** |
