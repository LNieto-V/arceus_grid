# Estudio y Análisis de Ingeniería: Etapa de Control para Pilotos Indicadores de 12V DC

## 1. Planteamiento del Problema Técnico

Cada estación de **ARCEUS GRID** requiere gobernar 3 pilotos industriales de 12 V DC (Verde, Amarillo y Rojo).  
El microcontrolador ESP32 opera a un nivel lógico de **3.3 V** en sus salidas de propósito general (GPIO27, GPIO32, GPIO33), con una corriente máxima absoluta recomendada por pin de **12 mA a 20 mA**.  
Los pilotos de panel de 12 V DC típicamente consumen entre **15 mA y 150 mA** (según si son LED o lámparas miniatura incandescentes) y operan a una tensión 3.6 veces superior al límite de tolerancia de los pines del ESP32.

> [!CAUTION]
> **REGLA ABSOLUTA:** Conectar directamente un piloto de 12 V o cualquier línea de 12 V a un pin GPIO del ESP32 destruirá de forma instantánea el pin y el silicio interno del microcontrolador.

---

## 2. Análisis de Topologías Candidatas

### Topología A: BJT 2N2222 como Pre-Driver + MOSFET Canal N de Potencia

Esta topología surge habitualmente cuando se dispone de MOSFETs de canal N de potencia estándar (e.g., IRF540N, IRF640, IRFZ44N) cuyo umbral de compuerta ($V_{GS(th)}$) es de 2.0 V a 4.0 V, pero requieren **$V_{GS} \ge 10\,\text{V}$** para entrar en la región óhmica profunda y alcanzar su $R_{DS(on)}$ nominal. Como el ESP32 solo suministra 3.3 V, no puede saturar un MOSFET estándar de forma directa.

#### Circuito Esquemático de la Topología A:
```
                +12V DC
                   |
             +-----+-----+
             |           |
          [R_pullup]  [PILOTO 12V] (Carga)
          (10 kOhm)      |
             |           +--------- DRENADOR (D)
             +--------------------- COMPUERTA (G) [MOSFET Canal N]
             |                     FUENTE (S)
             |                         |
      COLECTOR (C)                     |
           |                           |
  GPIO ---[R_base]--- BASE (B)         |
(ESP32)   (1 kOhm)   [2N2222]          |
                     EMISOR (E)        |
                         |             |
                         +------+------+
                                |
                           [GND COMÚN]
```

#### Análisis de Funcionamiento y Consideraciones Críticas:
1. **Inversión Lógica:**
   - Cuando el `GPIO = 0 V (LOW)`: El 2N2222 está en corte ($I_B = 0$). El resistor $R_{pullup}$ tira de la compuerta del MOSFET hacia +12 V ($V_{GS} = 12\,\text{V}$). El MOSFET conduce intensamente y el **piloto se enciende**.
   - Cuando el `GPIO = 3.3 V (HIGH)`: El 2N2222 se satura ($V_{CE(sat)} \approx 0.1\,\text{V}$ a $0.2\,\text{V}$). La compuerta del MOSFET cae a $\approx 0.2\,\text{V} < V_{GS(th)}$. El MOSFET se apaga y el **piloto se apaga**.
   - *Impacto:* Durante el arranque (bootloader) del ESP32, los GPIOs flotan en alta impedancia o entrada con pull-up débil; con esta topología, **los pilotos se encenderían brevemente al arrancar**, lo que representa un comportamiento indeseado.
2. **Cálculo de Componentes:**
   - Resistencia de base $R_{base}$:
     $$I_C \approx \frac{12\,\text{V}}{10\,\text{k}\Omega} = 1.2\,\text{mA}$$
     Para asegurar saturación con $\beta_{sat} = 10$:
     $$I_B \ge \frac{1.2\,\text{mA}}{10} = 0.12\,\text{mA}$$
     $$R_{base} = \frac{V_{GPIO} - V_{BE(sat)}}{I_B} = \frac{3.3\,\text{V} - 0.7\,\text{V}}{0.5\,\text{mA}} = \frac{2.6\,\text{V}}{0.5\,\text{mA}} \approx 4.7\,\text{k}\Omega \quad (\text{o } 1\,\text{k}\Omega \text{ a } 3.3\,\text{k}\Omega)$$
3. **Consumo Estático:**
   Cuando el piloto está apagado ($GPIO = HIGH$), el 2N2222 disipa potencia a través de $R_{pullup}$:
   $$P_{pullup} = \frac{(12\,\text{V})^2}{10\,\text{k}\Omega} = 14.4\,\text{mW}$$
   Es un consumo despreciable, pero añade complejidad innecesaria (2 semiconductores por cada piloto, 6 semiconductores por estación).

---

### Topología B (Recomendada): MOSFET de Nivel Lógico (Logic-Level N-Channel) Directo

Al utilizar un MOSFET con compuerta de nivel lógico (*Logic-Level*), se elimina completamente el 2N2222.  
Ejemplos de componentes verificados compatibles con nivel lógico de 3.3 V:
- **AO3400A** (SMD SOT-23): $V_{GS(th)} \approx 0.65\,\text{V} - 1.45\,\text{V}$; $R_{DS(on)} < 33\,\text{m}\Omega$ a $V_{GS} = 2.5\,\text{V}$.
- **IRLZ44N** (Through-hole TO-220): $V_{GS(th)} \approx 1.0\,\text{V} - 2.0\,\text{V}$; plenamente saturado a $V_{GS} = 3.3\,\text{V}$ para corrientes de hasta varios amperios.
- **2N7000 / BS170** (Para pilotos LED de baja corriente $< 200\,\text{mA}$): $V_{GS(th)} \approx 0.8\,\text{V} - 2.1\,\text{V}$.

#### Circuito Esquemático de la Topología B:
```
                      +12V DC
                         |
                    [PILOTO 12V]
                         |
                         +---------------- DRENADOR (D)
                         |
  GPIO ---[R_gate]-------+---- COMPUERTA (G) [MOSFET Canal N Nivel Lógico]
(ESP32)   (100 Ohm)      |
                         |                 FUENTE (S)
                    [R_pulldown]               |
                     (100 kOhm)                |
                         |                     |
                         +----------+----------+
                                    |
                               [GND COMÚN]
```

#### Parámetros de Diseño y Cálculos:
1. **Lógica Positiva Directa:**
   - $GPIO = 3.3\,\text{V} \implies$ Piloto ENCENDIDO.
   - $GPIO = 0\,\text{V} \implies$ Piloto APAGADO.
   - Durante el reset o arranque del ESP32, el resistor $R_{pulldown}$ garantiza que la compuerta permanezca a 0 V, evitando parpadeos aleatorios.
2. **Resistencia de Compuerta ($R_{gate} = 100\,\Omega$ a $220\,\Omega$):**
   - Limita el pico de corriente transitoria de carga de la capacitancia parásita de compuerta ($C_{iss} \approx 50\,\text{pF} - 1500\,\text{pF}$), protegiendo la salida del ESP32:
     $$I_{peak} = \frac{3.3\,\text{V}}{100\,\Omega} = 33\,\text{mA} \quad (\text{durante unos pocos nanosegundos})$$
3. **Resistencia Pulldown ($R_{pulldown} = 10\,\text{k}\Omega$ a $100\,\text{k}\Omega$):**
   - Mantiene la compuerta firmemente a 0 V cuando el GPIO está en alta impedancia (Hi-Z).
4. **Disipación Térmica en el MOSFET:**
   - Asumiendo un piloto de panel con bombilla incandescente o LED de 12V con $I = 150\,\text{mA} = 0.15\,\text{A}$:
   - Con IRLZ44N ($R_{DS(on)} \approx 0.035\,\Omega$ a 3.3 V):
     $$P_{dis} = I^2 \times R_{DS(on)} = (0.15\,\text{A})^2 \times 0.035\,\Omega = 0.79\,\text{mW}$$
   - ¡El transistor ni siquiera se entibia! No requiere disipador térmico alguno.

---

### Topología C (Máxima Seguridad Industrial): Aislamiento con Optoacoplador PC817

Si se busca **aislamiento galvánico total** entre el circuito de control del ESP32 y la línea de 12 V de los pilotos:

```
  +3.3V ESP32
      |
    [ANODO]
    [PC817 LED]
    [CATODO]
      |
  [R_led: 220 Ohm]
      |
  GPIO (Activo LOW)
(o GPIO directo al ánodo con R_led al GND de control)

                                  +12V DC
                                     |
                                [PILOTO 12V]
                                     |
                                  COLECTOR (PC817)
                                  EMISOR (PC817)
                                     |
                              [GND 12V POTENCIA]
```
> [!NOTE]
> El fototransistor del PC817 soporta hasta $I_C = 50\,\text{mA}$. Si el piloto consume más de 30 mA, el PC817 debe excitar a su vez la compuerta de un MOSFET de canal N o transistor de potencia.

---

## 3. Matriz Comparativa de Decisiones

| Criterio | Topología A (2N2222 + MOSFET) | Topología B (Logic-Level N-MOSFET) | Topología C (Optoacoplador PC817 + Driver) |
|---|---|---|---|
| **Número de Componentes por canal** | 2 transistores, 2 resistencias | 1 MOSFET, 2 resistencias | 1 opto, 1 transistor/MOSFET, 3 resistencias |
| **Lógica de Conmutación** | Invertida (LOW=ON, HIGH=OFF) | Directa (HIGH=ON, LOW=OFF) | Directa o Invertida configurable |
| **Comportamiento en Reset del ESP32** | Piloto tiende a encenderse | Piloto permanece apagado (seguro) | Piloto permanece apagado (seguro) |
| **Aislamiento Galvánico** | No (GND común) | No (GND común) | Sí (Aislamiento óptico > 2500 Vrms) |
| **Idoneidad para ARCEUS GRID** | Complejo y poco intuitivo | **ÓPTIMA (Recomendada)** | Excelente si hay perturbaciones severas |

---

## 4. Conclusión y Recomendación de Ingeniería

1. **Recomendación Directa:** Adoptar la **Topología B** empleando un MOSFET de canal N de nivel lógico como el **IRLZ44N** (formato TO-220 fácil de cablear en protoboard o placa perforada) o un módulo preensamblado de MOSFETs tipo **LR7843 / FR120N / AOD4184**.
2. **Si se dispone del 2N2222:** Se puede utilizar el 2N2222 en solitario si los pilotos son de tecnología LED ($I < 200\,\text{mA}$), conectando el piloto entre +12V y el colector del 2N2222, con emisor a GND, y una resistencia de base de $1\,\text{k}\Omega$ hacia el GPIO del ESP32. Esto funciona como interruptor de tierra directo (Low-Side Switch) con lógica no invertida y solo requiere 1 transistor y 1 resistencia por piloto.
