# 📐 MÓDULO 2: MODELAMIENTO MATEMÁTICO & FORMULARIO CLAVE
## Fórmulas, Demostraciones y Cifras Exactas de ARCEUS GRID

---

## 1. FORMULACIÓN MATEMÁTICA DE FEDERATED LEARNING (FEDAVG)

El aprendizaje federado resuelve un problema de optimización distribuida sobre $K$ estaciones:

$$\min_{w \in \mathbb{R}^d} f(w) \quad \text{donde} \quad f(w) = \sum_{k=1}^K \frac{n_k}{n} F_k(w)$$

### Parámetros:
* $K = 2$: Estación A y Estación B.
* $n_k = |\mathcal{D}_k|$: Número de observaciones capturadas por la estación $k$.
* $n = \sum_{k=1}^K n_k$: Total de observaciones de la microrred.
* $F_k(w) = \frac{1}{n_k} \sum_{i \in \mathcal{D}_k} \ell(x_i, y_i; w)$: Pérdida empírica local.

### Descenso de Gradiente Estocástico Local (Local SGD):
$$w_{t+1}^k = w_t^k - \eta \nabla F_k(w_t^k)$$

### Ecuación de Agregación Ponderada de Servidor (FedAvg):
$$\theta_{t+1} = \sum_{k=1}^K \frac{n_k}{n} w_{t+1}^k = \frac{n_A}{n_A + n_B} w_{t+1}^A + \frac{n_B}{n_A + n_B} w_{t+1}^B$$

---

## 2. RED NEURONAL DE BORDE (EDGE MLP) — ECUACIONES DE MATRICES

### Dimensiones:
* Entrada $X \in \mathbb{R}^{m \times 3}$: $[V, R_1, R_2]$
* Matriz de pesos capa 1: $W_1 \in \mathbb{R}^{3 \times 8}$, sesgo $b_1 \in \mathbb{R}^{1 \times 8}$
* Matriz de pesos capa 2: $W_2 \in \mathbb{R}^{8 \times 1}$, sesgo $b_2 \in \mathbb{R}^{1 \times 1}$
* Total de parámetros entrenables:
  $$\text{Parámetros} = (3 \times 8 + 8) + (8 \times 1 + 1) = 32 + 9 = \mathbf{41\,\text{parámetros}}$$
* Tamaño en memoria (Float32 = 4 bytes):
  $$\text{Vector de pesos} = 41 \times 4\,\text{bytes} = \mathbf{164\,\text{bytes}}$$

### Forward Pass:
1. $Z_1 = X \cdot W_1 + b_1$
2. $A_1 = \max(0, Z_1)$ (Activación ReLU)
3. $\hat{Y} = \hat{I} = A_1 \cdot W_2 + b_2$ (Salida lineal de corriente esperada)

### Función de Pérdida (MSE):
$$\mathcal{L} = \frac{1}{m} \sum_{i=1}^m (I_{real}^{(i)} - \hat{I}^{(i)})^2$$

---

## 3. DEMOSTRACIÓN DEL AHORRO >95% DE TRÁFICO DE RED

### Escenario A: Paradigma Centralizado Clásico en Nube
* Sensor ADXL345: 3 ejes ($X, Y, Z$) muestreados a $200\,\text{Hz}$ a 16 bits ($2\,\text{bytes/eje}$).
$$\text{Tasa de datos} = 200\,\text{Hz} \times 3 \times 2\,\text{bytes} = 1,200\,\text{bytes/s} = 1.2\,\text{kB/s}$$
$$\text{Consumo diario por máquina} = 1.2\,\text{kB/s} \times 86,400\,\text{s} = 103,680\,\text{kB} \approx \mathbf{103.68\,\text{MB/día}}$$

### Escenario B: Paradigma Federado ARCEUS GRID
* La inferencia ocurre en el borde (ESP32 / Coordinador local).
* Transmisión al servidor central: Únicamente el vector de pesos $W$ ($164\,\text{bytes}$) una vez por ronda federada (ej. cada 10 minutos):
$$\text{Rondas al día} = \frac{24 \times 60}{10} = 144\,\text{rondas/día}$$
$$\text{Consumo diario} = 144 \times 164\,\text{bytes} = 23,616\,\text{bytes} \approx \mathbf{23.6\,\text{kB/día}}$$

### Reducción Porcentual:
$$\Delta \text{Tráfico} = \left( 1 - \frac{23.6\,\text{kB}}{103,680\,\text{kB}} \right) \times 100\% = \mathbf{99.97\%\,\text{de reducción}}$$

---

## 4. FORMULARIO ELÉCTRICO Y FÍSICO

### Sensor INA219 (Shunt High-Side):
* $R_{shunt} = 0.1\,\Omega$
* $V_{shunt} = I \times R_{shunt}$
* Rango con PGA $\div 8$ ($V_{max} = 320\,\text{mV}$):
  $$I_{max} = \frac{0.320\,\text{V}}{0.1\,\Omega} = 3.20\,\text{A}$$
* Caída de tensión a $1\,\text{A}$:
  $$V_{drop} = 1.0\,\text{A} \times 0.1\,\Omega = 0.10\,\text{V} \quad (< 0.83\%\,\text{en bus de 12V})$$

### Coherencia de Potencia:
$$\Delta P = |P_{medida} - (V_{bus} \times I_{bus})|$$
$$\text{Si } \Delta P > 1.2\,\text{W} \implies \text{Anomalía de medición o reactiva.}$$

### Dimensionamiento Térmico de Pistas (Norma IPC-2221B):
$$A = \left( \frac{I}{k \cdot \Delta T^{0.44}} \right)^{\frac{1}{0.725}}$$
Para capas externas ($k = 0.048$), $I = 3\,\text{A}$, $\Delta T = 10^\circ\text{C}$:
$$W \approx 1.8\,\text{mm} - 2.0\,\text{mm} \quad (\text{para lámina de } 1\,\text{oz de cobre})$$

### Balance de Potencia y Factor de Seguridad (IEEE 40%):
$$I_{total} = (4 \times 0.46\,\text{A}) + (2 \times 0.08\,\text{A}) + (2 \times 0.13\,\text{A}) + 0.15\,\text{A} = 2.41\,\text{A}$$
$$I_{fuente} = 2.41\,\text{A} \times 1.40 = 3.37\,\text{A} \implies \text{Fuente seleccionada: } 12\,\text{V} @ 5\,\text{A} \; (60\,\text{W})$$
