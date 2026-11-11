# Diagrama de Cableado Pin-a-Pin — Estaciones A y B

## 1. Tabla de Asignación de Pines — Estación A

| Módulo / Periférico | Pin del Módulo | Pin ESP32 (Estación A) | Función Eléctrica | Notas de Conexión |
|---|---|---|---|---|
| **Alimentación ESP32** | OUT+ (LM2596-A) | **5V (VIN)** | Alimentación principal de placa | Calibrado obligatoriamente a 5.00 V DC |
| **Alimentación ESP32** | OUT- (LM2596-A) | **GND** | Referencia lógica de placa | Conectado al punto central de tierra |
| **Pantalla OLED SSD1306** | VCC | **3.3V** | Alimentación lógica pantalla | Corriente típica: 15-20 mA |
| **Pantalla OLED SSD1306** | GND | **GND** | Tierra lógica pantalla | Compartida con bus ESP32 |
| **Pantalla OLED SSD1306** | SDA | **GPIO21** | Datos I2C | Bus compartido a 400 kHz |
| **Pantalla OLED SSD1306** | SCL | **GPIO22** | Reloj I2C | Bus compartido a 400 kHz |
| **Sensor INA219** | VCC | **3.3V** | Alimentación lógica sensor | Mantiene bus I2C a niveles de 3.3V |
| **Sensor INA219** | GND | **GND** | Tierra lógica sensor | Compartida con bus ESP32 |
| **Sensor INA219** | SDA | **GPIO21** | Datos I2C (Dir: 0x40) | Bus compartido |
| **Sensor INA219** | SCL | **GPIO22** | Reloj I2C (Dir: 0x40) | Bus compartido |
| **Sensor INA219 (Shunt)** | VIN+ | **+12V (Fuente)** | Entrada de corriente High-Side | Tras fusible individual de Estación A |
| **Sensor INA219 (Shunt)** | VIN- | **COM Relé 1 y Relé 2** | Salida medida hacia cargas | Línea de distribución positiva de bombillas |
| **Módulo Relés (IN1)** | IN1 | **GPIO25** | Control Bombilla 1 | Activo en nivel LOW (optoacoplado) |
| **Módulo Relés (IN2)** | IN2 | **GPIO26** | Control Bombilla 2 | Activo en nivel LOW (optoacoplado) |
| **Módulo Relés** | VCC | **3.3V** | Alimentación ánodos optoacoplador | Con jumper JD-VCC removido |
| **Piloto Verde (12V)** | Compuerta Driver | **GPIO27** | Indicador de Estado NORMAL | Conexión vía MOSFET de nivel lógico |
| **Piloto Rojo (12V)** | Compuerta Driver | **GPIO32** | Indicador de ALARMA / DANGER | Conexión vía MOSFET de nivel lógico |
| **Piloto Amarillo (12V)** | Compuerta Driver | **GPIO33** | Indicador de ADVERTENCIA | Conexión vía MOSFET de nivel lógico |

---

## 2. Tabla de Asignación de Pines — Estación B

| Módulo / Periférico | Pin del Módulo | Pin ESP32 (Estación B) | Función Eléctrica | Notas de Conexión |
|---|---|---|---|---|
| **Alimentación ESP32** | OUT+ (LM2596-B) | **5V (VIN)** | Alimentación principal de placa | Calibrado obligatoriamente a 5.00 V DC |
| **Alimentación ESP32** | OUT- (LM2596-B) | **GND** | Referencia lógica de placa | Conectado al punto central de tierra |
| **Pantalla OLED SSD1306** | VCC | **3.3V** | Alimentación lógica pantalla | Corriente típica: 15-20 mA |
| **Pantalla OLED SSD1306** | GND | **GND** | Tierra lógica pantalla | Compartida con bus ESP32 |
| **Pantalla OLED SSD1306** | SDA | **GPIO21** | Datos I2C | Bus compartido a 400 kHz |
| **Pantalla OLED SSD1306** | SCL | **GPIO22** | Reloj I2C | Bus compartido a 400 kHz |
| **Sensor INA219** | VCC | **3.3V** | Alimentación lógica sensor | Mantiene bus I2C a niveles de 3.3V |
| **Sensor INA219** | GND | **GND** | Tierra lógica sensor | Compartida con bus ESP32 |
| **Sensor INA219** | SDA | **GPIO21** | Datos I2C (Dir: 0x40) | Bus compartido |
| **Sensor INA219** | SCL | **GPIO22** | Reloj I2C (Dir: 0x40) | Bus compartido |
| **Sensor INA219 (Shunt)** | VIN+ | **+12V (Fuente)** | Entrada de corriente High-Side | Tras fusible individual de Estación B |
| **Sensor INA219 (Shunt)** | VIN- | **COM Relé 3 y Relé 4** | Salida medida hacia cargas | Línea de distribución positiva de bombillas |
| **Módulo Relés (IN3)** | IN3 | **GPIO25** | Control Bombilla 1 (Est. B) | Activo en nivel LOW (optoacoplado) |
| **Módulo Relés (IN4)** | IN4 | **GPIO26** | Control Bombilla 2 (Est. B) | Activo en nivel LOW (optoacoplado) |
| **Módulo Relés** | VCC | **3.3V** | Alimentación ánodos optoacoplador | Con jumper JD-VCC removido |
| **Piloto Verde (12V)** | Compuerta Driver | **GPIO27** | Indicador de Estado NORMAL | Conexión vía MOSFET de nivel lógico |
| **Piloto Rojo (12V)** | Compuerta Driver | **GPIO32** | Indicador de ALARMA / DANGER | Conexión vía MOSFET de nivel lógico |
| **Piloto Amarillo (12V)** | Compuerta Driver | **GPIO33** | Indicador de ADVERTENCIA | Conexión vía MOSFET de nivel lógico |

---

## 3. Consideraciones Especiales del Bus I2C Compartido

Tanto la pantalla OLED (0x3C) como el sensor INA219 (0x40) disponen de resistencias de pull-up incorporadas en sus respectivos módulos (típicamente $4.7\,\text{k}\Omega$ o $10\,\text{k}\Omega$ conectadas a sus respectivos pines VCC).  
Al estar ambos módulos alimentados a **3.3 V**, las resistencias de pull-up quedan en paralelo:
$$R_{pullup\_eq} = \frac{R_1 \times R_2}{R_1 + R_2} \approx \frac{10\,\text{k}\Omega \times 10\,\text{k}\Omega}{20\,\text{k}\Omega} = 5\,\text{k}\Omega$$
Este valor es óptimo para operar el bus I2C a 400 kHz con tiempos de subida ($t_r < 300\,\text{ns}$) totalmente conformes a la especificación estándar I2C.  
**No es necesario añadir resistencias de pull-up externas adicionales.**
