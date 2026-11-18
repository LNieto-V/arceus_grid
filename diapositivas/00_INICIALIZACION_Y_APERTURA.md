# 🎬 ARCEUS GRID — Guía Maestra de Inicialización y Apertura de la Presentación

Este documento contiene las estrategias, guiones verbales y puestas en escena para los **primeros 2 minutos de la exposición**. El inicio es el momento crítico donde se captura la atención del jurado y se establece la autoridad técnica del equipo.

---

## 🎯 1. La Estrategia de los Primeros 120 Segundos

Una apertura estándar aburre rápidamente al jurado (*"Buenos días, hoy les venimos a hablar sobre nuestro proyecto..."*). 
Una **apertura de alto impacto** se compone de 4 fases rápidas:

```text
[0:00 - 0:30] EL GANCHO (The Hook)        ➔ Pregunta provocadora sobre un problema real y costoso.
[0:30 - 0:50] LA PRESENTACIÓN             ➔ Quiénes somos y qué es ARCEUS GRID en 1 sola frase.
[0:50 - 1:30] LA BRECHA TÉCNICA (Problema) ➔ Las 3 fallas de los sistemas convencionales.
[1:30 - 2:00] LA PROMESA Y HOJA DE RUTA    ➔ Qué van a ver hoy (Hardware + ESP-NOW + FedAvg en vivo).
```

---

## 🗣️ 2. Guiones de Apertura (Elige según tu estilo)

### Opción A: Enfoque Industrial e Impactante (Recomendado)
> *"Estimados jurados y docentes, muy buenos días.*
>
> *¿Qué ocurre en una microrred eléctrica o en una planta industrial cuando una sobrecarga de corriente tarda cientos de milisegundos en detectarse? La respuesta suele ser fatal: componentes destruidos, cortes intempestivos y pérdidas económicas.*
>
> *Pero además hay un segundo problema más silencioso: en la era de la Industria 4.0, para intentar predecir estas fallas, la industria tradicional intenta enviar millones de lecturas crudas a la nube, colapsando el ancho de banda y exponiendo información crítica sobre los patrones operativos de la instalación.*
>
> *Para resolver esta doble problemática creamos **ARCEUS GRID**: una plataforma integral ciberfísica que combina instrumentación de alta velocidad en el borde con microcontroladores ESP32, una red de radio privada con latencia inferior a 5 milisegundos mediante ESP-NOW, y algoritmos de **Aprendizaje Federado**, permitiendo que las estaciones eléctricas aprendan a predecir fallas colaborativamente sin enviar un solo dato privado a la red.*
>
> *En los próximos 12 minutos les mostraremos la arquitectura de potencia, los desafíos electromagnéticos que resolvimos en hardware y concluiremos con una demostración en tiempo real de control y entrenamiento federado."*

---

### Opción B: Enfoque Académico e Ingenieril (Científico / Investigativo)
> *"Buenos días a todos los presentes.*
>
> *La transición energética hacia microrredes distribuidas en corriente continua plantea un desafío fundamental en la ingeniería de control: **¿Cómo lograr supervisión eléctrica granular y analítica predictiva avanzada en sistemas de bajo costo, garantizando al mismo tiempo el aislamiento electromagnético y la privacidad de los datos?***
>
> *Mi nombre es Owen Mora y junto a mi equipo de trabajo hemos desarrollado **ARCEUS GRID**.*
>
> *Este proyecto demuestra la convergencia de cuatro disciplinas: la electrónica de potencia de baja tensión, el diseño de firmware no bloqueante para ESP32, los protocolos de comunicación industrial basados en MQTT y radio de capa 2, y la Inteligencia Artificial aplicada en el Borde mediante el algoritmo Federated Averaging (FedAvg).*
>
> *A lo largo de esta presentación desglosaremos el diseño circuital, el modelo de red híbrida y los resultados experimentales que validan la efectividad del sistema."*

---

## 📋 3. Checklist Técnico de los 60 Segundos Previos (Antes de decir la primera palabra)

Antes de iniciar la presentación, asegúrate de que el entorno esté preparado para no perder tiempo ni credibilidad:

1. **Hardware en Estado Nominal:**
   - [ ] Fuente de 12V conectada y alimentando el sistema.
   - [ ] Pilotos industriales en **Verde Fijo** en ambas estaciones.
   - [ ] Pantallas OLED mostrando: `NORMAL - EN LÍNEA` con los voltajes (~12.0V).
2. **Backend Docker Activo en Segundo Plano:**
   - [ ] Contenedores `arceus_mosquitto`, `arceus_collector` y `arceus_dashboard` corriendo (`docker compose ps`).
3. **Pestañas de Navegador Listas:**
   - [ ] Pestaña 1: Diapositivas proyectadas en pantalla completa (F11).
   - [ ] Pestaña 2: Dashboard Streamlit en `http://localhost:8501` ya precargado y conectado al broker.

---

## 💡 4. Lenguaje Corporal y Puesta en Escena durante el Inicio

* **No mires la diapositiva de portada:** Di las primeras tres frases mirando directamente a los ojos del jurado más exigente.
* **Señala el hardware físicamente:** Cuando pronuncies *"ARCEUS GRID"*, haz un gesto abierto con la mano hacia el banco de trabajo donde están los ESP32, los relés y los pilotos encendidos. Demuestra desde el segundo 20 que hay un desarrollo físico real operando.
* **Voz firme y pausada:** Comienza con un volumen 10% más alto de lo normal para romper el murmullo de la sala. Pausa 1 segundo completo después de la pregunta inicial (*"The Hook"*).
