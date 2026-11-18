# 🚀 ARCEUS GRID — Guía Paso a Paso para la Demostración en Vivo (Live Demo)

Esta guía detalla el procedimiento exacto que debe seguir el equipo expositor durante la presentación ante jurados o evaluadores para demostrar el funcionamiento del sistema en vivo.

---

## 📋 Lista de Verificación Previa (Checklist antes de exponer)

- [ ] Fuente de 12V conectada y reguladores LM2596 verificados a **5.0V DC** con multímetro.
- [ ] ESP32 Gateway conectado por USB o alimentado.
- [ ] Docker Desktop abierto y en ejecución en la máquina anfitriona.
- [ ] Red Wi-Fi activa (mismo SSID y contraseña configurados en el Gateway).
- [ ] Pantallas OLED de las Estaciones A y B encendidas mostrando métricas.
- [ ] Pilotos verdes encendidos (modo normal).

---

## 🎬 Paso 1: Iniciar el Ecosistema Docker en Terminal

Abre una terminal PowerShell y levanta todos los microservicios:

```powershell
cd C:\Users\owenm\proyectos\arceus_grid
docker compose up --build -d
```

Verifica que los tres contenedores estén en estado `Up`:

```powershell
docker compose ps
```

*Salida esperada:*
* `arceus_mosquitto` (Up - puertos 1883, 1884, 9001)
* `arceus_collector` (Up)
* `arceus_dashboard` (Up - puerto 8501)

---

## 🎬 Paso 2: Abrir el Dashboard de Operaciones

Abre en el navegador:
👉 **[http://localhost:8501](http://localhost:8501)**

**Qué señalar a los evaluadores:**
1. **Encabezado en Vivo:** Indicador de conexión MQTT con punto verde en estado `ONLINE`.
2. **Tarjetas de Métricas:** Valores en tiempo real de Tensión (V), Corriente (A), Potencia (W) y Estado de Relés para ambas estaciones.
3. **Gráficos Dinámicos:** Gráficos Plotly de series temporales que se actualizan automáticamente cada segundo.

---

## 🎬 Paso 3: Demostración de Control Bidireccional de Relés

1. En el Dashboard, localiza la sección de **Control de Cargas - Estación A**.
2. Haz clic en el interruptor de **Bombilla 1 (ON)**.
3. **Muestra el efecto físico inmediato:**
   * La bombilla física de 12V se enciende.
   * La pantalla OLED de la Estación A actualiza el estado a `R1: ON`.
   * El gráfico de corriente en el Dashboard salta de ~0.05A a ~0.45A.
4. Enciende la **Bombilla 2 (ON)** y muestra la suma acumulada de corriente en el sensor INA219.
5. Apaga las bombillas para mostrar la respuesta inmediata.

---

## 🎬 Paso 4: Demostración con el Simulador (Pruebas de Estrés / Respaldo)

Si por alguna razón física no tienes las bombillas conectadas en el auditorio, puedes ejecutar el simulador matemático integrado en una segunda terminal:

```powershell
python scripts\simulate_stations.py
```

*El simulador genera telemetría senoidal y responde a los clics del Dashboard en tiempo real.*

---

## 🎬 Paso 5: Prueba de Disparo de Seguridad por Anomalía (Protección Activa)

1. Explica al jurado: *"Vamos a simular una condición de sobrecorriente o anomalía crítica"*.
2. Si usas el simulador, puedes presionar `s` o activar sobrecarga para forzar la corriente por encima de **1.80 A**.
3. **Observa el comportamiento automático:**
   * El recolector detecta la sobrecorriente.
   * El ESP32 conmuta inmediatamente los relés a estado `OFF` (corte automático).
   * El piloto industrial cambia a **Rojo Fijo**.
   * La pantalla OLED física muestra en texto invertido la alerta: `¡ALERTA SOBRECORRIENTE!`.
   * En el Dashboard aparece una tarjeta de alarma roja parpadeante en la pestaña **Registro de Alarmas**.

---

## 🎬 Paso 6: Demostración del Algoritmo de Aprendizaje Federado (FedAvg)

1. En el Dashboard, navega a la pestaña **Inteligencia Artificial / Aprendizaje Federado**.
2. Haz clic en el botón **Ejecutar Ronda Federada (FedAvg)**.
3. **Explicación oral al jurado mientras corre el algoritmo:**
   > *"En este momento, el coordinador solicita a la Estación A y a la Estación B que ajusten sus modelos locales con sus últimas lecturas. Las estaciones calculan los gradientes en el borde y transmiten únicamente los vectores de pesos. El coordinador calcula el promedio ponderado FedAvg y despliega el modelo unificado."*
4. Muestra la gráfica de convergencia del error cuadrático medio (MSE) reduciéndose ronda tras ronda.

---

## 🎬 Paso 7: Cierre de la Demostración

Detén los contenedores cuando finalice la sesión de preguntas:

```powershell
docker compose down
```
