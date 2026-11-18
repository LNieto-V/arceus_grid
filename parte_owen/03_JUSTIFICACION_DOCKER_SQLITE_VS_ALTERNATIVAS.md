# 🐳 MÓDULO 3: JUSTIFICACIÓN TECNOLÓGICA (DOCKER & SQLITE)
## Por Qué Usamos Esta Arquitectura y No Otras Tecnologías

---

## 1. DOCKER & DOCKER COMPOSE VS. ALTERNATIVAS

### ¿Por qué Docker y no Máquinas Virtuales (VMWare / VirtualBox)?
1. **Consumo de Memoria:** Una máquina virtual necesita levantar un sistema operativo invitado completo, consumiendo $> 2\,\text{GB}$ de RAM y tiempo de booteo de 1 minuto. Los contenedores Docker comparten el kernel del host y corren en milisegundos consumiendo apenas **~120 MB de RAM**.
2. **Hardware de Borde (Edge Gateways):** Si el coordinador corre en una Raspberry Pi 4 o PC industrial en planta, las VMs son inviables. Docker permite microservicios de bajo impacto.

### ¿Por qué Docker y no Instalación Nativa (Bare Metal)?
1. **Reproducibilidad Inmediata ("Zero-Config"):** Elimina las pesadillas de compatibilidad de librerías en Windows vs. Linux. Con un solo comando (`docker compose up -d`), el broker Mosquitto en C, Python 3.11 y Streamlit quedan configurados idénticos en cualquier máquina.
2. **Aislamiento de Silicio y Fallas:** Si el hilo de Streamlit o de IA crashea por falta de memoria, el broker Mosquitto **sigue vivo** en su contenedor y los ESP32 no pierden telemetría.

### ¿Por qué no Kubernetes (K8s)?
* **Sobreingeniería (*Overkill*):** Kubernetes está diseñado para clústeres elásticos con cientos de nodos en la nube. Orquestar 3 contenedores locales con K8s agregaría una penalización masiva de CPU y complejidad innecesaria.

---

## 2. SQLITE (MODO WAL) VS. OTRAS BASES DE DATOS

### ¿Por qué SQLite y no Archivos Planos (CSV / JSON)?
* **Corrupción ante Cortes Eléctricos:** En una fábrica con paradas inesperadas, un corte de energía mientras se escribe un archivo `.csv` corrompe todo el fichero. SQLite ofrece transacciones **ACID atómicas**: la muestra se escribe al 100% o se descarta de forma segura.
* **Consultas de Ventana Temporal:** Consultar los últimos 15 minutos en SQL toma microsegundos; parsear un archivo CSV de 500 MB en Python bloquea el sistema.

### ¿Por qué SQLite y no MongoDB / PostgreSQL?
* **Arquitectura Serverless (Sin Demonios de Red):** MongoDB o PostgreSQL corren como servicios en segundo plano escuchando en puertos TCP, lo que añade vectores de ataque cibernético y riesgo de caídas del daemon. SQLite es una librería integrada directamente en el proceso de Python.
* **Huella de RAM Mínima:** MongoDB exige $> 300\,\text{MB}$ de memoria; SQLite opera con menos de **10 MB**.
* **Portabilidad Académica y de Planta:** Toda la base de datos es un único archivo (`arceus.db`). Se puede copiar a una USB o a Google Colab al instante sin volcados (`pg_dump`).

### El Secreto Clave: ¿Cómo logramos alta concurrencia en SQLite?
Mediante el modo **WAL (Write-Ahead Logging)**:
```sql
PRAGMA journal_mode=WAL;
```
En modo tradicional, SQLite bloquea todo el archivo cuando un hilo escribe. En modo WAL:
* El daemon MQTT escribe telemetría a alta velocidad en un búfer secuencial (`.db-wal`).
* El panel de Streamlit y el módulo de IA leen simultáneamente los históricos para graficar y entrenar.
* **Los lectores jamás bloquean a los escritores, y los escritores jamás bloquean a los lectores.**
