# 🧠 ARCEUS GRID — Fundamentación Matemática y Algorítmica Completa

Este documento detalla rigurosamente la justificación teórica, deducción matemática y funcionamiento paso a paso del modelo implementado en **ARCEUS GRID** (`coordinator/ai/federated_learning.py` y `coordinator/ai/anomaly_detector.py`).

---

## 1. ¿Por qué se utilizó este modelo matemático?

En un sistema eléctrico experimental, surge una pregunta inmediata:  
*¿Por qué no usar simplemente la Ley de Ohm ($I = V / R$) o enviar todos los datos a una base de datos central para hacer una regresión lineal común?*

Existen tres razones físicas, computacionales y de seguridad por las cuales se adoptó una **Red Neuronal Ligera Local (MLP)** acoplada con **Aprendizaje Federado (FedAvg)**:

### 1.1. No Linealidad Física de las Cargas Reales
* Una bombilla o LED **no es una resistencia óhmica pura**. 
* La resistencia interna de las cargas varía con la **temperatura del filamento/diodo** y con la **tensión de alimentación** ($R(T, V)$).
* La conmutación de los relés genera resistencias de contacto parásitas variables ($R_{contacto} \approx 10\text{ m}\Omega - 50\text{ m}\Omega$) y caídas de tensión en los conductores.
* Un modelo rígido determinista falla al predecir la corriente exacta bajo distintas combinaciones de carga y temperatura.

### 1.2. El Paradigma de la Privacidad y el Ancho de Banda (Por qué Aprendizaje Federado)
* **Enfoque Tradicional Centralizado:** Cada estación envía continuamente miles de muestras crudas $(V, I, P, R_1, R_2)$ por segundo a través de Wi-Fi.
  * *Consecuencia:* Satura el router, crea cuellos de botella y expone los hábitos de consumo de la instalación (en una microrred real, revela horarios de actividad, presencia de personas o procesos industriales confidenciales).
* **Enfoque Federado de ARCEUS GRID:** Los datos crudos **nunca salen de la estación**. Cada estación entrena su modelo localmente y lo único que viaja por la red son los **pesos matemáticos** ($\mathbf{W}, \mathbf{b}$). Se reduce el tráfico de red en más de un **95%**.

---

## 2. Nivel 1: El Modelo Predictivo Local (Perceptrón Multicapa 3-8-1)

El modelo local está programado en `coordinator/ai/federated_learning.py` bajo la clase `LocalModel`. Es una red neuronal feedforward completamente diferenciable diseñada para ejecutarse con consumo mínimo de CPU y memoria.

### 2.1. Vector de Entrada ($\mathbf{x}$)
La red toma un vector tridimensional que representa el estado operacional en un instante $t$:
$$\mathbf{x} = \begin{bmatrix} V & R_1 & R_2 \end{bmatrix} \in \mathbb{R}^{1 \times 3}$$
* $V \in \mathbb{R}^+$: Tensión de bus medida por el INA219 (típicamente $\approx 12.0\text{ V}$).
* $R_1 \in \{0, 1\}$: Estado digital de la Bombilla 1 (0: Apagada, 1: Encendida).
* $R_2 \in \{0, 1\}$: Estado digital de la Bombilla 2 (0: Apagada, 1: Encendida).

### 2.2. Capa Oculta (Hidden Layer con Activación ReLU)
La entrada se proyecta a un espacio latente de dimensión $h = 8$ para capturar interacciones no lineales entre la tensión y la activación de relés:

$$\mathbf{z}_1 = \mathbf{x} \mathbf{W}_1 + \mathbf{b}_1$$

$$\mathbf{a}_1 = \max(0, \mathbf{z}_1) \quad (\text{Función de Activación ReLU})$$

* $\mathbf{W}_1 \in \mathbb{R}^{3 \times 8}$: Matriz de pesos de la capa oculta.
* $\mathbf{b}_1 \in \mathbb{R}^{1 \times 8}$: Vector de sesgos (*biases*).
* **Inicialización He / Kaiming:** Los pesos se inicializan a partir de una distribución normal con desviación estándar:
  $$\sigma = \sqrt{\frac{2}{n_{in}}} = \sqrt{\frac{2}{3}} \approx 0.816$$
  Esto previene el desvanecimiento (*vanishing*) o explosión del gradiente durante las primeras iteraciones.

### 2.3. Capa de Salida (Linear Output)
La capa de salida combina las activaciones para estimar la corriente consumida esperada $\hat{y} = \hat{I}$ (en Amperios):

$$\hat{y} = \mathbf{a}_1 \mathbf{W}_2 + b_2$$

* $\mathbf{W}_2 \in \mathbb{R}^{8 \times 1}$: Vector de pesos de salida.
* $b_2 \in \mathbb{R}$: Sesgo de salida.

---

## 3. Optimización Local: Descenso de Gradiente Estocástico (SGD)

Para un lote de $n$ mediciones, el objetivo es minimizar el **Error Cuadrático Medio (MSE)**:

$$\mathcal{L} = \frac{1}{n} \sum_{i=1}^n (\hat{y}_i - y_i)^2$$

### 3.1. Deducción de Gradientes (Backpropagation)

1. **Gradiente de la salida:**
   $$\frac{\partial \mathcal{L}}{\partial \hat{y}} = \frac{2}{n} (\hat{y} - y)$$

2. **Gradientes de la capa de salida ($\mathbf{W}_2, b_2$):**
   $$\frac{\partial \mathcal{L}}{\partial \mathbf{W}_2} = \mathbf{a}_1^T \left(\frac{\partial \mathcal{L}}{\partial \hat{y}}\right)$$
   $$\frac{\partial \mathcal{L}}{\partial b_2} = \sum_{i=1}^n \left(\frac{\partial \mathcal{L}}{\partial \hat{y}}\right)_i$$

3. **Propagación hacia la capa oculta:**
   $$\frac{\partial \mathcal{L}}{\partial \mathbf{a}_1} = \left(\frac{\partial \mathcal{L}}{\partial \hat{y}}\right) \mathbf{W}_2^T$$
   $$\frac{\partial \mathcal{L}}{\partial \mathbf{z}_1} = \frac{\partial \mathcal{L}}{\partial \mathbf{a}_1} \odot \mathbb{I}(\mathbf{z}_1 > 0)$$
   *(donde $\odot$ es el producto Hadamard elemento a elemento y $\mathbb{I}(\mathbf{z}_1 > 0)$ es la derivada de ReLU).*

4. **Gradientes de la capa de entrada ($\mathbf{W}_1, \mathbf{b}_1$):**
   $$\frac{\partial \mathcal{L}}{\partial \mathbf{W}_1} = \mathbf{x}^T \left(\frac{\partial \mathcal{L}}{\partial \mathbf{z}_1}\right)$$
   $$\frac{\partial \mathcal{L}}{\partial \mathbf{b}_1} = \sum_{i=1}^n \left(\frac{\partial \mathcal{L}}{\partial \mathbf{z}_1}\right)_i$$

5. **Regla de Actualización de Pesos:**
   $$\theta \leftarrow \theta - \eta \cdot \nabla_\theta \mathcal{L}$$
   Con tasa de aprendizaje fija $\eta = 0.01$ y $E = 5$ épocas locales.

---

## 4. Nivel 2: Algoritmo de Agregación Global (FedAvg)

El coordinador orquesta las rondas de federación entre la Estación A y la Estación B utilizando el algoritmo canónico **Federated Averaging** (McMahan et al.):

### 4.1. La Ecuación de Agregación Ponderada
Sean $K$ estaciones participantes (en nuestro banco, $K = 2$: Estación A y Estación B).  
Sea $n_k$ el número de mediciones eléctricas locales disponibles en la estación $k$, y sea el total de muestras:

$$N = \sum_{k=1}^K n_k = n_A + n_B$$

En la ronda federada $t+1$, el servidor central recopila los parámetros locales actualizados $\theta_{t+1}^k = \{\mathbf{W}_1^k, \mathbf{b}_1^k, \mathbf{W}_2^k, \mathbf{b}_2^k\}$ y calcula el promedio ponderado:

$$\theta_{t+1}^{global} = \sum_{k=1}^K \left( \frac{n_k}{N} \right) \theta_{t+1}^k = \left( \frac{n_A}{n_A + n_B} \right) \theta_{t+1}^A + \left( \frac{n_B}{n_A + n_B} \right) \theta_{t+1}^B$$

### 4.2. ¿Por qué la ponderación por $\frac{n_k}{N}$ es crítica?
* Si la **Estación A** ha estado encendida durante 2 horas ($n_A = 2000$ muestras) y la **Estación B** acaba de conectarse hace 3 minutos ($n_B = 100$ muestras):
  * Un promedio simple daría $50\%$ de peso a cada estación, haciendo que el modelo global se desestabilice por el poco entrenamiento de B.
  * El factor ponderado otorga $\frac{2000}{2100} \approx 95.2\%$ de influencia a A y $\frac{100}{2100} \approx 4.8\%$ a B, garantizando estabilidad matemática y convergencia asintótica.

---

## 5. Nivel 3: El Modelo de Detección de Anomalías (Híbrido Determinista + Estadístico)

Implementado en `coordinator/ai/anomaly_detector.py`, combina dos barreras matemáticas:

### 5.1. Barrera 1: Coherencia Física y Ley de Potencia
El sensor INA219 calcula internamente la potencia multiplicando el voltaje y la corriente. El software evalúa el residuo de potencia $\Delta P$:
$$\Delta P = |P_{\text{medida}} - (V \times I)|$$
* Si $\Delta P > 1.2\text{ W}$, se diagnostica una falla interna en el ADC del sensor o ruido de conmutación severo.

### 5.2. Barrera 2: Puntuación Normalizada de Anomalía (Z-Score)
Para detectar caídas lentas de tensión (*brownouts*) o degradación de fuentes conmutadas, se utiliza la distancia estándar normalizada:
$$Z_V = \frac{|V - \mu_V|}{\sigma_V}$$
Donde la tensión nominal es $\mu_V = 12.0\text{ V}$ y la desviación estándar de operación segura es $\sigma_V = 0.35\text{ V}$.
* Una muestra con $Z_V > 3.0$ se encuentra a más de tres desviaciones estándar de la media ($p < 0.003$), activando de inmediato el modo de advertencia en los pilotos amarillos y el dashboard.
