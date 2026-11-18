---
marp: true
theme: uncover
paginate: true
header: "ARCEUS GRID — AIoT & Federated Learning"
footer: "Owen Mora | Proyecto de Ingeniería"
style: |
  section {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background-color: #0d1117;
    color: #c9d1d9;
    text-align: left;
    padding: 40px;
  }
  h1, h2 {
    color: #58a6ff;
  }
  h3 {
    color: #79c0ff;
  }
  strong {
    color: #f0883e;
  }
  code {
    background-color: #161b22;
    color: #7ee787;
  }
  table {
    font-size: 0.7em;
    color: #c9d1d9;
  }
  th {
    background-color: #21262d;
    color: #58a6ff;
  }
---

<!-- _class: lead -->
# ⚡ ARCEUS GRID
### Sistema Experimental de Monitoreo Eléctrico, AIoT y Aprendizaje Federado

**Autor:** Owen Mora & Equipo de Desarrollo
**Tecnologías:** ESP32 · INA219 · ESP-NOW · MQTT · Docker · FedAvg

---

## 🛑 1. El Problema de Ingeniería

* **Ceguera Operativa:** Protecciones convencionales pasivas (fusibles) sin medición en tiempo real por ramal.
* **Colapso de Red:** Enviar muestras de alta frecuencia a la nube satura enlaces Wi-Fi locales.
* **Riesgo de Privacidad:** La telemetría eléctrica detallada revela hábitos confidenciales.
* **Inestabilidad de Hardware:** Ruido de conmutación de relés y bucles de masa (*ground loops*).

---

## 💡 2. La Solución ARCEUS GRID

Una plataforma ciberfísica modular en 3 niveles:

1. **Borde (Edge):** 2 Estaciones sensoras con **ESP32 + INA219** y control por relés optoacoplados.
2. **Red de Radio Rápida (ESP-NOW):** Enlace inalámbrico de capa 2 (<5 ms) hacia un **ESP32 Gateway**.
3. **Coordinación y AIoT (Docker):** Mosquitto MQTT, base de datos SQLite WAL, Dashboard Streamlit y **FedAvg**.

---

## 🛠️ 3. Arquitectura Física del Sistema

| Módulo | Componentes | Rol Principal |
|---|---|---|
| **Estación A** | ESP32, INA219, OLED, 2x Cargas 12V, 3x Pilotos | Adquisición, visualización y corte local |
| **Estación B** | ESP32, INA219, OLED, 2x Cargas 12V, 3x Pilotos | Nodo sensor espejo para comparación |
| **Gateway Main** | 1x ESP32 Dual Mode (ESP-NOW + Wi-Fi) | Puente transparente de radio hacia Mosquitto |
| **Servidor Central** | Docker (Mosquitto + SQLite + Streamlit) | Ingesta, persistencia y agregación federada |

---

## 🔌 4. Ingeniería Eléctrica y Hardware Seguro

* **Aislamiento Galvánico Total (JD-VCC):**
  * Desconexión física entre el circuito digital de 3.3V y las bobinas de relé de 5V.
* **Tierra en Estrella (*Star Ground*):**
  * Masa centralizada en bornes de la fuente de 12V. Cero bucles de retorno espurios.
* **Regulación LM2596:**
  * Conversión DC-DC conmutada de alta eficiencia (12V a 5.0V filtrados).
* **Drivers de Pilotos 12V:**
  * Transistores 2N2222 para gobernar señalizadores industriales desde GPIOs.

---

## 📈 5. Instrumentación (INA219 + OLED SSD1306)

* **Sensor INA219 (High-Side):**
  * Resistencia shunt $0.1\ \Omega$, bus I2C a 400 kHz (0x40).
  * Monitoreo de Tensión (V), Corriente ($\pm 0.8\ \text{mA}$) y Potencia (W).
* **Pantalla OLED SSD1306 (128×64 px):**
  * Visualización local en tiempo real sin interferir con el bucle de sensado.
  * Muestra métricas, estado de relés y alertas de seguridad.

---

## 📡 6. Comunicaciones Híbridas: ESP-NOW + MQTT

* **¿Por qué ESP-NOW en los sensores?**
  * Sin dependencia de router ni contraseñas Wi-Fi en los nodos de campo.
  * Latencia inferior a **5 ms** y trama empaquetada de solo **24 bytes**.
  * Auto-sincronización de canal Wi-Fi con el Gateway.
* **El Gateway ESP32:**
  * Recibe la trama binaria por radio, la valida y la publica en JSON a **Mosquitto**.

---

## 🐳 7. Backend Contenerizado (Docker)

* **`arceus_mosquitto`:** Broker MQTT ultra liviano (puertos 1883/1884 y 9001).
* **`arceus_collector`:** Daemon Python con motor determinista de anomalías.
* **`arceus_dashboard`:** UI interactiva en **Streamlit** (puerto 8501).
* **SQLite en Modo WAL:**
  * Escrituras concurrentes del colector sin bloquear lecturas de la UI.

---

## 🛡️ 8. Detección de Anomalías y Protección Activa

* **Umbrales Críticos de Disparo:**
  * **Sobrecorriente:** $> 1.80\ \text{A}$ ➔ **Corte automático de relés (<15 ms)**.
  * **Subtensión:** $< 10.50\ \text{V}$ ➔ Alerta de sobrecarga en fuente.
  * **Sobretensión:** $> 13.50\ \text{V}$ ➔ Alerta de protección de bombillas.
* **Pilotos de Señalización Industrial:**
  * 🟢 **Verde:** Normal | 🟡 **Amarillo:** Warning | 🔴 **Rojo:** Emergencia.

---

## 🧠 9. Aprendizaje Federado (FedAvg)

$$\theta_{global} = \frac{n_A \theta_A + n_B \theta_B}{n_A + n_B}$$

1. El coordinador envía el modelo base a las Estaciones A y B.
2. Cada estación entrena localmente con sus lecturas de consumo privadas.
3. Se transmiten **únicamente los pesos matemáticos** ($\mathbf{W}, \mathbf{b}$).
4. El servidor promedia los pesos y redistribuye el modelo optimizado.
* **Resultado:** **100% de Privacidad** y **95% de ahorro de red**.

---

## 🖥️ 10. Demostración en Vivo (Live Demo)

* **Dashboard Web:** [http://localhost:8501](http://localhost:8501)
* **Acciones en Vivo:**
  1. Conmutación remota de Bombilla 1 y Bombilla 2.
  2. Verificación de corriente en OLED y Dashboard.
  3. Disparo por sobrecorriente simulada y corte automático.
  4. Ejecución de ronda federada en tiempo real.

---

## 🎯 11. Conclusiones y Futuro

* ✅ **Validación Completa:** Plataforma AIoT segura, desacoplada y de bajo costo.
* ✅ **Comunicaciones Eficientes:** ESP-NOW + MQTT demostró máxima robustez.
* ✅ **Inteligencia Privada:** FedAvg funcional en microrredes experimentales.
* 🚀 **Futuro:** PCB dedicada de 4 capas, TinyML (TensorFlow Lite) e integración solar.

---

<!-- _class: lead -->
# ¡Muchas Gracias!
### Preguntas y Discusión

**Repositorio:** [github.com/LNieto-V/arceus_grid](https://github.com/LNieto-V/arceus_grid)
