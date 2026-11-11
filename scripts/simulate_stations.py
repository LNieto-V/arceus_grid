#!/usr/bin/env python3
"""
ARCEUS GRID — Simulador de Estaciones A y B
Permite validar toda la arquitectura del Coordinador, Base de Datos,
Detección de Anomalías y Dashboard Streamlit sin requerir hardware físico conectado.
Responde de forma interactiva a comandos de relés y pilotos vía MQTT.
"""

import time
import json
import random
import signal
import sys
import os
import paho.mqtt.client as mqtt

BROKER_HOST = os.getenv("MQTT_BROKER_HOST", "localhost")
BROKER_PORT = int(os.getenv("MQTT_BROKER_PORT", 1883))

# Estado de las estaciones simuladas
stations_state = {
    "A": {
        "relay_1": False,
        "relay_2": False,
        "pilot_mode": "NORMAL",
        "nominal_v": 12.02,
        "bulb_i": 0.45,  # 450 mA por bombilla encendida
        "idle_i": 0.012   # 12 mA consumo residual
    },
    "B": {
        "relay_1": False,
        "relay_2": False,
        "pilot_mode": "NORMAL",
        "nominal_v": 11.98,
        "bulb_i": 0.52,  # 520 mA por bombilla encendida
        "idle_i": 0.015   # 15 mA consumo residual
    }
}

running = True

def on_connect(client, userdata, flags, rc, *args):
    print(f"[SIMULADOR] Conectado al broker MQTT en {BROKER_HOST}:{BROKER_PORT} (rc={rc})")
    # Suscribirse a comandos de ambas estaciones
    client.subscribe([("arceus/station_a/commands", 1), ("arceus/station_b/commands", 1)])
    print("[SIMULADOR] Escuchando comandos en arceus/station_a/commands y arceus/station_b/commands")

def on_message(client, userdata, msg):
    try:
        topic = msg.topic
        payload = json.loads(msg.payload.decode("utf-8"))
        st_id = "A" if "station_a" in topic else "B"

        cmd = payload.get("command")
        if cmd == "SET_RELAY":
            r_idx = payload.get("relay")
            state = bool(payload.get("state"))
            key = f"relay_{r_idx}"
            if key in stations_state[st_id]:
                stations_state[st_id][key] = state
                print(f"[SIMULADOR {st_id}] Relé {r_idx} fijado a {'ON' if state else 'OFF'}")
        elif cmd == "SET_PILOT":
            mode = payload.get("mode", "NORMAL")
            stations_state[st_id]["pilot_mode"] = mode
            print(f"[SIMULADOR {st_id}] Piloto forzado a: {mode}")
    except Exception as e:
        print(f"[SIMULADOR ERROR] Error parseando comando: {e}")

def main():
    global running
    def sig_handler(sig, frame):
        global running
        print("\nDeteniendo simulador...")
        running = False

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    try:
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="arceus_hardware_simulator")
    except AttributeError:
        client = mqtt.Client(client_id="arceus_hardware_simulator")

    client.on_connect = on_connect
    client.on_message = on_message

    print(f"[SIMULADOR] Conectando a {BROKER_HOST}:{BROKER_PORT}...")
    try:
        client.connect(BROKER_HOST, BROKER_PORT, 60)
        client.loop_start()
    except Exception as e:
        print(f"[ERROR CRÍTICO] No se pudo conectar al Broker MQTT en {BROKER_HOST}:{BROKER_PORT}: {e}")
        print("Asegúrate de que Mosquitto esté en ejecución (vía Docker o local).")
        sys.exit(1)

    print("\n[SIMULADOR ACTIVO] Publicando telemetría continua de Estación A y Estación B (cada 1s)...")
    tick = 0
    while running:
        tick += 1
        for st_id, st_data in stations_state.items():
            # Cálculo de voltaje con pequeña ondulación natural (ripple)
            voltage = st_data["nominal_v"] + random.uniform(-0.04, 0.04)

            # Cálculo de corriente según estado de los relés
            current = st_data["idle_i"] + random.uniform(-0.002, 0.002)
            if st_data["relay_1"]:
                current += st_data["bulb_i"] + random.uniform(-0.01, 0.01)
            if st_data["relay_2"]:
                current += st_data["bulb_i"] + random.uniform(-0.01, 0.01)

            # Inyección de pequeña anomalía controlada cada 45 segundos en Estación B para pruebas didácticas
            status = st_data["pilot_mode"]
            if st_id == "B" and tick % 45 == 0:
                voltage = 10.15  # Subtensión simulada
                status = "WARNING"
                print(f"[SIMULADOR B] Inyectando evento de subtensión para validación de alarmas: {voltage:.2f}V")

            current = max(0.0, current)
            power = voltage * current

            telemetry_payload = {
                "station": st_id,
                "voltage": round(voltage, 2),
                "current": round(current, 3),
                "power": round(power, 2),
                "relay_1": st_data["relay_1"],
                "relay_2": st_data["relay_2"],
                "status": status
            }

            topic = f"arceus/station_{st_id.lower()}/telemetry"
            client.publish(topic, json.dumps(telemetry_payload), qos=1)

        time.sleep(1.0)

    client.loop_stop()
    client.disconnect()
    print("[SIMULADOR] Apagado completo.")

if __name__ == "__main__":
    main()
