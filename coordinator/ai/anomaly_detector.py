import numpy as np
from typing import Dict, Any, Tuple
from config import (
    VOLTAGE_MIN_THRESHOLD,
    VOLTAGE_MAX_THRESHOLD,
    MAX_RATED_CURRENT_PER_STATION,
    STANDBY_MAX_CURRENT
)

class AnomalyDetector:
    """
    Dual-layer electrical anomaly detector for ARCEUS GRID:
    1. Deterministic physical electrical guardrails (instantaneous response).
    2. Statistical/ML-based anomaly scoring for drift, degraded contacts, and abnormal impedance.
    """

    def __init__(self):
        # Baseline statistical parameters for normal loads
        # Can be updated online via Federated Learning rounds
        self.nominal_voltage = 12.0
        self.voltage_std = 0.35
        self.current_std = 0.15

    def inspect_telemetry(self, data: Dict[str, Any]) -> Tuple[bool, float, str]:
        """
        Evaluate a single telemetry point.
        Returns:
            is_anomaly (bool): Whether an anomaly was detected.
            score (float): Normalized anomaly score [0.0 - 1.0].
            reason (str): Diagnostic description.
        """
        voltage = float(data.get("voltage", 0.0))
        current = float(data.get("current", 0.0))
        power = float(data.get("power", 0.0))
        r1 = bool(data.get("relay_1", False))
        r2 = bool(data.get("relay_2", False))

        # --- Rule 1: Extreme Voltage Guardrails ---
        if voltage < VOLTAGE_MIN_THRESHOLD:
            return True, 0.95, f"Subtensión crítica: {voltage:.2f} V (< {VOLTAGE_MIN_THRESHOLD} V)"
        if voltage > VOLTAGE_MAX_THRESHOLD:
            return True, 1.0, f"Sobretensión crítica: {voltage:.2f} V (> {VOLTAGE_MAX_THRESHOLD} V)"

        # --- Rule 2: Overcurrent Protection ---
        if current > MAX_RATED_CURRENT_PER_STATION:
            return True, 1.0, f"Sobrecorriente: {current:.2f} A (> {MAX_RATED_CURRENT_PER_STATION} A)"

        # --- Rule 3: Ghost current / Leakage when both relays are OFF ---
        if not r1 and not r2 and current > STANDBY_MAX_CURRENT:
            score = min(1.0, current / 0.5)
            return True, score, f"Corriente fantasma/fuga sin cargas activas: {current*1000:.1f} mA"

        # --- Rule 4: Open circuit / Burned bulb when relays are ON ---
        if (r1 or r2) and current < 0.03:
            return True, 0.85, "Circuito abierto: Relé activado pero corriente nula (< 30 mA)"

        # --- Rule 5: Electrical Coherence (INA219 Power vs V*I) ---
        expected_power = voltage * current
        discrepancy = abs(power - expected_power)
        if discrepancy > 1.2:
            return True, 0.75, f"Discrepancia en medición INA219 (V*I={expected_power:.2f}W vs P={power:.2f}W)"

        # --- Layer 2: Statistical Normalized Distance (Z-score based) ---
        z_v = abs(voltage - self.nominal_voltage) / max(self.voltage_std, 1e-4)
        score = min(0.6, (z_v / 5.0) * 0.5)

        if score > 0.5:
            return True, float(score), f"Desviación estadística del bus de tensión (Z={z_v:.2f})"

        return False, float(score), "Operación nominal"
