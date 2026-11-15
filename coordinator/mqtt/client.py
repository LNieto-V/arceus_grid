import json
import logging
import time
from typing import Callable, Optional, Dict, Any
import paho.mqtt.client as mqtt

from config import (
    MQTT_BROKER_HOST,
    MQTT_BROKER_PORT,
    MQTT_KEEPALIVE,
    TOPIC_TELEMETRY_WILDCARD,
    TOPIC_STATUS_WILDCARD,
    TOPIC_ALERTS_WILDCARD,
    get_command_topic
)
from database.db_manager import DatabaseManager
from ai.anomaly_detector import AnomalyDetector

logger = logging.getLogger(__name__)

class ArceusMQTTClient:
    """
    Manages MQTT communications for ARCEUS GRID Coordinator:
    Subscribes to telemetry, processes anomalies, writes to SQLite, and issues control commands.
    """

    def __init__(
        self,
        db_manager: DatabaseManager,
        anomaly_detector: AnomalyDetector,
        client_id: Optional[str] = None,
        subscribe_telemetry: bool = True
    ):
        self.db = db_manager
        self.detector = anomaly_detector
        self._is_connected = False
        self.subscribe_telemetry = subscribe_telemetry

        if not client_id:
            import uuid
            self.client_id = f"arceus_coordinator_{uuid.uuid4().hex[:8]}"
        else:
            self.client_id = client_id

        # Support both Paho MQTT v1 and v2
        try:
            # Paho v2 style
            self.client = mqtt.Client(
                mqtt.CallbackAPIVersion.VERSION2,
                client_id=self.client_id,
                clean_session=True
            )
        except AttributeError:
            # Paho v1 style fallback
            self.client = mqtt.Client(
                client_id=self.client_id,
                clean_session=True
            )

        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message

    @property
    def is_connected(self) -> bool:
        try:
            return self.client.is_connected()
        except Exception:
            return self._is_connected

    @is_connected.setter
    def is_connected(self, value: bool):
        self._is_connected = value

    def _on_connect(self, client, userdata, flags, rc, *args):
        if rc == 0:
            self._is_connected = True
            logger.info("Connected to MQTT Broker at %s:%d as %s", MQTT_BROKER_HOST, MQTT_BROKER_PORT, self.client_id)
            if self.subscribe_telemetry:
                # Subscribe to all stations
                client.subscribe([(TOPIC_TELEMETRY_WILDCARD, 1),
                                  (TOPIC_STATUS_WILDCARD, 1),
                                  (TOPIC_ALERTS_WILDCARD, 1)])
                logger.info("Subscribed to telemetry, status, and alerts topics.")
        else:
            logger.error("Failed to connect to MQTT broker with return code: %d", rc)

    def _on_disconnect(self, client, userdata, rc, *args):
        self._is_connected = False
        logger.warning("Disconnected from MQTT broker (rc=%s). Will attempt reconnect.", rc)

    def _on_message(self, client, userdata, msg):
        try:
            topic = msg.topic
            payload_str = msg.payload.decode("utf-8")
            payload = json.loads(payload_str)

            # Route by topic pattern
            if "/telemetry" in topic:
                self._handle_telemetry(payload)
            elif "/alerts" in topic:
                self._handle_station_alert(topic, payload)
            elif "/status" in topic:
                logger.info("Status received [%s]: %s", topic, payload)
        except Exception as e:
            logger.error("Error processing MQTT message on topic %s: %s", msg.topic, e, exc_info=True)

    def _handle_telemetry(self, data: Dict[str, Any]):
        station = data.get("station", "UNKNOWN").upper()
        # Inspect for anomalies
        is_anomaly, score, reason = self.detector.inspect_telemetry(data)

        data["is_anomaly"] = 1 if is_anomaly else 0
        data["anomaly_score"] = score
        data["notes"] = reason

        # Store in SQLite
        row_id = self.db.insert_telemetry(data)

        # Trigger alert record if anomaly found
        if is_anomaly:
            severity = "DANGER" if score > 0.8 else "WARNING"
            self.db.insert_alert(
                station=station,
                alert_type="ANOMALY_DETECTED",
                message=f"[Telemetry #{row_id}] {reason}",
                severity=severity
            )
            logger.warning("ANOMALY [%s]: %s (Score: %.2f)", station, reason, score)

    def _handle_station_alert(self, topic: str, data: Dict[str, Any]):
        station = data.get("station", "UNKNOWN").upper()
        alert_type = data.get("type", "HARDWARE_ALERT")
        message = data.get("message", "Alerta no especificada")
        severity = data.get("severity", "WARNING")
        self.db.insert_alert(station, alert_type, message, severity)
        logger.warning("STATION ALERT [%s]: %s (%s)", station, message, severity)

    def send_relay_command(self, station: str, relay_index: int, state: bool) -> bool:
        """
        Publishes a relay switch command to a station.
        Payload format: {"command": "SET_RELAY", "relay": 1, "state": true}
        """
        topic = get_command_topic(station)
        payload = {
            "command": "SET_RELAY",
            "relay": int(relay_index),
            "state": bool(state),
            "timestamp": time.time()
        }
        payload_bytes = json.dumps(payload).encode("utf-8")
        result = self.client.publish(topic, payload_bytes, qos=1)

        # Audit log in database
        self.db.insert_command(station, f"SET_RELAY_{relay_index}", payload)
        logger.info("Published command to %s: %s", topic, payload)
        return result.rc == mqtt.MQTT_ERR_SUCCESS

    def send_pilot_command(self, station: str, pilot_mode: str) -> bool:
        """
        Overrides pilot mode remotely: 'NORMAL', 'WARNING', 'DANGER', 'OFF'
        """
        topic = get_command_topic(station)
        payload = {
            "command": "SET_PILOT",
            "mode": pilot_mode.upper(),
            "timestamp": time.time()
        }
        payload_bytes = json.dumps(payload).encode("utf-8")
        result = self.client.publish(topic, payload_bytes, qos=1)
        self.db.insert_command(station, f"SET_PILOT_{pilot_mode}", payload)
        return result.rc == mqtt.MQTT_ERR_SUCCESS

    def start(self):
        """Starts the background MQTT network loop."""
        logger.info("Connecting to MQTT broker at %s:%d...", MQTT_BROKER_HOST, MQTT_BROKER_PORT)
        try:
            self.client.connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, MQTT_KEEPALIVE)
            self.client.loop_start()
        except Exception as e:
            logger.error("Could not connect to MQTT broker: %s", e)

    def stop(self):
        """Stops the MQTT client cleanly."""
        self.client.loop_stop()
        self.client.disconnect()
        logger.info("MQTT client stopped.")
