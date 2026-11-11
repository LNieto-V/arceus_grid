# Arquitectura Eléctrica y Distribución de Potencia — ARCEUS GRID

## 1. Topología General de Alimentación

El sistema experimental **ARCEUS GRID** se alimenta de una única fuente conmutada industrial de **12 V DC**.  
Para preservar la integridad del microcontrolador ESP32 y de la instrumentación sensible, se establece una **separación funcional estricta entre la etapa de potencia (12 V) y la etapa de control/lógica (5 V / 3.3 V)**.

```
                  +----------------------------------------------+
                  |           FUENTE CONMUTADA 12V DC            |
                  |          (Recomendada: >= 12V @ 5A)          |
                  +----------------------+-----------------------+
                                         |
                     +-------------------+-------------------+
                     | FUSIBLE GENERAL F1 (5A Fusión Rápida) |
                     +-------------------+-------------------+
                                         |
            +----------------------------+----------------------------+
            | LÍNEA 12V POSITIVA                                      |
            |                                                         |
     [DISTRIBUCIÓN ESTACIÓN A]                                 [DISTRIBUCIÓN ESTACIÓN B]
            |                                                         |
    +-------+-------+                                         +-------+-------+
    |               |                                         |               |
[LM2596-A]      [INA219-A (VIN+)]                         [LM2596-B]      [INA219-B (VIN+)]
 (IN: 12V)       (Medición High-Side)                      (IN: 12V)       (Medición High-Side)
 (OUT: 5.00V)            |                                 (OUT: 5.00V)            |
    |             [INA219-A (VIN-)]                           |             [INA219-B (VIN-)]
    v                    |                                    v                    |
ESP32-A (5V/VIN)  [RELÉ 1]   [RELÉ 2]                      ESP32-B (5V/VIN)  [RELÉ 3]   [RELÉ 4]
                     |          |                                             |          |
                  [BOMB 1]   [BOMB 2]                                      [BOMB 3]   [BOMB 4]
                     |          |                                             |          |
            +--------+----------+                                    +--------+----------+
            | RETORNO 12V POTENCIA                                   | RETORNO 12V POTENCIA
            v                                                        v
      [ESTRELLA GND-A]                                         [ESTRELLA GND-B]
            |                                                        |
            +----------------------------+---------------------------+
                                         |
                                  [GND CENTRAL] (Borne Negativo de la Fuente 12V)
```

---

## 2. Convertidores Step-Down LM2596 (Alimentación ESP32)

Cada estación dispone de su propio módulo convertidor buck LM2596.

### Procedimiento Obligatorio de Calibración Previa:
> [!CAUTION]
> **NUNCA** conectar el ESP32 al convertidor LM2596 sin haber calibrado y verificado primero con multímetro. El potenciómetro multivuelta de fábrica suele venir ajustado cerca de la tensión de entrada (12 V), lo que causaría la destrucción inmediata del ESP32 si se conecta a su regulador interno.

1. Conectar la entrada del LM2596 (`IN+`, `IN-`) a los 12 V de la fuente principal con el ESP32 **completamente desconectado**.
2. Colocar las puntas del multímetro digital en modo voltímetro DC sobre las bornas de salida (`OUT+`, `OUT-`).
3. Girar el tornillo del potenciómetro multivuelta (típicamente en sentido antihorario) hasta obtener una lectura estable de **5.00 V DC ± 0.05 V**.
4. Aplicar una pequeña carga de prueba (resistencia de 100 $\Omega$, 1 W) para verificar la regulación bajo carga.
5. Solo tras confirmar 5.00 V estables, conectar la salida al pin **5V (o VIN)** del ESP32 y a **GND**.

---

## 3. Topología de Medición High-Side con INA219

El INA219 se ubica en configuración **High-Side** sobre la línea de +12 V antes de derivar hacia los relés de cada estación:

- **Bornes del Shunt (`VIN+` y `VIN-`):**
  - `VIN+`: Conectado a la línea principal de +12 V protegida por el fusible de estación.
  - `VIN-`: Conectado a los bornes Comunes (COM) de los dos relés de la estación correspondiente.
- **Bornes lógicos del INA219:**
  - `VCC`: Conectado a los **3.3 V** del ESP32 (el INA219 admite 3.0 V a 5.5 V, pero al usar 3.3 V se garantiza que los niveles lógicos del bus I2C coincidan exactamente con el microcontrolador).
  - `GND`: Conectado al GND del ESP32.
  - `SDA`: Conectado a GPIO21.
  - `SCL`: Conectado a GPIO22.
- **Tensión de Modo Común:** El INA219 tolera hasta 26 V de modo común en el bus, lo que lo hace idóneo para 12 V DC.
- **Resistencia Shunt:** El módulo estándar incluye una resistencia $R_{shunt} = 0.1\,\Omega$ ($100\,\text{m}\Omega$, 1% o 0.5%).
  - Corriente máxima con PGA $\div 8$ ($\pm 320\,\text{mV}$):
    $$I_{max} = \frac{V_{shunt\_max}}{R_{shunt}} = \frac{0.320\,\text{V}}{0.1\,\Omega} = 3.20\,\text{A}$$
  - Caída de tensión típica a 1 A de carga:
    $$V_{drop} = 1.0\,\text{A} \times 0.1\,\Omega = 0.10\,\text{V}$$
    Por tanto, a las bombillas les llega $12.0\,\text{V} - 0.1\,\text{V} = 11.9\,\text{V}$, una pérdida insignificante (< 1%).
  - Potencia disipada en el Shunt a 2 A continuos:
    $$P_{shunt} = I^2 \times R = (2.0\,\text{A})^2 \times 0.1\,\Omega = 0.40\,\text{W}$$
    El resistor SMD 2512 del módulo está dimensionado para 1 W o 2 W, por lo que opera con amplio margen térmico.

---

## 4. Topología de Tierra en Estrella (Star Ground) y Prevención de Bucles

Uno de los problemas más críticos en sistemas de instrumentación mixta (señal débil I2C de 3.3V junto con cargas conmutadas de 12V y relés) es el **retorno de corriente por líneas de señal**.

### Reglas de Enrutamiento de Tierras:
1. **Punto Central de Masa (Star GND):** Ubicado físicamente en el borne negativo de la fuente de 12 V.
2. **Línea de Potencia GND de Cargas:** El retorno negativo de las 4 bombillas y los pilotos va por conductores dedicados (calibre AWG 18 o 16) directo al borne negativo de la fuente.
3. **Línea de Control GND del ESP32:** El `IN-` del convertidor LM2596 se conecta a la fuente, y el `OUT-` al pin GND del ESP32.
4. **Prohibición de Puentes Cruzados:** Nunca utilizar las patillas GND del ESP32 para cerrar el circuito negativo de las bombillas o pilotos de 12 V. Las corrientes de retorno de potencia provocarían caídas $I \times R$ en las pistas del ESP32, reseteos aleatorios (brownout) y ruido espurio en las lecturas I2C del INA219.
