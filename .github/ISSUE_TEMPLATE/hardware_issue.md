---
name: Incidencia Eléctrica o de Hardware
about: Reportar anomalías en instrumentación, INA219, relés, controladores MOSFET o caídas de tensión
title: '[HARDWARE] '
labels: 'hardware, electronics'
assignees: ''
---

**Componente / Etapa Eléctrica Afectada**
- [ ] Fuente de Alimentación 12V DC
- [ ] Convertidor Step-Down LM2596 (Regulación 5V)
- [ ] Sensor de Corriente/Tensión INA219 (I2C)
- [ ] Módulo de 4 Relés y Aislamiento JD-VCC
- [ ] Driver de Pilotos 12V (MOSFET / BJT)
- [ ] Pantalla OLED SSD1306
- [ ] Tierra en Estrella (Star GND) / Bucles de Retorno

**Lecturas Medidas con Multímetro u Osciloscopio**
- Tensión en entrada de fuente: [e.g. 12.05 V]
- Salida LM2596: [e.g. 5.01 V]
- Caída de tensión en Shunt INA219: [e.g. 45 mV]
- Corriente total medida en pinza: [e.g. 0.92 A]

**Descripción del Comportamiento Anómalo**
Descripción clara del problema observado (e.g. reseteo del ESP32 al conmutar relé, lecturas erráticas en I2C, calentamiento del MOSFET).
