#!/usr/bin/env bash
# ARCEUS GRID - Script de Configuración de Entorno Local (Alternativa sin Docker)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=========================================================="
echo "  ARCEUS GRID — Configuración del Entorno Python Local   "
echo "=========================================================="

cd "$PROJECT_ROOT"

# 1. Crear entorno virtual si no existe
if [ ! -d ".venv" ]; then
    echo "[1/3] Creando entorno virtual Python en .venv..."
    python3 -m venv .venv
else
    echo "[1/3] Entorno virtual .venv ya existe."
fi

# 2. Activar entorno virtual
source .venv/bin/activate

# 3. Instalar requerimientos del coordinador
echo "[2/3] Instalando dependencias de Python..."
pip install --upgrade pip
pip install -r coordinator/requirements.txt

echo "[3/3] Creando directorios de persistencia de datos..."
mkdir -p coordinator/data/federated_models
mkdir -p mosquitto/data mosquitto/log

echo ""
echo "=========================================================="
echo "  ¡Instalación completada exitosamente!                  "
echo "=========================================================="
echo "Para ejecutar de manera local:"
echo " 1. Iniciar Broker Mosquitto: mosquitto -c mosquitto/config/mosquitto.conf"
echo " 2. Iniciar Coordinador:      source .venv/bin/activate && python coordinator/main.py"
echo " 3. Iniciar Dashboard:        source .venv/bin/activate && streamlit run coordinator/dashboard/app.py"
echo " 4. (Opcional) Simulador:     source .venv/bin/activate && python scripts/simulate_stations.py"
echo ""
echo "O simplemente con Docker Compose:"
echo " docker compose up --build"
echo "=========================================================="
