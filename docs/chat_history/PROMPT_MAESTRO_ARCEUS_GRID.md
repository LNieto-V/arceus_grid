# Especificación Original del Proyecto: PROMPT MAESTRO — ARCEUS GRID

"# PROMPT MAESTRO — ARCEUS GRID

## 1. Rol y especialización

Actúa como un ingeniero especializado en:

- Electrónica e instrumentación eléctrica.
- Sistemas embebidos y microcontroladores ESP32.
- IoT e IIoT.
- Automatización industrial.
- Sistemas de adquisición de datos.
- Comunicación MQTT.
- Inteligencia artificial aplicada a sistemas embebidos (AIoT).
- Aprendizaje federado (Federated Learning).
- Procesamiento de datos y detección de anomalías.
- Diseño de circuitos electrónicos y sistemas de potencia de baja tensión.

Tu función es ayudarme a diseñar, construir, programar, probar y documentar mi proyecto **ARCEUS GRID**.

Proporciona explicaciones técnicas claras, cálculos eléctricos, diagramas de conexión, código funcional y recomendaciones fundamentadas.

No inventes especificaciones de componentes ni asumas que un circuito está validado sin haberlo comprobado.

---

## 2. Descripción general del proyecto

ARCEUS GRID es un sistema experimental de monitoreo inteligente de consumo eléctrico compuesto por dos estaciones independientes y un computador coordinador.

Su objetivo es medir el consumo energético de diferentes cargas, controlar su funcionamiento, detectar anomalías y experimentar con inteligencia artificial y aprendizaje federado.

El proyecto integra:

- IoT e IIoT.
- Instrumentación eléctrica.
- ESP32.
- Sensores INA219.
- Pantallas OLED.
- Relés.
- Comunicación MQTT.
- Monitoreo eléctrico en tiempo real.
- Base de datos SQLite.
- Dashboard Streamlit.
- Detección de anomalías.
- Aprendizaje federado mediante FedAvg.
- Control remoto de cargas eléctricas.

---

## 3. Arquitectura física

### Estación A

Componentes:

- 1 ESP32.
- 1 sensor INA219.
- 1 pantalla OLED de 0.96 pulgadas.
- 2 bombillas LED de 12 V DC.
- 2 canales de relé para controlar las bombillas.
- 3 pilotos indicadores de 12 V DC:
  - Verde.
  - Rojo.
  - Amarillo.

### Estación B

Componentes:

- 1 ESP32.
- 1 sensor INA219.
- 1 pantalla OLED de 0.96 pulgadas.
- 2 bombillas LED de 12 V DC.
- 2 canales de relé para controlar las bombillas.
- 3 pilotos indicadores de 12 V DC:
  - Verde.
  - Rojo.
  - Amarillo.

### Sistema central

Componentes:

- 1 fuente conmutada de 12 V DC.
- 1 módulo de cuatro relés.
- 2 convertidores LM2596, uno para cada ESP32.
- 1 computador personal como coordinador.
- Red Wi-Fi.
- Broker MQTT Mosquitto.
- Base de datos SQLite.
- Dashboard Streamlit.
- Coordinador Python para aprendizaje federado.

---

## 4. Arquitectura eléctrica

### Alimentación

Una fuente central de 12 V DC alimentará las dos estaciones.

Cada estación tendrá una distribución independiente de alimentación.

El ESP32 de cada estación recibirá alimentación mediante un convertidor LM2596 ajustado y verificado a 5 V antes de conectarlo a la placa.

La alimentación de las cargas de 12 V debe estar separada funcionalmente de la alimentación de control.

### INA219

Cada estación tendrá un INA219 para medir el consumo eléctrico total de sus dos bombillas.

El sensor se instalará en serie sobre la línea positiva de alimentación de las cargas, antes de la distribución hacia los dos relés.

Topología conceptual:

FUENTE 12 V (+)
        |
        |
      VIN+
      INA219
      VIN-
        |
        |
   DISTRIBUCIÓN
      /     
   RELÉ 1  RELÉ 2
      |       |
   LED 1    LED 2
      |       |
     GND     GND

El INA219 debe permitir medir:

- Tensión de bus.
- Corriente total de las dos bombillas.
- Potencia eléctrica.
- Comportamiento del consumo cuando se activan o desactivan las cargas.

No asumir que el sensor mide individualmente cada bombilla: mide la corriente combinada de las cargas conectadas aguas abajo.

Verificar el rango de corriente, la caída de tensión, la potencia y la compatibilidad del módulo INA219 utilizado.

### Comunicación I2C

Asignación inicial para ambas estaciones:

- SDA: GPIO21.
- SCL: GPIO22.

La pantalla OLED y el INA219 compartirán el mismo bus I2C.

Dirección OLED habitual: 0x3C, que debe verificarse físicamente.

Dirección INA219 habitual: 0x40, que también debe comprobarse.

### Pantallas OLED

Características:

- Tamaño: 0.96 pulgadas.
- Resolución: 128 × 64.
- Comunicación: I2C.
- Controlador esperado: SSD1306.
- Alimentación compatible con 3.3 V según el módulo.

Librerías previstas:

- Adafruit SSD1306.
- Adafruit GFX Library.
- Adafruit INA219.

Las pantallas deberán mostrar información de cada estación:

- Identificación de estación.
- Estado operativo.
- Voltaje.
- Corriente.
- Potencia.
- Estado de las bombillas.
- Estado de conexión MQTT.
- Alertas y anomalías.

---

## 5. Control de relés

El módulo de cuatro relés controlará exclusivamente las cuatro bombillas:

| Relé | Función |
|---|---|
| IN1 | Bombilla 1, estación A |
| IN2 | Bombilla 2, estación A |
| IN3 | Bombilla 1, estación B |
| IN4 | Bombilla 2, estación B |

Pines sugeridos por estación:

- GPIO25.
- GPIO26.

La asignación debe verificarse con el módulo físico.

No asumir que un módulo de relés con bobinas de 3.3 V acepta automáticamente señales lógicas de 3.3 V.

Comprobar:

- Tensión de alimentación de las bobinas.
- Corriente de las bobinas.
- Compatibilidad lógica.
- Polaridad de las entradas.
- Existencia de aislamiento.
- Configuración activa en HIGH o LOW.

Nunca alimentar bobinas directamente desde los GPIO del ESP32.

---

## 6. Control de pilotos indicadores

Cada estación tendrá tres pilotos de 12 V DC:

| Piloto | Función |
|---|---|
| Verde | Funcionamiento normal |
| Amarillo | Advertencia |
| Rojo | Alarma o anomalía |

Pines sugeridos:

- GPIO27: verde.
- GPIO32: rojo.
- GPIO33: amarillo.

Se está evaluando una etapa de control con:

- Transistor NPN 2N2222.
- MOSFET de canal N.
- Resistencias de polarización.
- Alimentación de 12 V DC.

La intención es estudiar el uso del 2N2222 como etapa de control del MOSFET y el MOSFET como interruptor de potencia.

No entregar conexiones definitivas sin analizar primero la topología.

Considerar:

- MOSFET de nivel lógico compatible con 3.3 V.
- Resistencia Gate-Source.
- Resistencia de base del transistor.
- Resistencia de compuerta.
- Corriente de los pilotos.
- Disipación térmica.
- Protección contra cortocircuitos.
- Distribución de GND.
- Retornos de corriente.
- Posible aislamiento mediante optoacopladores o drivers aislados.

El 2N2222 y el MOSFET no proporcionan aislamiento galvánico por sí solos.

Nunca conectar directamente los pilotos de 12 V a los GPIO.

---

## 7. Seguridad eléctrica

El prototipo trabajará con cargas de 12 V DC.

No utilizar tensión de red directamente en la maqueta.

Antes de recomendar un circuito definitivo:

1. Verificar las referencias exactas de los componentes.
2. Comprobar polaridades.
3. Calcular corrientes.
4. Calcular potencia de la fuente.
5. Verificar corrientes de arranque.
6. Dimensionar fusibles.
7. Comprobar sección de conductores.
8. Verificar compatibilidad de relés.
9. Revisar alimentación de bobinas.
10. Separar circuitos de potencia y control.
11. Diseñar apropiadamente la distribución de GND.
12. Evitar retornos de corriente por las líneas de señal.
13. Comprobar temperaturas y disipación.

No asumir la corriente total de la fuente hasta conocer la potencia real de las bombillas y pilotos.

---

## 8. Primera etapa de pruebas: ESP32 + LEDs + OLED

En las primeras pruebas se desarrolló una secuencia de tres LEDs conectados a:

- GPIO16.
- GPIO17.
- GPIO18.

Estos LEDs se utilizaron para experimentar con estados de alerta.

Comportamiento inicial:

| Comando serial | Estado | Comportamiento |
|---|---|---|
| 0 | OFF | Todos apagados |
| 1 | NORMAL | LED verde fijo |
| 2 | WARNING | LED amarillo intermitente |
| 3 | DANGER | LED rojo intermitente rápido |

Los tiempos de referencia fueron:

- Advertencia: 700 ms.
- Peligro: 200 ms.

Se implementó una pantalla OLED SSD1306 de 128 × 64 mediante I2C.

Los comandos se recibían por el monitor serial a 115200 baudios.

Esta implementación constituye una prueba didáctica inicial.

No confundir estos tres LEDs de prueba con los pilotos indicadores definitivos de 12 V de ARCEUS GRID.

La arquitectura definitiva utiliza GPIO27, GPIO32 y GPIO33 como asignación sugerida para los pilotos, sujeta a verificación.

---

## 9. Arquitectura de software de cada estación

Cada ESP32 deberá ejecutar un firmware independiente.

Responsabilidades:

1. Inicializar sensores y periféricos.
2. Leer el INA219.
3. Obtener voltaje, corriente y potencia.
4. Actualizar la OLED.
5. Controlar los dos relés.
6. Controlar los tres pilotos.
7. Publicar mediciones mediante MQTT.
8. Recibir comandos desde el coordinador.
9. Supervisar la conexión Wi-Fi.
10. Recuperarse de desconexiones.
11. Gestionar estados de error.
12. Detectar condiciones eléctricas anormales mediante umbrales locales.

La programación debe utilizar preferiblemente una arquitectura modular, separando:

- Configuración de pines.
- Lectura de sensores.
- Control de relés.
- Control de pilotos.
- Pantalla OLED.
- Comunicación MQTT.
- Gestión de alertas.
- Diagnóstico del sistema.

Evitar bloqueos prolongados con delay() en las versiones avanzadas.

Preferir millis(), máquinas de estados y tareas no bloqueantes cuando resulte conveniente.

---

## 10. Comunicación MQTT

El computador central ejecutará Mosquitto.

Cada estación tendrá identificación propia.

Ejemplo de tópicos:

arceus/station_a/telemetry
arceus/station_a/status
arceus/station_a/commands
arceus/station_a/alerts

arceus/station_b/telemetry
arceus/station_b/status
arceus/station_b/commands
arceus/station_b/alerts

Los mensajes de telemetría podrán utilizar JSON.

Ejemplo:

{
  "station": "A",
  "voltage": 12.05,
  "current": 0.85,
  "power": 10.24,
  "relay_1": true,
  "relay_2": false,
  "status": "NORMAL"
}

Definir claramente:

- QoS.
- Retención de mensajes.
- Frecuencia de publicación.
- Identificadores de estación.
- Reconexión automática.
- Validación de mensajes.
- Confirmación de comandos cuando corresponda.

Los comandos remotos deberán validarse antes de ejecutarse.

Incorporar estados seguros ante pérdida de comunicación.

---

## 11. Sistema central

El computador funcionará como coordinador.

Tecnologías:

- Python.
- Mosquitto.
- SQLite.
- Streamlit.
- FedAvg.

Responsabilidades:

- Recibir telemetría de ambas estaciones.
- Almacenar mediciones.
- Consultar históricos.
- Visualizar variables en tiempo real.
- Controlar cargas remotamente.
- Detectar anomalías.
- Ejecutar entrenamiento federado.
- Gestionar modelos.
- Supervisar el estado de ambas estaciones.

### SQLite

La base de datos almacenará:

- Identificador de estación.
- Fecha y hora.
- Voltaje.
- Corriente.
- Potencia.
- Estado de relés.
- Alertas.
- Variables derivadas.
- Resultados de modelos.

### Streamlit

El dashboard mostrará:

- Estado general de ARCEUS GRID.
- Estación A.
- Estación B.
- Voltaje.
- Corriente.
- Potencia.
- Consumo histórico.
- Comparación entre estaciones.
- Estado de las bombillas.
- Alertas.
- Anomalías detectadas.
- Estado del aprendizaje federado.

---

## 12. Inteligencia artificial y aprendizaje federado

El proyecto experimentará con detección de anomalías y aprendizaje federado.

El coordinador Python utilizará FedAvg para agregar las contribuciones de ambas estaciones.

Arquitectura conceptual:

ESTACIÓN A                 ESTACIÓN B
    |                          |
 Datos locales              Datos locales
    |                          |
 Modelo local               Modelo local
    |                          |
    +------------+-------------+
                 |
                 v
          CO" genera un proyecot en la carpeta de proyecto con los docker y los enlaces para implemnar todo

<ADDITIONAL_METADATA>
The current local time is: 2026-11-11T13:14:47-05:00.
</ADDITIONAL_METADATA>
<USER_SETTINGS_CHANGE>
The user changed setting `Model Selection` from None to Gemini 3.8 Flash (High). No need to comment on this change if the user doesn't ask about it. If reporting what model you are, please use a human readable name instead of the exact string.
</USER_SETTINGS_CHANGE>
