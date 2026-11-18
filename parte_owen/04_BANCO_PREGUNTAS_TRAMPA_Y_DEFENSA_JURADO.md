# 🛡️ MÓDULO 4: SIMULADOR DE JURADO (PREGUNTAS TRAMPA)
## 10 Preguntas Difíciles con Respuestas de Nivel Posgrado

---

### ❓ Pregunta 1: "¿Por qué en vez de FedAvg no enviaron todos los datos a la nube para entrenar un modelo más robusto?"
> **Respuesta:** *"Por tres restricciones físicas de planta: latencia, ancho de banda y propiedad intelectual. Muestrear aceleración a 200 Hz genera más de 100 MB diarios por máquina, lo que en una planta con 50 motores saturaría el enlace industrial. Además, la nube introduce latencias de 80 a 300 ms, incompatibles con la respuesta inmediata ante fallas. Con FedAvg los datos crudos no salen del borde y solo transmitimos 164 bytes de pesos una vez por ronda, reduciendo el tráfico de red en más del 95%."*

---

### ❓ Pregunta 2: "¿Cómo garantiza que el microcontrolador no se cuelgue si ocurre una anomalía grave?"
> **Respuesta:** *"Mediante dos principios de diseño:  
> 1. Programación no bloqueante con `millis()` y máquinas de estado, descartando por completo el uso de `delay()`.  
> 2. Separación de capas: la protección por sobrecorriente ($I > 2.8\,\text{A}$) es un guardrail determinista en silicio que evalúa el sensor en cada ciclo y corta los relés en menos de 1 ms, sin depender de la red ni de la IA."*

---

### ❓ Pregunta 3: "¿Qué significa Non-IID y cómo lo maneja el algoritmo?"
> **Respuesta:** *"Significa que los datos entre estaciones No son Independientes ni Idénticamente Distribuidos. En planta, una máquina puede operar al 40% de carga y otra al 90%. FedAvg lo compensa ejecutando $E = 5$ épocas de SGD local antes de sincronizar y calculando un promedio ponderado por el número de muestras ($n_k/n_{total}$), convergiendo hacia las leyes físicas comunes de la carga sin desestabilizarse por el sesgo particular de una máquina."*

---

### ❓ Pregunta 4: "¿Por qué retiraron el jumper JD-VCC del módulo de relés?"
> **Respuesta:** *"Para lograr aislamiento galvánico real. Si se deja el jumper, las bobinas de 5V y los LEDs del optoacoplador comparten línea. Al gobernar con el ESP32 (3.3V), se produce una caída inversa de $5.0\,\text{V} - 3.3\,\text{V} = 1.7\,\text{V}$, que supera el umbral de conducción del optoacoplador ($1.2\,\text{V}-1.4\,\text{V}$), impidiendo que el relé se apague y transmitiendo ruido inductivo (Back-EMF) a la lógica del microcontrolador."*

---

### ❓ Pregunta 5: "¿Qué problema resuelve la topología de Tierra en Estrella (Star Ground)?"
> **Respuesta:** *"Elimina los bucles de masa y el acoplamiento resistivo. Cuando los motores o bombillas conmutan corriente, la resistencia del cableado genera una caída $V = I \times R$. Si esa línea se comparte con la masa del ESP32, el nivel de 0V se desplaza, causando reseteos por Brownout Reset y lecturas erráticas en el bus I2C (INA219 y OLED). La estrella garantiza que cada circuito retorne por separado al borne negativo de la fuente."*

---

### ❓ Pregunta 6: "¿Por qué eligieron SQLite y no MongoDB o PostgreSQL?"
> **Respuesta:** *"Porque en Edge Computing buscamos una base de datos embebida serverless con huella de RAM mínima (< 10 MB) y cero puertos TCP expuestos. Además, configuramos el modo WAL (Write-Ahead Logging), lo que permite que el daemon MQTT inserte telemetría continua sin ser bloqueado por las consultas analíticas del panel de Streamlit o del entrenamiento federado."*

---

### ❓ Pregunta 7: "¿Por qué implementaron ESP-NOW en lugar de conectar todos los ESP32 directo al Wi-Fi?"
> **Respuesta:** *"Por seguridad, velocidad y robustez. ESP-NOW utiliza tramas de acción rápida a nivel MAC (capa de enlace) de apenas 25 bytes binarios sin sobrecarga de cabeceras TCP/IP. Los nodos sensores no tienen dirección IP, lo que los hace inmunes a ataques por red. El único nodo con IP es el Gateway central, que hace de puente seguro hacia MQTT."*

---

### ❓ Pregunta 8: "¿Qué función cumple el sensor INA219 y por qué se colocó en configuración High-Side?"
> **Respuesta:** *"Mide tensión de bus, corriente continua y potencia mediante una resistencia shunt de $0.1\,\Omega$. Se ubica en High-Side (aguas arriba de las cargas en la línea positiva) porque permite detectar fugas a tierra en cualquier punto del cableado o de los relés, algo que una medición en Low-Side no puede registrar si la fuga ocurre antes del shunt."*

---

### ❓ Pregunta 9: "¿Cómo verificaron la coherencia física entre potencia y mediciones?"
> **Respuesta:** *"Calculamos en tiempo real la discrepancia $\Delta P = |P_{medida} - (V \times I)|$. Si excede $1.2\,\text{W}$, se genera una alerta inmediata de descalibración del sensor o de armónicos reactivos anómalos en el circuito."*

---

### ❓ Pregunta 10: "¿Qué normativas IPC aplicaron en el diseño de hardware?"
> **Respuesta:** *"Aplicamos la norma IPC-2221B para dimensionar el ancho de pista según la corriente máxima de 3A y una elevación térmica admisible $\Delta T \le 10^\circ\text{C}$ (resultando en pistas de ~1.8 a 2.0 mm en cobre de 1 oz), y para mantener distancias dieléctricas de aislamiento (> 1.5 mm) frente a transitorios de conmutación inductiva."*
