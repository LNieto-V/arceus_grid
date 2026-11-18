# 🎙️ MÓDULO 5: GUION MAESTRO DE SUSTENTACIÓN — OWEN MORA
## Speech Cronometrado, Frases de Entrada y Handoffs

---

## ⏱️ APERTURA DEL PROYECTO (Minuto 0:00 a 3:00)

### 🎙️ Entrada Fuerte (Los primeros 60 segundos):
> *"Buenos días, señores miembros del jurado. En la industria moderna, el mantenimiento predictivo de maquinaria rotativa —como motores, bombas y abanicos— es la diferencia entre una operación continua y pérdidas millonarias por paradas imprevistas.*  
>  
> *Tradicionalmente, la industria ha intentado resolver esto enviando telemetría continua a la nube. Sin embargo, en entornos reales este modelo fracasa por tres fallas estructurales: saturación del ancho de banda industrial, latencias de cientos de milisegundos y exposición de propiedad intelectual.*  
>  
> *Frente a este problema, desarrollamos **ARCEUS GRID**: una plataforma experimental de instrumentación y diagnóstico eléctrico en el borde (AIoT) basada en microcontroladores ESP32, comunicación híbrida ESP-NOW y MQTT, y Aprendizaje Federado con el algoritmo FedAvg orquestado en contenedores Docker."*

---

## ⏱️ DEFENSA DE LA INTELIGENCIA ARTIFICIAL (Minuto 12:00 a 15:00)

### 🎙️ Speech de la Arquitectura de IA y FedAvg:
> *"A nivel de software e inteligencia artificial, nuestro sistema no comete el error de depender de la nube ni de delegar la seguridad física a un modelo de caja negra.*  
>  
> *Implementamos una **Arquitectura Dual de Detección**:  
> 1. En la primera capa, guardrails deterministas en C++ que operan en menos de un milisegundo en el propio silicio del ESP32, cortando la alimentación si la corriente supera los 2.8 amperios o si detectamos fugas a tierra superiores a 40 miliamperios.  
> 2. En la segunda capa, detección de degradación incipiente mediante Z-score estadístico de tensión y una red neuronal ligera de impedancia implementada en NumPy puro.*  
>  
> *Para coordinar el conocimiento entre máquinas sin centralizar datos crudos, aplicamos **Aprendizaje Federado (FedAvg)**. Cada estación entrena localmente durante 5 épocas de descenso de gradiente estocástico sobre sus propios datos. El computador coordinador recibe únicamente vectores de pesos de 164 bytes y calcula un promedio ponderado según el volumen de datos de cada estación.*  
>  
> *Como resultado, demostramos una precisión de detección superior al 95% ante anomalías inducidas de vibración y sobrecarga térmica, reduciendo el tráfico de red en más del 95% y garantizando operación continua incluso ante caídas totales del enlace Wi-Fi."*

---

## 🤝 CÓMO RESPONDER CUANDO EL JURADO TE INTERRUMPA:
* **Si te piden justificar la elección de tecnologías:**  
  *"Elegimos Docker porque nos da aislamiento de procesos y despliegue en un solo comando sin conflictos de librerías; y SQLite con modo WAL porque nos otorga transacciones ACID atómicas en menos de 10 MB de RAM, permitiendo lecturas y escrituras simultáneas sin bloqueos."*
* **Si te preguntan por qué no usamos PyTorch:**  
  *"Porque PyTorch requiere más de 400 MB de RAM y cientos de megabytes en disco; nuestra red en NumPy pesa 25 KB y ejecuta inferencias en 0.15 milisegundos, lo que permite portarla directamente a C++ en microcontroladores de borde."*
