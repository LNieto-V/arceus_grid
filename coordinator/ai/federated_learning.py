import json
import logging
from pathlib import Path
from typing import Dict, Any, List, Tuple
import numpy as np
import pandas as pd
from config import FEDERATED_ROUNDS_DIR, LOCAL_EPOCHS, LEARNING_RATE

logger = logging.getLogger(__name__)

class LocalModel:
    """
    Lightweight 2-layer Neural Model for Electrical Impedance & Power Profiling.
    Predicts expected current given [voltage, relay_1, relay_2, nominal_power_factor].
    Architecture: Input (3) -> Hidden (8, ReLU) -> Output (1, Linear).
    """

    def __init__(self, input_dim: int = 3, hidden_dim: int = 8, output_dim: int = 1):
        self.input_dim = input_dim
        self.hidden_dim = hidden_dim
        self.output_dim = output_dim
        self.weights: Dict[str, np.ndarray] = self._init_weights()

    def _init_weights(self) -> Dict[str, np.ndarray]:
        rng = np.random.RandomState(42)
        return {
            "W1": rng.randn(self.input_dim, self.hidden_dim) * np.sqrt(2.0 / self.input_dim),
            "b1": np.zeros((1, self.hidden_dim)),
            "W2": rng.randn(self.hidden_dim, self.output_dim) * np.sqrt(2.0 / self.hidden_dim),
            "b2": np.zeros((1, self.output_dim))
        }

    def get_weights(self) -> Dict[str, np.ndarray]:
        return {k: v.copy() for k, v in self.weights.items()}

    def set_weights(self, new_weights: Dict[str, np.ndarray]) -> None:
        self.weights = {k: v.copy() for k, v in new_weights.items()}

    def forward(self, X: np.ndarray) -> Tuple[np.ndarray, np.ndarray]:
        """Forward pass with ReLU activation."""
        Z1 = np.dot(X, self.weights["W1"]) + self.weights["b1"]
        A1 = np.maximum(0, Z1)  # ReLU
        Y_pred = np.dot(A1, self.weights["W2"]) + self.weights["b2"]
        return Y_pred, A1

    def train_on_data(self, X: np.ndarray, y: np.ndarray, epochs: int = LOCAL_EPOCHS, lr: float = LEARNING_RATE) -> float:
        """Local SGD training on private station data."""
        if len(X) == 0:
            return 0.0

        n = len(X)
        final_loss = 0.0

        for epoch in range(epochs):
            # Forward
            Y_pred, A1 = self.forward(X)
            diff = Y_pred - y
            loss = np.mean(diff ** 2)
            final_loss = float(loss)

            # Backward
            dY = (2.0 / n) * diff
            dW2 = np.dot(A1.T, dY)
            db2 = np.sum(dY, axis=0, keepdims=True)

            dA1 = np.dot(dY, self.weights["W2"].T)
            dZ1 = dA1 * (A1 > 0)  # ReLU derivative
            dW1 = np.dot(X.T, dZ1)
            db1 = np.sum(dZ1, axis=0, keepdims=True)

            # Gradient Step
            self.weights["W2"] -= lr * dW2
            self.weights["b2"] -= lr * db2
            self.weights["W1"] -= lr * dW1
            self.weights["b1"] -= lr * db1

        return final_loss


class FederatedCoordinator:
    """
    Orchestrates the Federated Averaging (FedAvg) rounds between Station A and Station B.
    Formula:
        θ_global = Σ (n_k / n_total) * θ_k
    """

    def __init__(self, rounds_dir: Path = FEDERATED_ROUNDS_DIR):
        self.rounds_dir = rounds_dir
        self.global_model = LocalModel()
        self.current_round = 0

    def preprocess_station_df(self, df: pd.DataFrame) -> Tuple[np.ndarray, np.ndarray]:
        """Extract features [voltage, relay_1, relay_2] and target [current]."""
        if df.empty or len(df) < 5:
            # Fallback synthetic seed if not enough physical samples yet
            rng = np.random.RandomState(42)
            n_samples = 30
            v = 12.0 + rng.randn(n_samples) * 0.2
            r1 = rng.choice([0, 1], size=n_samples)
            r2 = rng.choice([0, 1], size=n_samples)
            # Simulated current: ~0.45A per bulb + small noise
            i = (r1 * 0.45 + r2 * 0.45) * (v / 12.0) + rng.randn(n_samples) * 0.01
            X = np.column_stack([v, r1, r2])
            y = i.reshape(-1, 1)
            return X, y

        X = df[["voltage", "relay_1", "relay_2"]].to_numpy(dtype=np.float32)
        y = df[["current"]].to_numpy(dtype=np.float32)
        return X, y

    def execute_round(self, station_data: Dict[str, pd.DataFrame]) -> Dict[str, Any]:
        """
        Executes one FedAvg aggregation round across participating stations.
        """
        self.current_round += 1
        participants = list(station_data.keys())
        local_weights: List[Dict[str, np.ndarray]] = []
        sample_counts: List[int] = []
        local_losses: Dict[str, float] = {}

        # 1. Distribute global weights and train locally
        global_weights = self.global_model.get_weights()

        for station_id, df in station_data.items():
            X, y = self.preprocess_station_df(df)
            n_k = len(X)
            sample_counts.append(n_k)

            # Local worker
            client_model = LocalModel()
            client_model.set_weights(global_weights)
            loss_k = client_model.train_on_data(X, y)
            local_losses[station_id] = loss_k
            local_weights.append(client_model.get_weights())

        # 2. FedAvg Aggregation: Weighted Average of Parameters
        total_samples = sum(sample_counts)
        aggregated_weights = {}

        for key in global_weights.keys():
            weighted_sum = np.zeros_like(global_weights[key])
            for i, client_w in enumerate(local_weights):
                weight_factor = sample_counts[i] / total_samples
                weighted_sum += weight_factor * client_w[key]
            aggregated_weights[key] = weighted_sum

        # 3. Update global model
        self.global_model.set_weights(aggregated_weights)

        # 4. Global loss evaluation
        global_loss = float(np.mean(list(local_losses.values())))

        # 5. Save round checkpoint
        checkpoint_path = self.rounds_dir / f"fedavg_round_{self.current_round}.json"
        serializable_weights = {
            k: v.tolist() for k, v in aggregated_weights.items()
        }
        with open(checkpoint_path, "w") as f:
            json.dump({
                "round": self.current_round,
                "participants": participants,
                "sample_counts": sample_counts,
                "local_losses": local_losses,
                "global_loss": global_loss,
                "weights": serializable_weights
            }, f, indent=2)

        metrics = {
            "round": self.current_round,
            "participants": participants,
            "sample_counts": dict(zip(participants, sample_counts)),
            "local_losses": local_losses,
            "global_loss": global_loss,
            "checkpoint": str(checkpoint_path)
        }

        logger.info("FedAvg Round %d complete. Global loss: %.5f", self.current_round, global_loss)
        return metrics
