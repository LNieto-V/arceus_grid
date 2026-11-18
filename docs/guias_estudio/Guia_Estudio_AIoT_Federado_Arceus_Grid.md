# 📘 GUÍA INTEGRAL DE ESTUDIO Y SUSTENTACIÓN TÉCNICA
## Detección Federada de Anomalías en Maquinaria Industrial mediante Arquitecturas IIoT y AIoT (ARCEUS GRID)

* **Autores:** Owen Mora, Braulio Ribón, Angelo Charris, Manuel Pertuz
* **Institución:** Universidad Cooperativa de Colombia (UCC) — Facultad de Ingeniería
* **Área Temática:** Computación Ubicua, Sistemas Embebidos, IIoT, Inteligencia Artificial en el Borde (AIoT)
* **Documentos de Referencia:**
  * Artículo Científico: *Detección Federada de Anomalías en Maquinaria Industrial mediante Arquitecturas IIoT y AIoT* (ubicua_final.pdf)
  * Repositorio de Ingeniería y Código Abierto: [LNieto-V/arceus_grid](https://github.com/LNieto-V/arceus_grid)

---

## ÍNDICE GENERAL DE MATERIAS

1. **Fundamentos y Motivación de la Arquitectura Descentralizada**
2. **Hardware, Instrumentación y Acondicionamiento de Señales (IIoT)**
3. **Ingeniería Eléctrica, Seguridad de Planta y Normativas IPC**
4. **Topología de Redes y Protocolos de Comunicación (ESP-NOW & MQTT)**
5. **Inteligencia Artificial en el Borde (AIoT) y Aprendizaje Federado (FedAvg)**
6. **Coordinador Central, Ingestión Concurrente y Visualización**
7. **Análisis Profundo de las 4 Métricas de Evaluación del Paper**
8. **Banco Exhaustivo de Preguntas y Respuestas para Sustentación Oral**
9. **Glosario Técnico y Fórmulas Clave**
10. **Referencias Bibliográficas**

---

## 1. FUNDAMENTOS Y MOTIVACIÓN DE LA ARQUITECTURA DESCENTRALIZADA

### 1.1. El Problema del Paradigma Cloud Tradicional en la Industria
En los esquemas convencionales de mantenimiento predictivo (PdM), los sensores montados sobre motores, bombas y maquinaria rotativa transmiten continuamente telemetría cruda (series temporales de aceleración a frecuencias de cientos de hercios, muestras de corriente y temperatura) hacia servidores centralizados en la nube. Este modelo adolece de tres fallas estructurales:
1. **Saturación del Ancho de Banda:** Cientos de nodos transmitiendo a 1 kHz saturan rápidamente los enlaces de red de planta y los túneles WAN, disparando los costos de infraestructura y nube.
2. **Latencia Crítica e Inaceptable:** El tiempo de viaje de ida y vuelta (*Round Trip Time* - RTT) hacia un servidor en la nube ronda entre 50 ms y 500 ms. Ante una sobrecorriente destructiva, bloqueo de rotor o embalamiento térmico, una respuesta tardía destruye el devanado antes de recibir la orden de apagado.
3. **Vulneración de la Privacidad y Secreto Operacional:** Los perfiles de carga, cadencias de producción y regímenes de operación constituyen secretos industriales (IP). La transmisión de datos sin procesar fuera del perímetro físico de la fábrica plantea riesgos severos de ciberseguridad y espionaje corporativo.

### 1.2. La Propuesta: AIoT + Aprendizaje Federado (Federated Learning)
El proyecto implementa una arquitectura descentralizada donde:
* **La inferencia y el entrenamiento local se ejecutan en el borde (*Edge Computing*):** Microcontroladores de bajo costo y alto rendimiento periférico (ESP32) adquieren las variables físicas y ejecutan modelos ligeros.
* **Privacidad por Diseño (*Privacy by Design*):** Los datos crudos jamás abandonan la memoria volátil del microcontrolador ni se transmiten al servidor.
* **Intercambio Exclusivo de Parámetros:** Solo se envían al coordinador central los pesos sinápticos y sesgos actualizados ($\theta$) resultantes del entrenamiento local.
* **Agregación Matemática Global:** El coordinador ejecuta el algoritmo **Federated Averaging (FedAvg)** para crear un modelo consensuado que aprovecha la experiencia de toda la flota de maquinaria sin centralizar telemetría.

```
       MAQUINARIA 1                     MAQUINARIA 2                     MAQUINARIA N
  [Motor + Sensores IIoT]          [Motor + Sensores IIoT]          [Motor + Sensores IIoT]
             │                                │                                │
             ▼                                ▼                                ▼
    [Nodo AIoT (ESP32)]              [Nodo AIoT (ESP32)]              [Nodo AIoT (ESP32)]
   • Muestreo síncrono              • Muestreo síncrono              • Muestreo síncrono
   • Guardrail local < 2.8A         • Guardrail local < 2.8A         • Guardrail local < 2.8A
   • Entrenamiento SGD local        • Entrenamiento SGD local        • Entrenamiento SGD local
             │                                │                                │
             │ (Pesos θ_1)                    │ (Pesos θ_2)                    │ (Pesos θ_N)
             └───────────────────────┬────────┴────────────────────────────────┘
                                     ▼
                     [COORDINADOR CENTRAL (FedAvg)]
                      θ_global = Σ (n_k / n) * θ_k
                      Actualización del Modelo Maestro
                                     │
             ┌───────────────────────┴────────────────────────────────┐
             │ (Modelo Global Actualizado θ_global)                   │
             ▼                                                        ▼
    [Nodo AIoT (ESP32)]                                      [Nodo AIoT (ESP32)]
```

---

## 2. HARDWARE, INSTRUMENTACIÓN Y ACONDICIONAMIENTO DE SEÑALES (IIoT)

### 2.1. Inventario Físico de Componentes (Cuadro I del Paper y ARCEUS GRID)

| Componente | Especificación Técnica | Cantidad | Función en el Sistema |
|---|---|---|---|
| **Motores de Abanico** | AC Monofásicos industriales | 4 unidades | Cargas rotativas reales para inducir anomalías dinámicas (desbalance, roce de aspas, sobrecarga). |
| **Unidades MCU** | ESP32 NodeMCU (Xtensa dual-core LX6, 240 MHz) | 3 unidades | Nodos de procesamiento de borde (2 estaciones de sensórica + 1 Gateway/Coordinador). |
| **Acelerómetro Triaxial** | ADXL345 (Digital, bus I2C / SPI) | 3 unidades | Medición dinámica de vibraciones mecánicas (rango configurable $\pm 2g$ a $\pm 16g$, 13 bits). |
| **Sensor de Corriente/Voltaje** | INA219 (I2C, dirección `0x40`, Shunt $0.1\,\Omega$) | 2 unidades | Instrumentación eléctrica High-Side (tensión de bus, corriente DC de carga y potencia). |
| **Sensor Térmico** | DS18B20 (Digital, bus OneWire, encapsulado metálico) | 1 unidad | Supervisión de temperatura en el devanado crítico del estator ($-55^\circ\text{C}$ a $+125^\circ\text{C}$). |
| **Sensores de Corriente AC** | Sensores inductivos tipo Split-Core (no invasivos) | Lote | Medición de corriente alterna en las fases de los motores de abanico. |
| **Protección Eléctrica** | Tacos termomagnéticos bipolares de 20A | 3 unidades | Protección electromagnética y térmica de las líneas principales de alimentación AC. |
| **Módulo de Relés** | 4 canales, bobinas de 5V con optoacopladores | 1 módulo | Conmutación de cargas de potencia con aislamiento óptico. |
| **Pantallas de Supervisión** | OLED SSD1306 (0.96 pulgadas, $128 \times 64$, I2C `0x3C`) | 2 unidades | Visualización local de parámetros eléctricos, estado de enlace y diagnósticos. |
| **Reguladores de Tensión** | Convertidores Buck DC-DC LM2596 | 2 unidades | Reducción eficiente de 12V DC a 5.00V DC para alimentar la lógica del ESP32. |
| **Pilotos Indicadores** | Pilotos industriales 12V DC (Verde, Amarillo, Rojo) | 6 unidades | Señalización óptica del estado de máquina (Normal, Advertencia, Falla crítica). |

### 2.2. Acondicionamiento de Señales Dinámicas
* **Vibración con ADXL345:** Ubicado rígidamente en el chasis del motor. Detecta el desbalance dinámico (armónicos a la velocidad de giro $1\times$) y fallas de rodamiento en pistas externas/internas (altas frecuencias).
* **Temperatura con DS18B20:** Fijado al devanado mediante grasa térmica. Permite correlacionar anomalías de vibración con elevación térmica por fricción excesiva o pérdida de ventilación.
* **Corriente y Potencia con INA219:** Configuración **High-Side** en la barra positiva de alimentación. Al medir aguas arriba de los relés, permite registrar la corriente combinada de todas las cargas activas, detectando fugas a tierra antes del corte.

---

## 3. INGENIERÍA ELÉCTRICA, SEGURIDAD DE PLANTA Y NORMATIVA IPC

### 3.1. Normativas IPC para Diseño y Fabricación de PCB
Para operar en entornos industriales ruidosos sin bloqueos por interferencia electromagnética (EMI), el diseño de la tarjeta de circuito impreso sigue los estándares internacionales:
* **IPC-2221B (Generic Standard on Printed Board Design):**
  * **Cálculo del ancho de pista:** El ancho se dimensiona según la densidad de corriente y el calentamiento admisible ($\Delta T \le 10^\circ\text{C}$). Ecuación IPC-2221:
    $$A = \left( \frac{I}{k \cdot \Delta T^{0.44}} \right)^{\frac{1}{0.725}}$$
    *(donde $k = 0.048$ para pistas externas, $I$ en amperios y $A$ en mils²).*
  * **Distancias dieléctricas (*Clearance* y *Creepage*):** Separación mínima de 1.5 mm a 2.5 mm entre pistas de señal débil (I2C) y líneas de potencia para evitar arcos y fugas dieléctricas.
* **IPC-2222 (Sectional Design Standard for Rigid Printed Boards):**
  * Espesor de lámina de cobre estándar de 1 oz/ft² ($35\,\mu\text{m}$).
  * Planos de masa (*Ground Pours*) con apantallamiento perimetral para disipar ruido radiado de motores AC.

### 3.2. Topología de Masa en Estrella (*Star Ground*)
En circuitos mixtos (microcontroladores digitales + conmutación de relés + motores inductivos), compartir líneas de tierra genera bucles de masa (*Ground Loops*).
* **Fenómeno:** La corriente de retorno de una carga ($I_{carga}$) a través de la resistencia del cable ($R_{cable}$) induce una tensión espurio $V_{ruido} = I_{carga} \times R_{cable}$ en el GND del ESP32.
* **Consecuencias Evitadas:** Reseteos aleatorios por subtensión interna (*Brownout Reset*), lecturas erráticas en el bus I2C y cuelgues del ADC.
* **Implementación:** Todos los cables negativos de potencia (motores, bombillas, relés) y los retornos del convertidor LM2596 convergen físicamente en un único borne central en la fuente de 12V.

### 3.3. Aislamiento Óptico de Relés y el Jumper JD-VCC
Los módulos de relés Songle SRD incorporan optoacopladores pero vienen de fábrica con un puente plástico uniendo los pines `VCC` y `JD-VCC`.
* **Riesgo:** Si no se retira el jumper, las bobinas de 5V y los LEDs de señal comparten alimentación. Si se conecta a los 5V y se gobierna con el ESP32 (3.3V), ocurre una fuga de tensión inversa:
  $$V_{LED} = 5.0\,\text{V} - 3.3\,\text{V} = 1.7\,\text{V}$$
  Dado que el diodo del optoacoplador conduce a partir de $1.2\,\text{V} - 1.4\,\text{V}$, el relé **permanece encendido o zumba sin poder apagarse**.
* **Solución Mandatoria en ARCEUS GRID:**
  1. Retirar el jumper plástico `JD-VCC - VCC`.
  2. Conectar `JD-VCC` y `GND` a la fuente externa de 5.0V (aislando el pico inductivo de conmutación).
  3. Conectar `VCC` a los **3.3V** del ESP32.
  4. Los pines `IN1..IN4` se gobiernan con lógica **Active-LOW** (el optoacoplador conduce cuando el GPIO drena corriente a nivel bajo).

### 3.4. Etapa de Control para Pilotos 12V
El microcontrolador entrega 3.3V y un máximo seguro de 12 mA por pin, mientras que un piloto industrial requiere 12V y hasta 100 mA.
* **Análisis BJT 2N2222 + MOSFET estándar:** El 2N2222 polariza la compuerta a 12V para saturar MOSFETs comunes (ej. IRF540N). Desventaja: Inversión lógica (enciende en LOW) y destellos indeseados durante el arranque del microcontrolador.
* **Solución Óptima: MOSFET de Nivel Lógico (*Logic-Level*):** Modelos como el **IRLZ44N**, **AO3400A** o **2N7000** alcanzan resistencia ultrabaja ($R_{DS(on)} < 40\,\text{m}\Omega$) con apenas $V_{GS} = 3.3\,\text{V}$. Se gobiernan directamente con una resistencia en serie de $100\,\Omega$ y una resistencia *pull-down* de $10\,\text{k}\Omega$ a tierra.

### 3.5. Memoria de Balance Eléctrico y Térmico
* Consumo total simultáneo en el peor escenario (Full Load):
  * 4 Bombillas / Cargas: $4 \times 0.46\,\text{A} = 1.84\,\text{A}$
  * 2 Pilotos activos: $2 \times 0.08\,\text{A} = 0.16\,\text{A}$
  * 2 ESP32 + OLEDs + Sensores: $2 \times 0.13\,\text{A} = 0.26\,\text{A}$
  * 4 Bobinas de relé: $\approx 0.15\,\text{A}$ en 12V
  * **Corriente Nominal Total:** $I_{total} = 2.41\,\text{A}$
* **Factor de Seguridad Industrial (IEEE 40%):**
  $$I_{fuente} = 2.41\,\text{A} \times 1.40 = 3.37\,\text{A} \implies \text{Fuente Seleccionada: 12V @ 5A (60W)}$$
* **Protecciones:** Fusible general de 5A fusión rápida (F) a la salida de la fuente; fusibles de 2A por cada estación sensórica. Conductor troncal: AWG 18 / 16 (caída $< 1\%$).

---

## 4. TOPOLOGÍA DE REDES Y PROTOCOLOS DE COMUNICACIÓN

### 4.1. Arquitectura Híbrida: ESP-NOW + MQTT
Para solventar el problema de saturación y gestión de redes en entornos industriales, el sistema divide la comunicación en dos niveles:

```
[Estación A (Sensor)] ───┐  (ESP-NOW: Paquetes Binarios ~25 bytes)
                         ├──► [ESP32 Main Gateway] ──► (MQTT TCP/IP) ──► [Mosquitto Broker]
[Estación B (Sensor)] ───┘                                                      │
                                                                                ▼
                                                                       [Coordinador FedAvg]
                                                                       [Dashboard Streamlit]
```

1. **Nivel de Campo (ESP-NOW):**
   * Protocolo sin conexión a nivel MAC basado en tramas de acción IEEE 802.11.
   * **Cero configuración Wi-Fi:** Los nodos sensores no requieren SSID ni contraseña; escanean dinámicamente canales RF (1 al 13) hasta encontrar el Gateway.
   * **Payload Binario Compacto:** Empaqueta variables en estructuras de C puro (`struct`) de apenas 25 bytes.
   * **Aislamiento de Ciberseguridad:** Los nodos de campo no poseen dirección IP, siendo inmunes a ataques por red.
2. **Nivel de Coordinación (MQTT):**
   * El Gateway ESP32 desempaqueta la trama ESP-NOW, la serializa en JSON y la publica en el broker **Mosquitto** en tópicos jerárquicos:
     * `arceus/station_a/telemetry`
     * `arceus/station_a/alerts`
     * `arceus/station_a/commands`

### 4.2. Firmware No Bloqueante y Seguridad de Borde
* **Gestión por `millis()`:** Toda la temporización de adquisición, refresco OLED SSD1306 y balizas luminosas se ejecuta mediante máquinas de estados no bloqueantes.
* **Disparo Local Autónomo (*Edge Guardrail*):** Si la corriente excede **2.8 A**, el microcontrolador corta los relés de forma instantánea e independiente de la conectividad de radio.

---

## 5. INTELIGENCIA ARTIFICIAL EN EL BORDE (AIoT) Y APRENDIZAJE FEDERADO

### 5.1. Detección Dual de Anomalías
El sistema no confía exclusivamente en IA; implementa un esquema en dos capas:
1. **Capa 1: Guardrails Físicos Deterministas (Tiempo de Respuesta < 1 ms):**
   * *Subtensión Crítica:* $V < 10.0\,\text{V}$ (alerta de fallo de fuente o batería descargada).
   * *Sobretensión Crítica:* $V > 14.0\,\text{V}$ (riesgo de daño dieléctrico).
   * *Sobrecorriente Inmediata:* $I > 2.8\,\text{A}$ (disparo y desconexión de relés).
   * *Corriente Fantasma / Fuga:* Relés en OFF pero $I > 40\,\text{mA}$ (posible cortocircuito parcial o fuga a chasis).
   * *Circuito Abierto / Rotura:* Relé en ON pero $I < 30\,\text{mA}$ (rotura de devanado o filamento fundido).
   * *Discrepancia de Potencia:* $|P_{medida} - (V \times I)| > 1.2\,\text{W}$ (error de calibración o componente reactivo anómalo).
2. **Capa 2: Detección Estadística por Desvío Z-Score:**
   $$Z_V = \frac{|V_{medido} - \mu_{nominal}|}{\sigma_V}$$
   Permite detectar resistencia de contacto parásita por bornes oxidados, descalibración lenta o degradación mecánica antes de que ocurra una falla destructiva.

### 5.2. El Algoritmo Federated Averaging (FedAvg)
El Aprendizaje Federado resuelve el problema de optimización distribuida:
$$\min_{w \in \mathbb{R}^d} f(w) \quad \text{donde} \quad f(w) = \sum_{k=1}^K \frac{n_k}{n} F_k(w)$$

* $K$: Número total de nodos clientes (máquinas o estaciones).
* $n_k$: Número de muestras de datos locales en la estación $k$.
* $n = \sum n_k$: Volumen total de datos de la red.
* $F_k(w)$: Función de pérdida local evaluada sobre los datos privados de la estación $k$.

#### Ciclo de Entrenamiento Federado por Rondas:
1. **Distribución:** El servidor central envía el vector de pesos global actual $\theta_t$ a los nodos participantes.
2. **Descenso de Gradiente Local (SGD):** Cada estación $k$ entrena su modelo local durante $E$ épocas con tasa de aprendizaje $\eta$:
   $$w_{t+1}^k = w_t^k - \eta \nabla F_k(w_t^k)$$
3. **Subida de Parámetros:** Cada estación envía únicamente sus nuevos pesos $w_{t+1}^k$ al servidor.
4. **Agregación Ponderada (FedAvg):** El servidor calcula el nuevo modelo global:
   $$\theta_{t+1} = \sum_{k=1}^K \frac{n_k}{n} w_{t+1}^k$$

### 5.3. Red Neuronal Ligera de Borde (Edge MLP)
Para estimar la impedancia eléctrica esperada y predecir la corriente en función de la tensión y las cargas conectadas, se implementa un modelo compacto:
* **Entrada ($\mathbb{R}^3$):** $[V, \text{relé}_1, \text{relé}_2]$
* **Capa Oculta:** 8 neuronas con activación no lineal ReLU:
  $$A_1 = \max(0, X W_1 + b_1)$$
* **Capa de Salida:** 1 neurona lineal para la corriente esperada:
  $$\hat{Y} = A_1 W_2 + b_2$$
* **Función de Pérdida:** Error Cuadrático Medio (MSE):
  $$\mathcal{L} = \frac{1}{n} \sum (Y_{real} - \hat{Y})^2$$

---

## 6. LAS 4 MÉTRICAS FUNDAMENTALES DEL PAPER (SECCIÓN IV)

Para la sustentación oral, dominar estas cuatro métricas es indispensable:

```
┌────────────────────────────────────────────────────────────────────────┐
│            LAS 4 MÉTRICAS DE EVALUACIÓN DEL PAPER CIENTÍFICO           │
├─────────────────────────┬──────────────────────────────────────────────┤
│ 1. Precisión de         │ Capacidad de detectar anomalías inducidas de │
│    Detección            │ vibración y calentamiento térmico en motores.│
├─────────────────────────┼──────────────────────────────────────────────┤
│ 2. Tráfico de Red       │ Reducción drástica de bytes transmitidos     │
│                         │ frente a arquitecturas centralizadas en nube.│
├─────────────────────────┼──────────────────────────────────────────────┤
│ 3. Heterogeneidad de    │ Capacidad de adaptación y convergencia ante  │
│    Datos (Non-IID)      │ motores con perfiles de carga disímiles.     │
├─────────────────────────┼──────────────────────────────────────────────┤
│ 4. Resistencia a        │ Estabilidad y autonomía de los nodos locales │
│    Desconexiones        │ ante pérdidas intermitentes de enlace Wi-Fi. │
└─────────────────────────┴──────────────────────────────────────────────┘
```

### 1. Precisión de Detección (*Detection Accuracy*)
* **Definición:** Tasa de acierto y sensibilidad (*Recall*) del sistema ante anomalías físicas inducidas deliberadamente en el banco de pruebas:
  * Desbalance mecánico agregando masa excéntrica a las aspas del abanico (registrado por ADXL345).
  * Sobrecarga térmica tapando la ventilación para calentar el estator (registrado por DS18B20).
  * Falla eléctrica conmutando cargas indebidas (registrado por INA219).

### 2. Tráfico de Red (*Network Traffic Mitigation*)
* **Comparativa:** 
  * *Arquitectura Centralizada:* Muestreando 3 ejes de aceleración a 200 Hz (16 bits) + telemetría eléctrica genera $> 100\,\text{MB}$ de datos crudos por máquina al día.
  * *Arquitectura Federada:* El modelo local realiza la inferencia en el borde. Solo se envían al servidor los pesos de la red ($\approx 40$ valores de coma flotante $\approx 160\,\text{bytes}$) una vez por ronda federada.
  * **Resultado:** Reducción superior al **95% - 98%** en el volumen de datos en tránsito.

### 3. Heterogeneidad de Datos (*Data Heterogeneity / Non-IID Robustness*)
* **Desafío Estadístico:** En una planta real, los datos no son Independientes e Idénticamente Distribuidos (Non-IID). Un motor puede estar al 30% de carga y otro al 90%; uno puede ser nuevo y otro tener holgura mecánica.
* **Evaluación:** Demostrar que el algoritmo FedAvg converge a un modelo global robusto que generaliza el concepto de falla sin desestabilizarse por el sesgo particular de una estación.

### 4. Resistencia a Desconexiones (*Disconnection Resilience*)
* **Prueba de Campo:** Se inducen micro-cortes y pérdida prolongada de balizas Wi-Fi entre los ESP32 y el servidor.
* **Resultado Esperado:** El nodo de borde no bloquea su bucle de control, mantiene la supervisión del hardware, apaga los relés localmente si ocurre una falla eléctrica y almacena los gradientes para sincronizarlos al reconectar.

---

## 7. BANCO EXHAUSTIVO DE PREGUNTAS Y RESPUESTAS PARA EXAMEN / SUSTENTACIÓN

### Pregunta 1: ¿Cuál es la diferencia conceptual entre Edge Computing (AIoT) y Cloud Computing en este proyecto?
> **Respuesta:** En Cloud Computing, los nodos son pasivos ("tontos") y se limitan a enviar mediciones crudas a un servidor remoto que procesa y decide. En AIoT (Edge Computing), el microcontrolador ESP32 posee capacidad de cómputo local: acondiciona la señal, ejecuta inferencia en tiempo real, aplica reglas de protección física instantáneas (< 1 ms) y calcula gradientes matemáticos, utilizando la red únicamente para coordinar conocimientos globales.

### Pregunta 2: ¿Por qué en Federated Learning se envían pesos y no datos crudos?
> **Respuesta:** Por dos principios fundamentales: **Privacidad** (los datos operativos de la máquina no se exponen ni salen del borde) y **Eficiencia de Red** (el vector de pesos ocupa una fracción ínfima de memoria comparado con el flujo continuo de series temporales de alta velocidad).

### Pregunta 3: Explique matemáticamente cómo funciona la agregación en FedAvg.
> **Respuesta:** Cada cliente $k$ entrena su modelo y obtiene un vector de pesos $w_k$. El servidor calcula el promedio ponderado asignando mayor peso a los clientes que contribuyeron con más muestras de entrenamiento:
> $$\theta_{global} = \sum_{k=1}^K \frac{n_k}{n_{total}} w_k$$
> Donde $n_k$ es el número de muestras de la estación $k$ y $n_{total}$ es la suma de muestras de todos los clientes participantes.

### Pregunta 4: ¿Por qué fue mandatorio retirar el jumper JD-VCC del módulo de relés?
> **Respuesta:** Para lograr aislamiento galvánico real. El jumper une la alimentación de las bobinas electromecánicas de 5V con los LEDs del optoacoplador. Al removerlo, `JD-VCC` se energiza con una fuente externa de 5V que soporta los picos inductivos de conmutación, mientras que `VCC` se alimenta con los 3.3V del ESP32. Esto evita reseteos por *brownout* y asegura que el relé se apague completamente al eliminar la tensión residual ($5\text{V} - 3.3\text{V} = 1.7\text{V}$).

### Pregunta 5: ¿Qué problema resuelve la topología de "Tierra en Estrella" (Star Ground)?
> **Respuesta:** Resuelve el acoplamiento resistivo de masa. Cuando una carga de potencia drena corriente, la resistencia del cableado genera una caída $I \times R$. Si la masa de control del microcontrolador comparte esa línea, su nivel de referencia de 0V se desplaza, provocando ruidos espurios en el bus I2C (INA219, ADXL345, OLED) o disparos del circuito de reinicio interno del microcontrolador. La estrella garantiza que cada circuito retorne independientemente al borne negativo de la fuente.

### Pregunta 6: ¿Por qué se implementó ESP-NOW además de MQTT en el firmware de ARCEUS GRID?
> **Respuesta:** ESP-NOW permite crear un bus inalámbrico punto a multipunto ultrarrápido y seguro. Los nodos de medición no necesitan conectarse al router Wi-Fi ni negociar DHCP, reduciendo el consumo energético y el tamaño de la trama a 25 bytes binarios. El ESP32 Gateway central es el único que se asocia al Wi-Fi para hacer de puente hacia el broker MQTT Mosquitto, blindando los nodos sensores contra ataques de red.

### Pregunta 7: ¿Qué normativas IPC se aplicaron en la PCB y cuál es su relevancia industrial?
> **Respuesta:** Se aplicaron la **IPC-2221B** (diseño genérico, dimensionamiento de ancho de pistas según corriente admisible y distancias de aislamiento dieléctrico *clearance/creepage*) y la **IPC-2222** (estándar para circuitos rígidos de 1 oz de cobre). Su relevancia radica en garantizar que la tarjeta tolere picos de conmutación de los motores sin sobrecalentamiento de pistas ni arcos eléctricos.

### Pregunta 8: ¿Cómo se gestionan las anomalías sin que el microcontrolador se bloquee?
> **Respuesta:** Mediante programación estrictamente no bloqueante basada en la función `millis()`. Se descarta el uso de `delay()`, permitiendo que el microcontrolador ejecute su máquina de estados cíclicamente. Esto asegura que la función de protección por sobrecorriente ($I > 2.8\,\text{A}$) evalúe el sensor en cada iteración y conmute los relés en microsegundos.

### Pregunta 9: ¿Qué es el fenómeno de datos Non-IID y cómo afecta al modelo federado?
> **Respuesta:** Non-IID significa que los datos entre estaciones no son idénticamente distribuidos. En la práctica, un motor puede presentar mayores vibraciones por desgaste y otro operar a bajas revoluciones. Si no se compensa, los gradientes locales apuntan en direcciones divergentes. FedAvg lo mitiga promediando las contribuciones ponderadas y aumentando las épocas locales antes de sincronizar.

### Pregunta 10: ¿Qué ventajas aporta SQLite con modo WAL en el servicio coordinador?
> **Respuesta:** El modo WAL (*Write-Ahead Logging*) permite concurrencia real entre hilos: los procesos de lectura (el dashboard de Streamlit consultando gráficos históricos) no bloquean a los procesos de escritura (el daemon MQTT insertando telemetría en tiempo real a alta frecuencia), eliminando el error clásico de base de datos bloqueada (`database is locked`).

---

## 8. GLOSARIO TÉCNICO

* **AIoT (Artificial Intelligence of Things):** Integración de algoritmos de inteligencia artificial directamente en dispositivos embebidos de IoT para procesar datos en el origen.
* **Back-EMF (Fuerza Contraelectromotriz):** Pico de tensión destructivo generado por una bobina inductiva (relé o devanado de motor) al interrumpir abruptamente la corriente. Requiere diodos de rueda libre (*Flyback*).
* **Brownout Reset (BOR):** Mecanismo de seguridad de silicio del ESP32 que reinicia la CPU cuando la tensión de alimentación cae por debajo de su umbral operativo seguro ($\approx 2.43\,\text{V}$).
* **ESP-NOW:** Protocolo de comunicación inalámbrica desarrollado por Espressif que permite el intercambio de paquetes breves sin conexión entre microcontroladores ESP32/ESP8266.
* **FedAvg (Federated Averaging):** Algoritmo canónico de Aprendizaje Federado propuesto por McMahan et al. (2017) que consolida modelos locales promediando sus pesos de forma ponderada.
* **High-Side Sensing:** Técnica de medición de corriente colocando la resistencia shunt entre el polo positivo de la fuente y la carga, lo que preserva la integridad del plano de tierra.
* **Non-IID:** Distribución de datos no independientes y no idénticamente distribuidos entre los diferentes nodos de una red federada.
* **OneWire:** Protocolo de bus serie asíncrono maestro-esclavo que utiliza una única línea de datos más tierra (usado por el sensor DS18B20).
* **Star Ground (Masa en Estrella):** Técnica de trazado de tierras donde todos los retornos convergen físicamente en un único nodo equipotencial.

---

## 9. REFERENCIAS BIBLIOGRÁFICAS FORMALES

1. **B. McMahan, E. Moore, D. Ramage, S. Hampson, and B. A. y Arcas**, "Communication-efficient learning of deep networks from decentralized data," in *Artificial Intelligence and Statistics*, PMLR, 2017, pp. 1273–1282.
2. **K. Bonawitz et al.**, "Towards federated learning at scale: System design," *Proceedings of Machine Learning and Systems*, vol. 1, pp. 374–388, 2019.
3. **Espressif Systems**, *ESP32 Series Datasheet*, Rev. 3.7, Espressif Systems, 2023.
4. **IPC Association Connecting Electronics Industries**, *IPC-2221B: Generic Standard on Printed Board Design*, IPC, 2012.
5. **IPC Association Connecting Electronics Industries**, *IPC-2222: Sectional Design Standard for Rigid Printed Boards*, IPC, 2020.
6. **L. Nieto**, *Arceus Grid Repository*, GitHub repository, 2024. [En línea]. Disponible en: `https://github.com/LNieto-V/arceus_grid`.
