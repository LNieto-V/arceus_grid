import os
import sys
import time
from pathlib import Path
import streamlit as st
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go

# Add parent directory to python path for internal imports
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from config import DB_PATH, VOLTAGE_MIN_THRESHOLD, VOLTAGE_MAX_THRESHOLD
from database.db_manager import DatabaseManager
from ai.anomaly_detector import AnomalyDetector
from ai.federated_learning import FederatedCoordinator
from mqtt.client import ArceusMQTTClient

st.set_page_config(
    page_title="ARCEUS GRID — Monitor Eléctrico & AIoT",
    page_icon="⚡",
    layout="wide",
    initial_sidebar_state="expanded"
)

# Custom Styling
st.markdown("""
<style>
    .metric-card {
        background-color: #1a1c24;
        border-radius: 8px;
        padding: 15px;
        border-left: 4px solid #00d2ff;
        margin-bottom: 10px;
    }
    .status-normal { color: #00e676; font-weight: bold; }
    .status-warning { color: #ffd600; font-weight: bold; }
    .status-danger { color: #ff1744; font-weight: bold; }
</style>
""", unsafe_allow_html=True)

@st.cache_resource
def get_system_components():
    db = DatabaseManager(DB_PATH)
    detector = AnomalyDetector()
    fed_coordinator = FederatedCoordinator()
    mqtt_client = ArceusMQTTClient(db, detector)
    # Start MQTT client in background if not already started
    mqtt_client.start()
    return db, detector, fed_coordinator, mqtt_client

db, detector, fed_coordinator, mqtt_client = get_system_components()

# Sidebar
st.sidebar.title("⚡ ARCEUS GRID")
st.sidebar.caption("Sistema Experimental de Monitoreo Eléctrico y Aprendizaje Federado")

auto_refresh = st.sidebar.checkbox("Auto-refrescar datos (3s)", value=True)
sample_limit = st.sidebar.slider("Historial de muestras a graficar", min_value=50, max_value=2000, value=300, step=50)

st.sidebar.divider()
st.sidebar.subheader("🔌 Estado de Enlace MQTT")
broker_status = "🟢 Conectado" if mqtt_client.is_connected else "🔴 Desconectado (Reintentando)"
st.sidebar.write(f"Broker: {broker_status}")

# Header
st.title("⚡ ARCEUS GRID — Panel de Operaciones Central")
st.caption("Supervisión de Microred de Corriente Continua (12V DC), Detección de Anomalías e Inferencia Federada")

tabs = st.tabs(["📊 Monitoreo en Tiempo Real", "🕹️ Control Remoto", "🧠 Aprendizaje Federado (FedAvg)", "🚨 Registro de Alarmas"])

# Fetch latest telemetry
df_all = db.get_telemetry_df(limit=sample_limit)
latest_a = db.get_latest_telemetry("A")
latest_b = db.get_latest_telemetry("B")

# ================= TAB 1: TIEMPO REAL =================
with tabs[0]:
    col_a, col_b = st.columns(2)

    # Station A Card
    with col_a:
        st.subheader("🔵 Estación A")
        if latest_a:
            v_a = latest_a["voltage"]
            i_a = latest_a["current"]
            p_a = latest_a["power"]
            status_a = latest_a["status"]
            r1_a = "ON" if latest_a["relay_1"] else "OFF"
            r2_a = "ON" if latest_a["relay_2"] else "OFF"

            col_m1, col_m2, col_m3 = st.columns(3)
            col_m1.metric("Tensión (V)", f"{v_a:.2f} V", delta=f"{v_a-12.0:.2f} V" if abs(v_a-12.0) > 0.05 else None)
            col_m2.metric("Corriente (A)", f"{i_a:.3f} A")
            col_m3.metric("Potencia (W)", f"{p_a:.2f} W")

            status_color = "status-normal" if status_a == "NORMAL" else ("status-warning" if status_a == "WARNING" else "status-danger")
            st.markdown(f"**Estado:** <span class='{status_color}'>{status_a}</span> | **Relés:** B1: `{r1_a}` | B2: `{r2_a}`", unsafe_allow_html=True)
            if latest_a["is_anomaly"]:
                st.warning(f"⚠️ Anomalía activa: {latest_a['notes']}")
        else:
            st.info("Esperando telemetría de la Estación A...")

    # Station B Card
    with col_b:
        st.subheader("🟢 Estación B")
        if latest_b:
            v_b = latest_b["voltage"]
            i_b = latest_b["current"]
            p_b = latest_b["power"]
            status_b = latest_b["status"]
            r1_b = "ON" if latest_b["relay_1"] else "OFF"
            r2_b = "ON" if latest_b["relay_2"] else "OFF"

            col_m1, col_m2, col_m3 = st.columns(3)
            col_m1.metric("Tensión (V)", f"{v_b:.2f} V", delta=f"{v_b-12.0:.2f} V" if abs(v_b-12.0) > 0.05 else None)
            col_m2.metric("Corriente (A)", f"{i_b:.3f} A")
            col_m3.metric("Potencia (W)", f"{p_b:.2f} W")

            status_color = "status-normal" if status_b == "NORMAL" else ("status-warning" if status_b == "WARNING" else "status-danger")
            st.markdown(f"**Estado:** <span class='{status_color}'>{status_b}</span> | **Relés:** B1: `{r1_b}` | B2: `{r2_b}`", unsafe_allow_html=True)
            if latest_b["is_anomaly"]:
                st.warning(f"⚠️ Anomalía activa: {latest_b['notes']}")
        else:
            st.info("Esperando telemetría de la Estación B...")

    st.divider()

    # Graphs Section
    if not df_all.empty:
        col_g1, col_g2 = st.columns(2)

        with col_g1:
            fig_v = px.line(
                df_all,
                x="timestamp",
                y="voltage",
                color="station",
                title="Tensión en Bus de Carga (12V Nominal)",
                labels={"voltage": "Voltaje (V)", "timestamp": "Hora", "station": "Estación"}
            )
            fig_v.add_hline(y=VOLTAGE_MAX_THRESHOLD, line_dash="dash", line_color="red", annotation_text="Max V")
            fig_v.add_hline(y=VOLTAGE_MIN_THRESHOLD, line_dash="dash", line_color="orange", annotation_text="Min V")
            st.plotly_chart(fig_v, use_container_width=True)

        with col_g2:
            fig_i = px.line(
                df_all,
                x="timestamp",
                y="current",
                color="station",
                title="Consumo de Corriente Total por Estación",
                labels={"current": "Corriente (A)", "timestamp": "Hora", "station": "Estación"}
            )
            st.plotly_chart(fig_i, use_container_width=True)

        fig_p = px.area(
            df_all,
            x="timestamp",
            y="power",
            color="station",
            title="Potencia Eléctrica Disipada (W)",
            labels={"power": "Potencia (W)", "timestamp": "Hora", "station": "Estación"}
        )
        st.plotly_chart(fig_p, use_container_width=True)
    else:
        st.info("No hay registros históricos suficientes para graficar todavía.")

# ================= TAB 2: CONTROL REMOTO =================
with tabs[1]:
    st.subheader("🕹️ Conmutación Remota de Cargas Eléctricas")
    st.write("Envío directo de tramas JSON validadas a través de los tópicos MQTT `arceus/station_x/commands`.")

    col_ctrl_a, col_ctrl_b = st.columns(2)

    with col_ctrl_a:
        st.markdown("### Control Estación A")
        c1, c2 = st.columns(2)
        with c1:
            if st.button("💡 Encender Bombilla 1 (Estación A)", key="btn_on_a1"):
                mqtt_client.send_relay_command("A", 1, True)
                st.success("Comando ON enviado a Relé 1 (Est. A)")
            if st.button("🌑 Apagar Bombilla 1 (Estación A)", key="btn_off_a1"):
                mqtt_client.send_relay_command("A", 1, False)
                st.info("Comando OFF enviado a Relé 1 (Est. A)")

        with c2:
            if st.button("💡 Encender Bombilla 2 (Estación A)", key="btn_on_a2"):
                mqtt_client.send_relay_command("A", 2, True)
                st.success("Comando ON enviado a Relé 2 (Est. A)")
            if st.button("🌑 Apagar Bombilla 2 (Estación A)", key="btn_off_a2"):
                mqtt_client.send_relay_command("A", 2, False)
                st.info("Comando OFF enviado a Relé 2 (Est. A)")

        st.markdown("#### Pilotos Indicadores (Estación A)")
        p_mode_a = st.selectbox("Modo de Pilotos A", ["NORMAL", "WARNING", "DANGER", "OFF"], key="sel_pilot_a")
        if st.button("Aplicar Piloto en A"):
            mqtt_client.send_pilot_command("A", p_mode_a)
            st.success(f"Piloto forzado a {p_mode_a}")

    with col_ctrl_b:
        st.markdown("### Control Estación B")
        c3, c4 = st.columns(2)
        with c3:
            if st.button("💡 Encender Bombilla 1 (Estación B)", key="btn_on_b1"):
                mqtt_client.send_relay_command("B", 1, True)
                st.success("Comando ON enviado a Relé 1 (Est. B)")
            if st.button("🌑 Apagar Bombilla 1 (Estación B)", key="btn_off_b1"):
                mqtt_client.send_relay_command("B", 1, False)
                st.info("Comando OFF enviado a Relé 1 (Est. B)")

        with c4:
            if st.button("💡 Encender Bombilla 2 (Estación B)", key="btn_on_b2"):
                mqtt_client.send_relay_command("B", 2, True)
                st.success("Comando ON enviado a Relé 2 (Est. B)")
            if st.button("🌑 Apagar Bombilla 2 (Estación B)", key="btn_off_b2"):
                mqtt_client.send_relay_command("B", 2, False)
                st.info("Comando OFF enviado a Relé 2 (Est. B)")

        st.markdown("#### Pilotos Indicadores (Estación B)")
        p_mode_b = st.selectbox("Modo de Pilotos B", ["NORMAL", "WARNING", "DANGER", "OFF"], key="sel_pilot_b")
        if st.button("Aplicar Piloto en B"):
            mqtt_client.send_pilot_command("B", p_mode_b)
            st.success(f"Piloto forzado a {p_mode_b}")

# ================= TAB 3: FEDERATED LEARNING =================
with tabs[2]:
    st.subheader("🧠 Coordinador de Aprendizaje Federado (FedAvg)")
    st.markdown("""
    Cada estación entrena de forma descentralizada un modelo sobre sus perfiles de consumo eléctrico local.
    El coordinador agrega los gradientes/pesos usando el algoritmo canónico **FedAvg**:
    $$\\theta_{global}^{(t+1)} = \\sum_{k=1}^K \\frac{n_k}{n} \\theta_k^{(t)}$$
    """)

    col_fl1, col_fl2 = st.columns([1, 2])

    with col_fl1:
        st.markdown("#### Ejecutar Ronda Federada")
        st.write("Participantes activos: **Estación A** y **Estación B**")

        if st.button("🚀 Iniciar Ronda FedAvg Ahora", type="primary"):
            with st.spinner("Extrayendo datos locales y ejecutando entrenamiento federado..."):
                data_a = db.get_station_training_data("A", limit=500)
                data_b = db.get_station_training_data("B", limit=500)
                station_inputs = {"A": data_a, "B": data_b}

                metrics = fed_coordinator.execute_round(station_inputs)
                db.insert_federated_round(
                    round_num=metrics["round"],
                    participants=metrics["participants"],
                    global_loss=metrics["global_loss"],
                    metrics=metrics,
                    weights_file=metrics["checkpoint"]
                )
                st.success(f"¡Ronda {metrics['round']} completada! Pérdida global: {metrics['global_loss']:.5f}")

    with col_fl2:
        fed_history = db.get_federated_history(limit=20)
        if fed_history:
            st.markdown("#### Histórico de Rondas Federadas")
            df_fed = pd.DataFrame(fed_history)
            st.dataframe(df_fed[["round_number", "timestamp", "participants", "global_loss"]], use_container_width=True)

            fig_loss = px.line(
                df_fed.sort_values("round_number"),
                x="round_number",
                y="global_loss",
                markers=True,
                title="Convergencia de la Pérdida Global (FedAvg)",
                labels={"round_number": "Ronda Federada", "global_loss": "Error Cuadrático Medio (MSE)"}
            )
            st.plotly_chart(fig_loss, use_container_width=True)
        else:
            st.info("Aún no se han ejecutado rondas de aprendizaje federado.")

# ================= TAB 4: ALARMAS =================
with tabs[3]:
    st.subheader("🚨 Registro Central de Alarmas y Eventos")
    alerts_list = db.get_alerts(limit=50)
    if alerts_list:
        df_alerts = pd.DataFrame(alerts_list)
        st.dataframe(df_alerts[["timestamp", "station", "severity", "alert_type", "message"]], use_container_width=True)
    else:
        st.success("No se registran alarmas activas en la red. Sistema operando con normalidad.")

# Auto-refresh loop using rerun
if auto_refresh:
    time.sleep(3)
    st.rerun()
