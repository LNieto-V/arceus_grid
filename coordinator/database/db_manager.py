import sqlite3
import json
import logging
from datetime import datetime
from pathlib import Path
from typing import Dict, Any, List, Optional
import pandas as pd
from config import DB_PATH

logger = logging.getLogger(__name__)

class DatabaseManager:
    """Thread-safe SQLite manager for ARCEUS GRID with WAL mode enabled."""

    def __init__(self, db_path: Path = DB_PATH):
        self.db_path = db_path
        self._init_database()

    def _get_connection(self) -> sqlite3.Connection:
        conn = sqlite3.connect(str(self.db_path), timeout=30.0)
        conn.row_factory = sqlite3.Row
        # Enable WAL (Write-Ahead Logging) and foreign keys for performance and concurrency
        conn.execute("PRAGMA journal_mode=WAL;")
        conn.execute("PRAGMA synchronous=NORMAL;")
        conn.execute("PRAGMA foreign_keys=ON;")
        return conn

    def _init_database(self) -> None:
        """Create tables and indexes if they do not exist."""
        with self._get_connection() as conn:
            cursor = conn.cursor()

            # Telemetry table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS telemetry (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                station TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                voltage REAL NOT NULL,
                current REAL NOT NULL,
                power REAL NOT NULL,
                relay_1 INTEGER NOT NULL,
                relay_2 INTEGER NOT NULL,
                status TEXT NOT NULL,
                is_anomaly INTEGER DEFAULT 0,
                anomaly_score REAL DEFAULT 0.0,
                notes TEXT
            );
            """)

            # Alerts table
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS alerts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                station TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                alert_type TEXT NOT NULL,
                message TEXT NOT NULL,
                severity TEXT NOT NULL,
                acknowledged INTEGER DEFAULT 0
            );
            """)

            # Commands audit log
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS commands (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                station TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                command TEXT NOT NULL,
                payload TEXT,
                status TEXT DEFAULT 'SENT'
            );
            """)

            # Federated learning rounds log
            cursor.execute("""
            CREATE TABLE IF NOT EXISTS federated_rounds (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                round_number INTEGER NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                participants TEXT NOT NULL,
                global_loss REAL NOT NULL,
                metrics_json TEXT NOT NULL,
                weights_file TEXT
            );
            """)

            # Indexes for fast historical queries
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_telemetry_station_time ON telemetry (station, timestamp);")
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_telemetry_anomaly ON telemetry (is_anomaly);")
            cursor.execute("CREATE INDEX IF NOT EXISTS idx_alerts_time ON alerts (timestamp);")
            conn.commit()
            logger.info("Database initialized successfully at %s", self.db_path)

    def insert_telemetry(self, data: Dict[str, Any]) -> int:
        """Insert a telemetry record into the database."""
        timestamp = data.get("timestamp") or datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            INSERT INTO telemetry (
                station, timestamp, voltage, current, power,
                relay_1, relay_2, status, is_anomaly, anomaly_score, notes
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                data["station"].upper(),
                timestamp,
                float(data["voltage"]),
                float(data["current"]),
                float(data["power"]),
                1 if data.get("relay_1") else 0,
                1 if data.get("relay_2") else 0,
                data.get("status", "NORMAL"),
                1 if data.get("is_anomaly") else 0,
                float(data.get("anomaly_score", 0.0)),
                data.get("notes", "")
            ))
            conn.commit()
            return cursor.lastrowid

    def insert_alert(self, station: str, alert_type: str, message: str, severity: str = "WARNING") -> int:
        """Insert an operational alert."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            INSERT INTO alerts (station, alert_type, message, severity)
            VALUES (?, ?, ?, ?)
            """, (station.upper(), alert_type, message, severity.upper()))
            conn.commit()
            return cursor.lastrowid

    def insert_command(self, station: str, command: str, payload: Dict[str, Any]) -> int:
        """Log a sent command."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            INSERT INTO commands (station, command, payload, status)
            VALUES (?, ?, ?, 'SENT')
            """, (station.upper(), command, json.dumps(payload)))
            conn.commit()
            return cursor.lastrowid

    def insert_federated_round(self, round_num: int, participants: List[str], global_loss: float,
                               metrics: Dict[str, Any], weights_file: Optional[str] = None) -> int:
        """Record the outcome of a Federated Learning round."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            INSERT INTO federated_rounds (round_number, participants, global_loss, metrics_json, weights_file)
            VALUES (?, ?, ?, ?, ?)
            """, (round_num, ",".join(participants), global_loss, json.dumps(metrics), weights_file or ""))
            conn.commit()
            return cursor.lastrowid

    def get_latest_telemetry(self, station: str) -> Optional[Dict[str, Any]]:
        """Retrieve the latest telemetry record for a specific station."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
            SELECT * FROM telemetry WHERE station = ? ORDER BY id DESC LIMIT 1
            """, (station.upper(),))
            row = cursor.fetchone()
            return dict(row) if row else None

    def get_telemetry_df(self, station: Optional[str] = None, limit: int = 500) -> pd.DataFrame:
        """Retrieve telemetry records as a pandas DataFrame."""
        with self._get_connection() as conn:
            if station:
                query = "SELECT * FROM telemetry WHERE station = ? ORDER BY id DESC LIMIT ?"
                df = pd.read_sql_query(query, conn, params=(station.upper(), limit))
            else:
                query = "SELECT * FROM telemetry ORDER BY id DESC LIMIT ?"
                df = pd.read_sql_query(query, conn, params=(limit,))
            if not df.empty:
                df["timestamp"] = pd.to_datetime(df["timestamp"])
                df = df.sort_values("id", ascending=True)
            return df

    def get_station_training_data(self, station: str, limit: int = 1000) -> pd.DataFrame:
        """Fetch historical features for model training."""
        with self._get_connection() as conn:
            query = """
            SELECT voltage, current, power, relay_1, relay_2
            FROM telemetry
            WHERE station = ?
            ORDER BY id DESC LIMIT ?
            """
            df = pd.read_sql_query(query, conn, params=(station.upper(), limit))
            return df

    def get_alerts(self, limit: int = 50) -> List[Dict[str, Any]]:
        """Fetch the most recent alerts."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM alerts ORDER BY id DESC LIMIT ?", (limit,))
            return [dict(row) for row in cursor.fetchall()]

    def get_federated_history(self, limit: int = 20) -> List[Dict[str, Any]]:
        """Fetch recent federated rounds."""
        with self._get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM federated_rounds ORDER BY id DESC LIMIT ?", (limit,))
            return [dict(row) for row in cursor.fetchall()]
