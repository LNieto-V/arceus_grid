import os
from pathlib import Path

# Base Paths
BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = Path(os.getenv("ARCEUS_DATA_DIR", BASE_DIR / "data"))
DATA_DIR.mkdir(parents=True, exist_ok=True)
DB_PATH = DATA_DIR / "arceus_grid.db"

# MQTT Configuration
MQTT_BROKER_HOST = os.getenv("MQTT_BROKER_HOST", "localhost")
MQTT_BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", 1883))
MQTT_KEEPALIVE = 60

# MQTT Topics
TOPIC_TELEMETRY_WILDCARD = "arceus/+/telemetry"
TOPIC_STATUS_WILDCARD = "arceus/+/status"
TOPIC_ALERTS_WILDCARD = "arceus/+/alerts"

def get_telemetry_topic(station_id: str) -> str:
    return f"arceus/station_{station_id.lower()}/telemetry"

def get_command_topic(station_id: str) -> str:
    return f"arceus/station_{station_id.lower()}/commands"

def get_alerts_topic(station_id: str) -> str:
    return f"arceus/station_{station_id.lower()}/alerts"

# Physical and Electrical Bounds (Anomaly Detection Rules)
NOMINAL_VOLTAGE = 12.0  # V
VOLTAGE_MIN_THRESHOLD = 10.5  # V (undervoltage alarm)
VOLTAGE_MAX_THRESHOLD = 13.5  # V (overvoltage alarm)
MAX_RATED_CURRENT_PER_STATION = 3.0  # A
STANDBY_MAX_CURRENT = 0.05  # A when both relays are OFF

# Federated Learning Configuration
FEDERATED_ROUNDS_DIR = DATA_DIR / "federated_models"
FEDERATED_ROUNDS_DIR.mkdir(parents=True, exist_ok=True)
LOCAL_EPOCHS = 5
LEARNING_RATE = 0.01
INPUT_FEATURES = ["voltage", "current", "power"]
