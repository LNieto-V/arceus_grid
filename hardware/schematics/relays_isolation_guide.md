# Guía Técnica: Módulo de 4 Relés y Aislamiento Óptico (JD-VCC / VCC)

## 1. Especificaciones Críticas del Módulo de 4 Relés

Los módulos comerciales estándar de 4 relés (habitualmente basados en relés electromecánicos Songle SRD-05VDC-SL-C) presentan las siguientes características operativas:

- **Tensión nominal de bobina:** **5 V DC**. No existen módulos SRD de 3.3V comunes; excitar una bobina de 5V con 3.3V genera una fuerza magnetomotriz insuficiente para garantizar el cierre seguro de los contactos bajo vibración o desgaste.
- **Corriente por bobina:** $\approx 70\,\text{mA}$ a $75\,\text{mA}$ cada una.
  - Con los 4 relés activados simultáneamente: $4 \times 72\,\text{mA} \approx 290\,\text{mA}$.
  - > [!CAUTION]
    > **NUNCA** alimentar las bobinas de los relés desde el pin de 3.3V del ESP32 ni mucho menos desde sus pines GPIO (cuyo límite absoluto es 12-20 mA). Hacerlo causará caídas de tensión instantáneas (Brownout Reset) y daño permanente al regulador interno del ESP32.
- **Lógica de Entrada (Optoacoplada):** En el 95% de los módulos comerciales, las entradas `IN1`, `IN2`, `IN3`, `IN4` son **ACTIVAS EN BAJO (Active LOW)**.

---

## 2. El Jumper de Aislamiento: JD-VCC vs VCC

El módulo cuenta con un conector de 3 pines con un puente (jumper) extraíble rotulado como `[VCC - JD-VCC - GND]`:

```
          [Conector con Jumper]
             +---+---+---+
             |VCC|JD |GND|
             +---+---+---+
                   ^
                   | Jumper puesto une VCC con JD-VCC
```

- **JD-VCC:** Alimenta directamente las **bobinas electromecánicas** de los relés a través del transistor driver.
- **VCC:** Alimenta los **LEDs internos de los optoacopladores** de señal.

### Configuración A: Sin aislamiento galvánico (Jumper Puesto VCC - JD-VCC)
- Alimenta bobinas y lógica desde la misma fuente de 5V.
- Si se conecta VCC a los 5V del convertidor LM2596 y la señal a los pines del ESP32 (3.3V), el LED del optoacoplador queda sometido a una diferencia de potencial de:
  $$V_{LED} = 5.0\,\text{V} - 3.3\,\text{V} = 1.7\,\text{V}$$
  Dado que el umbral del diodo infrarrojo del optoacoplador suele ser de $1.2\,\text{V}$ a $1.4\,\text{V}$, **el relé podría permanecer encendido o en un estado de conducción parcial no deseado incluso cuando el ESP32 pone el pin en HIGH**.

### Configuración B (Recomendada para ARCEUS GRID: Aislamiento Óptico Total)
Para garantizar compatibilidad perfecta con la lógica de 3.3V del ESP32 y eliminar el ruido inductivo de conmutación (Fuerza Contraelectromotriz):

1. **RETIRAR el jumper plástico entre VCC y JD-VCC**.
2. Conectar **JD-VCC** a la salida de **5.00 V** del convertidor LM2596 (o a una línea dedicada de 5V).
3. Conectar el pin **GND del módulo** al GND de la fuente de 5V/12V.
4. Conectar el pin **VCC** (del peine de pines IN1..IN4) al pin de **3.3 V** del ESP32.
5. Conectar `IN1` a GPIO25 y `IN2` a GPIO26 de la Estación A (e `IN3`, `IN4` para la Estación B).
6. **NO CONECTAR** el GND del peine de señales al ESP32 (el retorno del optoacoplador se efectúa a través del pin GPIO cuando se pone en LOW).

```
   ESP32 (3.3V)                                     MÓDULO DE 4 RELÉS
  +------------+                                   +-------------------+
  |       3.3V |---------------------------------->| VCC (Optoacopl.)  |
  |     GPIO25 |---------------------------------->| IN1 (Bombilla 1)  |
  |     GPIO26 |---------------------------------->| IN2 (Bombilla 2)  |
  +------------+                                   |                   |
                                                   | JD-VCC (Bobinas)  | <--- Salida +5.0V (LM2596)
                                                   | GND (Módulo)      | <--- Tierra Potencia (GND)
                                                   +-------------------+
```

---

## 3. Conexión de los Contactos de Potencia a las Bombillas de 12V

Cada relé cuenta con 3 terminales en su bornera de tornillo:
- **COM (Común):** Conectado aguas abajo del sensor INA219 (polo positivo protegido +12 V).
- **NO (Normalmente Abierto - Normally Open):** Conectado al polo positivo (+) de la bombilla de 12 V.
- **NC (Normalmente Cerrado - Normally Closed):** Dejar sin conexión (aislado).

El polo negativo (-) de cada bombilla retorna directamente a la barra de tierra en estrella (Star GND) de 12 V.

```
       [Línea Positiva desde INA219 (VIN-)]
                       |
                       +-------------------+
                       |                   |
                  [COM Relé 1]        [COM Relé 2]
                  [NO  Relé 1]        [NO  Relé 2]
                       |                   |
                  (+) Bombilla 1      (+) Bombilla 2
                  (-) Bombilla 1      (-) Bombilla 2
                       |                   |
                       +---------+---------+
                                 |
                          [GND Potencia 12V]
```

### Ventaja de Seguridad:
Al estar el contacto en configuración Normally Open (NO), en caso de pérdida de energía o reinicio del microcontrolador, las bombillas permanecen automáticamente en **estado seguro apagado**.
