# GUIÓN OWEN MORA — PUNTOS 1, 2, 7 y 8
## ARCEUS GRID | Monitoreo Eléctrico 12V DC, ESP-NOW y Aprendizaje Federado
### Universidad Cooperativa de Colombia

---

> **ROL DE OWEN:** Primer expositor — abre la presentación, presenta la agenda y luego vuelve
> para los puntos 7 y 8 (Backend Docker + Detección de Anomalías Z-Score).
> **Tiempo estimado total:** ~6 minutos (1.5 min + 1 min + 1.5 min + 2 min)

---

## ══════════════════════════════════════════
## PUNTO 1 — APERTURA + ENGANCHE
### Diapositiva 1 | ~1 min 30 seg
## ══════════════════════════════════════════

### 🔥 ENGANCHE (primeros 20 segundos — antes de que pongan atención)

> *Pausa de 2 segundos. Mira al jurado. Luego habla sin prisa.*

**"¿Qué pasaría si un corto circuito en un equipo industrial tardara 10 segundos en detectarse?
En ese tiempo ya quemaste el actuador, ya fundiste el fusible… y ya perdiste datos.
Nosotros construimos un sistema que lo detecta en menos de 15 milisegundos.
Desde el borde. Sin depender de internet. Sin exponer un solo dato privado a la nube.
Eso es ARCEUS GRID."**

---

### 📢 PRESENTACIÓN FORMAL DEL PROYECTO (~30 seg)

**"Buenos días. Mi nombre es Owen Mora, y junto a mis compañeros Angelo Charris, Braulio Ribón,
Manuel Pertuz y Milton Sánchez, les presentamos ARCEUS GRID:
una plataforma de monitoreo eléctrico para microrredes 12V DC,
que combina comunicación ESP-NOW, inteligencia artificial federada y Edge Computing
para proteger equipos industriales en tiempo real."**

---

### 🔗 TRANSICIÓN AL PUNTO 2 (agenda)

**"Antes de entrar en detalle, les doy un mapa de lo que vamos a ver..."**
*(avanzar a diapositiva 2)*

---

## ══════════════════════════════════════════
## PUNTO 2 — CONTENIDO / AGENDA
### Diapositiva 2 | ~1 min
## ══════════════════════════════════════════

### 📋 LEER Y EXPLICAR LA ESTRUCTURA

**"La presentación está organizada en tres bloques: Inicio, Desarrollo y Cierre."**

Señalar cada bloque en la diapositiva mientras se menciona:

| Bloque | Tema | Quién lo explica |
|--------|------|-----------------|
| **1. El Problema** | Riesgos en 12V DC y nube | Yo — Owen |
| **2. La Solución** | Plataforma en 3 niveles | Yo — Owen |
| **3. Hardware Físico** | 2 Estaciones + Gateway | Compañero |
| **4. Hardware Seguro** | Aislamiento JD-VCC | Compañero |
| **5. Red ESP-NOW** | INA219 + ESP-NOW + MQTT | Compañero |
| **6. IA y Resultados** | FedAvg + Métricas | Compañero |
| **7. Backend Docker** | SQLite WAL + Monitoreo | Yo — Owen |
| **8. Detección Anomalías** | Z-Score + Corte | Yo — Owen |

**"Empecemos con el problema que nos motivó a construir esto."**

*(ceder la palabra al siguiente expositor o continuar con el problema — depende del orden de grupo)*

> **NOTA:** Si el orden es que Owen pasa al problema inmediatamente, seguir con la diap. 3.
> Si otro compañero toma las diap. 3-6, Owen regresa para las 7 y 8.

---

## ══════════════════════════════════════════
## PUNTO 7 — BACKEND DOCKER + MONITOREO EN VIVO
### Diapositiva 9 | ~1 min 30 seg
## ══════════════════════════════════════════

### 🖥️ INTRODUCCIÓN AL PUNTO 7

*(Owen retoma el micrófono / turno)*

**"Gracias [nombre]. Ahora yo les explico cómo guardamos y visualizamos esos datos
en tiempo real desde el servidor central."**

---

### 📌 CONTENIDO TÉCNICO A EXPONER

La diapositiva 9 dice:
- Broker **Mosquitto** en contenedor Docker
- **SQLite Modo WAL** concurrente
- Gráficas en vivo a **60 FPS**
- Bandas seguras **10.5–13.5 V**

**Guión:**

**"El servidor central corre sobre Docker. ¿Por qué Docker?
Porque nos garantiza que el sistema funcione igual en cualquier máquina,
sin instalar dependencias manualmente. Un solo comando: `docker compose up -d`, y el sistema está listo."**

**"Dentro de Docker tenemos el broker Mosquitto, que recibe los datos del gateway vía MQTT.
Esos datos se persisten en SQLite usando el modo WAL — Write-Ahead Logging —
que permite lecturas y escrituras simultáneas sin bloquear el sistema."**

**"El dashboard de Streamlit muestra las gráficas en tiempo real a 60 FPS,
con bandas de seguridad visuales entre 10.5 y 13.5 voltios.
Si la línea sale de esa banda, el operador lo ve de inmediato."**

---

### ❓ PREGUNTA TRAMPA FRECUENTE — PUNTO 7

> **Jurado:** "¿Por qué no usaron PostgreSQL o MySQL en vez de SQLite?"

**Respuesta modelo:**
*"SQLite en modo WAL es suficiente para esta escala: menos de 10 nodos, menos de 5 escrituras
por segundo. No necesita un servidor separado, cabe en un solo archivo .db,
consume menos de 10 MB de RAM y soporta lecturas concurrentes perfectamente.
PostgreSQL sería sobredimensionado y agregaría complejidad de administración innecesaria
para un sistema embebido de borde."*

---

### 🔗 TRANSICIÓN AL PUNTO 8

**"Y una vez que esos datos están persistidos y visibles, el sistema también los analiza
automáticamente para detectar anomalías. Eso es exactamente lo siguiente..."**

---

## ══════════════════════════════════════════
## PUNTO 8 — DETECCIÓN DE ANOMALÍAS Y CORTE
### Diapositiva 10 | ~2 min
## ══════════════════════════════════════════

### ⚡ INTRODUCCIÓN AL PUNTO 8

**"Este es uno de los puntos más fuertes del proyecto desde el punto de vista matemático y de ingeniería."**

---

### 📌 CONTENIDO TÉCNICO A EXPONER

La diapositiva 10 dice:
- **Sobrecorriente > 1.80 A** → corte inmediato
- **Subtensión < 10.50 V** → alerta amarilla
- **Anomalía estadística:**

```
Z_V = |V - 12.0| / 0.35  >  3.0
```

---

**Guión:**

**"El sistema tiene tres niveles de protección."**

**"Nivel 1 — Protección por corriente:**
Si la corriente supera 1.80 amperios, el relé corta el circuito de inmediato.
Este corte ocurre en el borde, en el ESP32, en menos de 15 milisegundos.
Sin pasar por el servidor, sin esperar confirmación de la nube."**

**"Nivel 2 — Alerta por tensión:**
Si el voltaje cae por debajo de 10.5 voltios, el sistema lanza una alerta amarilla.
Esto indica que la fuente está degradada o que hay una carga excesiva en la red."**

**"Nivel 3 — Detección estadística con Z-Score:**
Aquí está lo matemáticamente elegante del sistema.
Calculamos el Z-Score de la tensión con esta fórmula:"**

> **Z_V = |V − 12.0| / 0.35**

**"¿Qué significa esto? La tensión nominal de nuestra red es 12 voltios.
La desviación estándar histórica del sistema es 0.35 voltios.
Si el voltaje medido se aleja más de 3 desviaciones estándar del valor nominal,
el sistema lo clasifica como anomalía estadística."**

**"Por ejemplo: si medimos 10.95 V:"**
> Z_V = |10.95 − 12.0| / 0.35 = 1.05 / 0.35 = **3.0** → umbral exacto de anomalía

**"Esto no es una regla arbitraria. Viene de la estadística normal:
el 99.73% de los valores normales caen dentro de ±3σ.
Si Z > 3, tenemos evidencia estadística de que algo anómalo está ocurriendo."**

---

### ❓ PREGUNTA TRAMPA FRECUENTE — PUNTO 8

> **Jurado:** "¿Por qué 1.80 A como umbral de corte? ¿Cómo lo determinaron?"

**Respuesta modelo:**
*"El umbral de 1.80 A fue determinado experimentalmente con las cargas reales del laboratorio
(dos bombillas de 12V). La corriente nominal de operación está alrededor de 1.2–1.4 A.
El margen de 1.80 A equivale al 130% de la carga nominal, lo que es un criterio estándar
en protección eléctrica para evitar falsos positivos por arranque de carga
mientras se detectan sobrecargas reales con rapidez suficiente."*

> **Jurado:** "¿El Z-Score es suficiente o necesitan un modelo más complejo?"

**Respuesta modelo:**
*"El Z-Score nos da detección estadística en O(1) — sin entrenamiento, sin parámetros extras.
Para este prototipo es suficiente como primera capa. La segunda capa es el MLP con FedAvg
que aprende patrones más complejos con el tiempo. Son complementarios:
el Z-Score actúa en microsegundos, el MLP mejora con rondas de aprendizaje federado."*

---

### 🏁 CIERRE DE LA PARTICIPACIÓN DE OWEN

*(Al terminar el punto 8, ceder para conclusiones o hacer cierre si es el último en hablar)*

**"En resumen: ARCEUS GRID no solo mide. Protege en el borde, persiste con eficiencia
y detecta anomalías tanto con lógica de ingeniería como con fundamento estadístico.
Muchas gracias."**

---

## ══════════════════════════════════════════
## RESUMEN RÁPIDO — CHEATSHEET OWEN
## ══════════════════════════════════════════

| Punto | Diap. | Frase clave |
|-------|-------|-------------|
| **1 — Apertura** | 1 | *"15 ms. Sin internet. Sin exponer datos."* |
| **2 — Agenda** | 2 | *"Inicio → Desarrollo → Cierre. Empecemos."* |
| **7 — Docker** | 9 | *"`docker compose up -d`. SQLite WAL. 60 FPS."* |
| **8 — Z-Score** | 10 | *"Z = |V−12|/0.35 > 3 → anomalía estadística."* |

### Fórmulas que DEBES saber de memoria:
```
Z_V = |V - 12.0| / 0.35       (Z-Score tensión)
I_corte = 1.80 A               (Umbral de corte)
V_min_alerta = 10.50 V         (Subtensión)
Latencia de corte < 15 ms      (Tiempo de respuesta en borde)
```

---
*Generado automáticamente desde presentacion_arceus_grid.pdf — ARCEUS GRID*
