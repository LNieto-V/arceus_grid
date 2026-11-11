#include "display_oled.h"

DisplayOLEDManager::DisplayOLEDManager()
    : m_display(128, 64, &Wire, -1), m_initialized(false) {}

bool DisplayOLEDManager::begin(uint8_t i2c_address) {
    if (!m_display.begin(SSD1306_SWITCHCAPVCC, i2c_address)) {
        Serial.printf("[OLED ERROR] No se pudo inicializar SSD1306 en 0x%02X\n", i2c_address);
        m_initialized = false;
        return false;
    }
    m_initialized = true;
    m_display.clearDisplay();
    m_display.setTextColor(SSD1306_WHITE);
    m_display.setTextSize(1);
    m_display.setCursor(15, 20);
    m_display.println(F("ARCEUS GRID"));
    m_display.setCursor(15, 35);
    m_display.println(F("Iniciando nodo..."));
    m_display.display();
    return true;
}

void DisplayOLEDManager::render(char stationId,
                                const ElectricalMetrics& metrics,
                                bool relay1,
                                bool relay2,
                                bool mqttConnected,
                                PilotMode pilotMode,
                                const char* alertMsg) {
    if (!m_initialized) return;

    m_display.clearDisplay();
    m_display.setTextColor(SSD1306_WHITE);

    // Barra superior de Titulo y Estado de Red
    m_display.setTextSize(1);
    m_display.setCursor(0, 0);
    m_display.printf("ESTACION %c  [%s]", stationId, mqttConnected ? "MQTT" : "DISC");
    m_display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

    // Mediciones electricas principales
    m_display.setCursor(0, 13);
    m_display.printf("V: %5.2f V", metrics.busVoltage_V);

    m_display.setCursor(70, 13);
    m_display.printf("P: %4.1f W", metrics.power_W);

    m_display.setCursor(0, 25);
    m_display.printf("I: %5.3f A (%4.0f mA)", metrics.current_A, metrics.current_A * 1000.0f);

    // Estado de Reles
    m_display.drawFastHLine(0, 36, 128, SSD1306_WHITE);
    m_display.setCursor(0, 40);
    m_display.printf("RELES: B1:%s B2:%s", relay1 ? "ON " : "OFF", relay2 ? "ON " : "OFF");

    // Barra inferior: Alerta o Estado Operativo
    m_display.setCursor(0, 52);
    if (alertMsg && strlen(alertMsg) > 0) {
        m_display.printf("! %s", alertMsg);
    } else {
        const char* stStr = "NORMAL";
        if (pilotMode == PILOT_MODE_WARNING) stStr = "WARNING";
        else if (pilotMode == PILOT_MODE_DANGER) stStr = "DANGER!";
        else if (pilotMode == PILOT_MODE_OFF) stStr = "OFF";
        m_display.printf("ESTADO: %s", stStr);
    }

    m_display.display();
}
