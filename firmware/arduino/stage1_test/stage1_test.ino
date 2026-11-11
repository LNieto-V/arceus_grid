/**
 * ARCEUS GRID - Etapa 1: Prueba Didáctica
 * Plataforma: ESP32 DevKit V1
 * Periféricos: 3 LEDs en GPIO16 (Verde), GPIO17 (Amarillo), GPIO18 (Rojo) + Pantalla OLED SSD1306 (I2C)
 *
 * Comportamiento por comandos seriales (115200 baudios):
 *  '0' -> OFF: Todos los LEDs apagados.
 *  '1' -> NORMAL: LED Verde fijo encendido.
 *  '2' -> WARNING: LED Amarillo intermitente (período: 700 ms).
 *  '3' -> DANGER: LED Rojo intermitente rápido (período: 200 ms).
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_I2C_ADDRESS 0x3C

// Pines de prueba de etapa 1
#define LED_GREEN_PIN 16
#define LED_YELLOW_PIN 17
#define LED_RED_PIN 18

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum TestState {
    STATE_OFF = 0,
    STATE_NORMAL = 1,
    STATE_WARNING = 2,
    STATE_DANGER = 3
};

TestState currentState = STATE_OFF;
unsigned long lastBlinkMillis = 0;
bool blinkOutputState = false;

void updateOLED(const char* statusText) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("ARCEUS GRID - ETAPA 1"));
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(10, 24);
    display.println(statusText);

    display.setTextSize(1);
    display.setCursor(0, 52);
    display.print(F("Comando: "));
    display.println((int)currentState);
    display.display();
}

void setAllLedsOff() {
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    setAllLedsOff();

    Wire.begin(21, 22);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        Serial.println(F("[ERROR] No se pudo inicializar la pantalla SSD1306"));
    } else {
        display.clearDisplay();
        updateOLED("SISTEMA OK");
    }

    Serial.println(F("\n=============================================="));
    Serial.println(F(" ARCEUS GRID - BANCO DE PRUEBAS ETAPA 1"));
    Serial.println(F(" Ingrese comando serial:"));
    Serial.println(F("   0: OFF (Apagado)"));
    Serial.println(F("   1: NORMAL (Verde Fijo)"));
    Serial.println(F("   2: WARNING (Amarillo 700ms)"));
    Serial.println(F("   3: DANGER (Rojo 200ms)"));
    Serial.println(F("=============================================="));
}

void loop() {
    // 1. Lectura de comandos por puerto serial
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        switch (cmd) {
            case '0':
                currentState = STATE_OFF;
                setAllLedsOff();
                updateOLED("MODO: OFF");
                Serial.println(F(">> Estado cambiado a: OFF"));
                break;
            case '1':
                currentState = STATE_NORMAL;
                setAllLedsOff();
                digitalWrite(LED_GREEN_PIN, HIGH);
                updateOLED("NORMAL");
                Serial.println(F(">> Estado cambiado a: NORMAL (Verde fijo)"));
                break;
            case '2':
                currentState = STATE_WARNING;
                setAllLedsOff();
                updateOLED("WARNING");
                Serial.println(F(">> Estado cambiado a: WARNING (Amarillo 700ms)"));
                break;
            case '3':
                currentState = STATE_DANGER;
                setAllLedsOff();
                updateOLED("DANGER!");
                Serial.println(F(">> Estado cambiado a: DANGER (Rojo 200ms)"));
                break;
            default:
                // Ignorar saltos de línea y retornos de carro
                if (cmd != '\r' && cmd != '\n') {
                    Serial.print(F("[AVISO] Comando desconocido: "));
                    Serial.println(cmd);
                }
                break;
        }
    }

    // 2. Máquina de estados no bloqueante con millis() para parpadeo
    unsigned long currentMillis = millis();

    if (currentState == STATE_WARNING) {
        // Intermitencia a 700 ms
        if (currentMillis - lastBlinkMillis >= 700) {
            lastBlinkMillis = currentMillis;
            blinkOutputState = !blinkOutputState;
            digitalWrite(LED_YELLOW_PIN, blinkOutputState ? HIGH : LOW);
            digitalWrite(LED_GREEN_PIN, LOW);
            digitalWrite(LED_RED_PIN, LOW);
        }
    } else if (currentState == STATE_DANGER) {
        // Intermitencia rápida a 200 ms
        if (currentMillis - lastBlinkMillis >= 200) {
            lastBlinkMillis = currentMillis;
            blinkOutputState = !blinkOutputState;
            digitalWrite(LED_RED_PIN, blinkOutputState ? HIGH : LOW);
            digitalWrite(LED_GREEN_PIN, LOW);
            digitalWrite(LED_YELLOW_PIN, LOW);
        }
    }
}
