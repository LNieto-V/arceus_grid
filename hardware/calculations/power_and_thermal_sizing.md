# Memoria de Cálculo Eléctrico y Térmico — ARCEUS GRID

## 1. Balance de Potencia y Corriente Máxima Simultánea

Para dimensionar correctamente la fuente central de 12 V DC, los conductores y las protecciones, se establece el balance de consumo en el peor caso operativo (**Worst-Case Scenario**), donde ambas estaciones operan con todas sus cargas activas simultáneamente.

### Parámetros de Cargas Unitarias:
- **Bombillas LED de 12V DC (4 unidades en total):**
  - Potencia nominal estimada: $P_{bulb} = 5.0\,\text{W}$ a $6.0\,\text{W}$ por bombilla.
  - Corriente unitaria: $I_{bulb} = \frac{5.5\,\text{W}}{12.0\,\text{V}} \approx 0.46\,\text{A}$ ($460\,\text{mA}$).
- **Pilotos Indicadores de 12V DC (máximo 1 activo simultáneamente por estación):**
  - Corriente unitaria: $I_{pilot} \approx 0.05\,\text{A}$ a $0.10\,\text{A}$ ($50-100\,\text{mA}$).
- **ESP32 + Pantalla OLED + INA219 (consumo en 5V a la salida del LM2596):**
  - Pico con WiFi TX (transmisión MQTT): $I_{peak\_5V} \approx 250\,\text{mA}$ ($0.25\,\text{A}$).
  - Potencia en 5V: $P_{control} = 5.0\,\text{V} \times 0.25\,\text{A} = 1.25\,\text{W}$.
  - Con eficiencia típica del convertidor buck LM2596 de $\eta \approx 80\%$:
    $$P_{in\_12V} = \frac{1.25\,\text{W}}{0.80} = 1.56\,\text{W} \implies I_{in\_12V} = \frac{1.56\,\text{W}}{12.0\,\text{V}} \approx 0.13\,\text{A}$$
- **Bobinas del Módulo de 4 Relés (alimentadas a 5V desde LM2596):**
  - $4 \times 72\,\text{mA} = 288\,\text{mA}$ a 5V $\implies P = 5\,\text{V} \times 0.288\,\text{A} = 1.44\,\text{W}$.
  - Corriente equivalente en el bus de 12V: $\approx 0.15\,\text{A}$.

### Resumen de Corrientes Máximas en 12V:

| Subsistema | Estación A (A) | Estación B (A) | Sistema Central (A) | Total (A) |
|---|---|---|---|---|
| 2 Bombillas LED 12V | $2 \times 0.46 = 0.92\,\text{A}$ | $2 \times 0.46 = 0.92\,\text{A}$ | — | $1.84\,\text{A}$ |
| 1 Piloto 12V activo | $0.08\,\text{A}$ | $0.08\,\text{A}$ | — | $0.16\,\text{A}$ |
| ESP32 + Sensores + OLED | $0.13\,\text{A}$ | $0.13\,\text{A}$ | — | $0.26\,\text{A}$ |
| 4 Bobinas de Relés (5V) | — | — | $0.15\,\text{A}$ | $0.15\,\text{A}$ |
| **Subtotal Corriente Nominal** | **$1.13\,\text{A}$** | **$1.13\,\text{A}$** | **$0.15\,\text{A}$** | **$2.41\,\text{A}$** |

---

## 2. Dimensionamiento de la Fuente de Alimentación de 12V

Aplicando un factor de seguridad estándar del **40% de sobrecapacidad** (según prácticas recomendadas IEEE/IEC para evitar estrés térmico en fuentes conmutadas cerradas):
$$I_{fuente\_req} = I_{total} \times 1.40 = 2.41\,\text{A} \times 1.40 = 3.37\,\text{A}$$
$$P_{fuente\_req} = 12.0\,\text{V} \times 3.37\,\text{A} \approx 40.5\,\text{W}$$

> [!IMPORTANT]
> **Especificación Seleccionada para la Fuente Central:**  
> Fuente de alimentación conmutada industrial de **12 V DC, 5 A (60 W)** (por ejemplo, tipo Mean Well LRS-50-12 o LRS-75-12).  
> Esta fuente operará a un régimen cómodo del 48% de su capacidad nominal continua.

---

## 3. Dimensionamiento de Protecciones (Fusibles)

Para proteger el circuito contra cortocircuitos francos o fallas catastróficas en el cableado:

1. **Fusible General de Entrada ($F_1$):**
   - Ubicación: En serie con el polo positivo (+) inmediatamente a la salida de la fuente de 12 V.
   - Corriente de corte: **$F_1 = 4.0\,\text{A}$ o $5.0\,\text{A}$ (Tipo rápido F / Fast-Acting, $5 \times 20\,\text{mm}$ o tipo automotriz ATO/ATC)**.
2. **Fusibles Individuales por Estación ($F_{A}$ y $F_{B}$):**
   - Ubicación: Antes de la bornera `VIN+` del sensor INA219 de cada estación.
   - Corriente de disparo: **$2.0\,\text{A}$ (Fusión Rápida)**.
   - Justificación: La corriente nominal de las 2 bombillas es $\approx 0.92\,\text{A}$. Un fusible de 2.0 A permite las corrientes de inrush capacitivas del encendido sin disparos en falso, pero desconectará la estación en menos de 100 ms ante un cortocircuito.

---

## 4. Selección de Calibre de Conductores (AWG)

Criterio: Caída de tensión máxima admisible en corriente continua $< 2.0\%$ a plena carga ($V_{drop\_max} < 0.24\,\text{V}$) y densidad de corriente segura $< 4\,\text{A/mm}^2$.

- **Línea Troncal de 12 V y Retorno GND Central:**
  - Corriente: hasta 3.5 A pico.
  - Longitud estimada de mazo: $1.0\,\text{m}$.
  - Conductor recomendado: **AWG 18 ($0.82\,\text{mm}^2$) o AWG 16 ($1.31\,\text{mm}^2$)**.
  - Resistencia de AWG 18: $\approx 21\,\text{m}\Omega/\text{m}$. Caída para 2 m de ida y vuelta a 2.5 A:
    $$V_{drop} = 2.5\,\text{A} \times 0.042\,\Omega = 0.105\,\text{V} \implies 0.87\% \quad (\text{Excelente})$$
- **Líneas de Distribución a Bombillas y Relés:**
  - Corriente: $0.5\,\text{A}$ a $1.0\,\text{A}$.
  - Conductor recomendado: **AWG 20 ($0.52\,\text{mm}^2$) o AWG 22 ($0.33\,\text{mm}^2$)**.
- **Líneas de Señal I2C y GPIOs (Control):**
  - Corriente: $< 15\,\text{mA}$.
  - Conductor recomendado: **AWG 24 o AWG 26 (Cables tipo Dupont flexibles)**.

---

## 5. Análisis Térmico del Convertidor Step-Down LM2596

El LM2596 opera mediante conmutación a $150\,\text{kHz}$.
- Tensión de entrada: $V_{in} = 12.0\,\text{V}$
- Tensión de salida: $V_{out} = 5.0\,\text{V}$
- Corriente de salida: $I_{out} \approx 0.35\,\text{A}$ (ESP32 + OLED + 2 bobinas de relé de la estación)
- Eficiencia del módulo comercial: $\eta \approx 82\%$
- Potencia entregada a la carga: $P_{out} = 5.0\,\text{V} \times 0.35\,\text{A} = 1.75\,\text{W}$
- Potencia absorbida: $P_{in} = \frac{1.75\,\text{W}}{0.82} = 2.13\,\text{W}$
- **Potencia Disipada por el Circuito Integrado LM2596:**
  $$P_{dis} = P_{in} - P_{out} = 2.13\,\text{W} - 1.75\,\text{W} = 0.38\,\text{W}$$
- Resistencia térmica unión-ambiente con plano de cobre PCB: $\theta_{JA} \approx 50\,^\circ\text{C/W}$
- Elevación de temperatura sobre la temperatura ambiente:
  $$\Delta T = P_{dis} \times \theta_{JA} = 0.38\,\text{W} \times 50\,^\circ\text{C/W} = 19\,^\circ\text{C}$$
- A temperatura ambiente de $25\,^\circ\text{C}$, el encapsulado TO-263 operará a $\approx 44\,^\circ\text{C}$, muy por debajo del límite de $125\,^\circ\text{C}$.  
  **No requiere disipador adicional de aluminio para estas corrientes.**
