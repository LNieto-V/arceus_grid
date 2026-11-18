# 🧠 MÓDULO 1: INGENIERÍA DE INTELIGENCIA ARTIFICIAL & FEDERATED LEARNING
## Dominio Técnico de Pie a Cabeza para Owen Mora

---

## 1. EL GRAN DILEMA INDUSTRIAL: ¿POR QUÉ NO CLOUD Y POR QUÉ AIoT?

En la industria tradicional de mantenimiento predictivo (PdM), la maquinaria envía sus datos crudos a servidores en la nube (AWS, Azure o Google Cloud). En ARCEUS GRID demostramos que ese modelo colapsa por tres fallas estructurales:

1. **Saturación del Ancho de Banda:** Muestrear aceleración triaxial (ADXL345) a 200 Hz con resolución de 16 bits genera $> 100\,\text{MB}$ de telemetría diaria por cada motor. En una planta con 50 motores, son gigabytes continuos compitiendo con el control operacional.
2. **Latencia Inadmisible:** Una llamada HTTP/Cloud tarda entre $80$ y $300\,\text{ms}$ en responder. Una sobrecorriente destructiva o un desbalance crítico quema un bobinado en menos de $10\,\text{ms}$.
3. **Privacidad y Secreto Industrial:** Ninguna fábrica quiere transmitir patrones de vibración o perfiles de carga a servidores externos, porque revelan su volumen exacto de producción e información propietaria.

**Solución de ARCEUS GRID:** **AIoT (Artificial Intelligence of Things)**. La inferencia y la protección residen en el borde (microcontrolador ESP32 y computador coordinador local). Los datos crudos mueren donde nacen; solo se sincronizan parámetros de aprendizaje.

---

## 2. ARQUITECTURA DE DETECCIÓN DUAL (EL NÚCLEO DEL SISTEMA)

Un error novato es responder que *"la inteligencia artificial corta la energía si hay cortocircuito"*. Un jurado de ingeniería te descalificará si dices eso. 

La respuesta correcta es: **"Implementamos una Arquitectura de Detección en Dos Capas con separación estricta de responsabilidades"**:

```
                              TELEMETRÍA ENTRANTE (INA219 / ESP32)
                                               │
             ┌─────────────────────────────────┴─────────────────────────────────┐
             ▼                                                                   ▼
   [CAPA 1: GUARDRAILS DETERMINISTAS]                                  [CAPA 2: ML & ESTADÍSTICA]
   • Ejecución: C++ embebido en ESP32 (< 1 ms)                         • Ejecución: Python / NumPy en Coordinador
   • Filosofía: Reglas físicas rígidas (Fail-Safe)                     • Filosofía: Detección de degradación progresiva
   • Umbrales de corte inmediato:                                      • Componentes:
     - I > 2.8 A  ──► Disparo instantáneo de relé                        - Z-Score normalizado de tensión (Z > 0.5)
     - V < 10.0 V ──► Subtensión crítica                                 - Edge MLP: Impedancia y corriente esperada
     - V > 14.0 V ──► Sobretensión peligrosa                             - Discrepancia INA219 (|P - V*I| > 1.2 W)
     - Relés OFF e I > 40 mA ──► Fuga a tierra                         • Detecta:
     - Relés ON e I < 30 mA ──► Circuito abierto / rotura                 - Bornes oxidados, descalibración lenta,
   • Detecta: Fallas catastróficas inmediatas                             pérdida de aislamiento, fatiga mecánica
```

---

## 3. LA RED NEURONAL DE BORDE (EDGE MLP)

En `coordinator/ai/federated_learning.py` implementamos un modelo de red neuronal perceptrón multicapa (MLP) ultraligero para perfilar el comportamiento eléctrico.

### 3.1. ¿Cuál es el objetivo de la Red?
Aprender la función de impedancia y predecir la **corriente eléctrica esperada** ($\hat{I}$) en función del estado de alimentación y conmutación:

$$\hat{I} = f(V, R_1, R_2)$$

### 3.2. Topología de la Red:
* **Capa de Entrada ($X \in \mathbb{R}^3$):**  
  Vector de 3 variables: $[V, R_1, R_2]$ (Tensión medida por INA219, estado de Relé 1 [0/1], estado de Relé 2 [0/1]).
* **Capa Oculta (8 neuronas, activación ReLU):**  
  $$Z_1 = X \cdot W_1 + b_1 \quad (W_1 \in \mathbb{R}^{3 \times 8}, \; b_1 \in \mathbb{R}^{1 \times 8})$$
  $$A_1 = \max(0, Z_1)$$  
  *¿Por qué ReLU?* Porque introduce no-linealidad sin requerir exponenciales pesadas (como Sigmoide o Tanh), siendo trivial de computar en hardware restringido.
* **Capa de Salida (1 neurona, activación Lineal):**  
  $$\hat{Y} = \hat{I} = A_1 \cdot W_2 + b_2 \quad (W_2 \in \mathbb{R}^{8 \times 1}, \; b_2 \in \mathbb{R}^{1 \times 1})$$
* **Función de Pérdida (MSE — Error Cuadrático Medio):**  
  $$\mathcal{L} = \frac{1}{m} \sum_{i=1}^m (I_{real}^{(i)} - \hat{I}^{(i)})^2$$

### 3.3. ¿Por qué usamos NumPy puro y no PyTorch o TensorFlow?
* **Consumo de Memoria:** PyTorch o TensorFlow cargan binarios de cientos de megabytes y requieren $> 400\,\text{MB}$ de RAM solo para inicializar.
* **Eficiencia de Borde:** Nuestra implementación en NumPy pesa menos de **25 KB** en memoria, ejecuta la inferencia en menos de **$0.15\,\text{ms}$** y puede portarse directamente a C puro para correr dentro del ESP32.

---

## 4. APRENDIZAJE FEDERADO (ALGORITMO FEDAVG)

### 4.1. ¿Qué es Federated Learning?
Es un paradigma donde múltiples dispositivos (Estación A y Estación B) entrenan colaborativamente un modelo global coordinado por un servidor central, **sin compartir jamás sus conjuntos de datos locales**.

### 4.2. El Ciclo por Rondas de FedAvg (Paso a Paso):
1. **Paso 1 (Distribución):** El coordinador central envía los pesos globales actuales $\theta_t$ a la Estación A y a la Estación B.
2. **Paso 2 (Entrenamiento Local - SGD):** Cada estación entrena su modelo local sobre sus propios datos durante $E = 5$ épocas con tasa de aprendizaje $\eta = 0.01$:
   $$w_{t+1}^k = w_t^k - \eta \nabla F_k(w_t^k)$$
3. **Paso 3 (Subida de Pesos):** Cada estación transmite **únicamente** su nuevo vector de pesos $w_{t+1}^k$ al coordinador vía MQTT/red.
4. **Paso 4 (Agregación Ponderada - FedAvg):** El coordinador calcula el nuevo modelo promediando ponderadamente según la cantidad de datos aportados por cada estación:
   $$\theta_{t+1} = \sum_{k=1}^K \frac{n_k}{n_{total}} w_{t+1}^k = \left( \frac{n_A}{n_A + n_B} \right) w_{t+1}^A + \left( \frac{n_B}{n_A + n_B} \right) w_{t+1}^B$$
5. **Paso 5 (Re-distribución):** El nuevo modelo $\theta_{t+1}$ se convierte en la base para la siguiente ronda.

---

## 5. EL FENÓMENO DE DATOS NON-IID (LA PREGUNTA FAVORITA DEL JURADO)

### ¿Qué significa Non-IID?
Significa datos **No Independientes e Idénticamente Distribuidos** (*Non-Independent and Identically Distributed*).

### ¿Por qué ocurre en nuestro proyecto?
En una fábrica real, la máquina de la Estación A puede ser un motor nuevo operando al $40\%$ de carga con ventilación óptima, mientras que el motor de la Estación B tiene holgura en los rodamientos y opera al $85\%$ de carga. Sus distribuciones de corriente y temperatura son totalmente distintas:

$$P_A(X, Y) \neq P_B(X, Y)$$

### ¿Cómo lo resuelve FedAvg en ARCEUS GRID?
1. **Ponderación Proporcional:** Al multiplicar por $\frac{n_k}{n_{total}}$, una estación con pocas muestras no puede sesgar bruscamente el modelo global.
2. **Múltiples Épocas Locales ($E$):** Entrenar varias épocas en cada estación permite que el modelo aprenda patrones estables de impedancia local antes de sincronizar gradientes.
3. **Convergencia a la Física Común:** El modelo global resultante converge hacia las leyes físicas comunes del electromagnetismo (relación tensión-corriente), descartando el ruido particular de cada máquina.

---

## 6. LAS 4 MÉTRICAS DE EVALUACIÓN DEL PAPER (SECCIÓN IV)

| Métrica | Definición | Resultado Obtenido en ARCEUS GRID |
|---|---|---|
| **1. Precisión de Detección** | Tasa de acierto ante desbalance mecánico y sobrecalentamiento | **$> 95\%$** de sensibilidad (*Recall*) sin falsos disparos. |
| **2. Tráfico de Red** | Reducción del volumen de bytes transmitidos vs. streaming centralizado | **$> 95\%$ a $99\%$ de reducción** (de $103\,\text{MB/día}$ a $24\,\text{kB/día}$). |
| **3. Heterogeneidad (Non-IID)** | Estabilidad del modelo global ante perfiles de carga disímiles | Convergencia uniforme del error cuadrático medio (MSE $< 0.04$). |
| **4. Resiliencia a Desconexión** | Autonomía de operación si se cae el enlace Wi-Fi | **$100\%$ de disponibilidad local**: inferencia y disparo de relés sin pausa. |
