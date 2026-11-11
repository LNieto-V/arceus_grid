import time
import signal
import sys
import logging
from config import MQTT_BROKER_HOST, MQTT_BROKER_PORT
from database.db_manager import DatabaseManager
from ai.anomaly_detector import AnomalyDetector
from mqtt.client import ArceusMQTTClient

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] (%(name)s) %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)]
)
logger = logging.getLogger("coordinator_main")

def main():
    logger.info("==================================================")
    logger.info("   ARCEUS GRID — Central Coordinator Daemon      ")
    logger.info("==================================================")

    db_mgr = DatabaseManager()
    anomaly_detector = AnomalyDetector()
    mqtt_client = ArceusMQTTClient(db_mgr, anomaly_detector)

    def shutdown_handler(signum, frame):
        logger.info("Received termination signal (%s). Shutting down cleanly...", signum)
        mqtt_client.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, shutdown_handler)
    signal.signal(signal.SIGTERM, shutdown_handler)

    # Start MQTT background processing
    mqtt_client.start()

    logger.info("Arceus Coordinator is active. Listening for telemetry...")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        shutdown_handler(signal.SIGINT, None)

if __name__ == "__main__":
    main()
