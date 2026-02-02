#include <TinyWireM.h>   // I2C for ATtiny USI

// TMP112 I2C address when ADD0 = GND
static const uint8_t TMP112_ADDR = 0x48;

// LED pin (PB1 on ATtiny85)
const uint8_t LED_PIN = 1;

// Thresholds (°C)
const float ON_THRESH  = 0.5;
const float OFF_THRESH = 0;

// Required ON time (milliseconds)
const unsigned long REQUIRED_ON_TIME = 30000; // 30 seconds

bool ledOn = false;
unsigned long aboveThreshStart = 0;
bool timingActive = false;

float readTMP112_C() {
  TinyWireM.beginTransmission(TMP112_ADDR);
  TinyWireM.send(0x00);
  TinyWireM.endTransmission();

  TinyWireM.requestFrom(TMP112_ADDR, (uint8_t)2);
  if (TinyWireM.available() < 2) return NAN;

  uint8_t msb = TinyWireM.receive();
  uint8_t lsb = TinyWireM.receive();

  int16_t raw = (int16_t)((msb << 8) | lsb);
  raw >>= 4;

  if (raw & 0x0800) raw |= 0xF000;

  return raw * 0.0625f;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  TinyWireM.begin();
}

void loop() {
  float tempC = readTMP112_C();

  if (!isnan(tempC)) {

    // --- TURN ON LOGIC (requires 30 sec above threshold) ---
    if (!ledOn) {
      if (tempC >= ON_THRESH) {
        if (!timingActive) {
          aboveThreshStart = millis();   // Start timing
          timingActive = true;
        }

        // Check if 30 seconds have passed
        if (millis() - aboveThreshStart >= REQUIRED_ON_TIME) {
          ledOn = true;
          digitalWrite(LED_PIN, HIGH);
        }
      } 
      else {
        // Reset timer if temp drops below ON threshold
        timingActive = false;
      }
    }

    // --- TURN OFF LOGIC (immediate, hysteresis) ---
    else if (ledOn && tempC < OFF_THRESH) {
      ledOn = false;
      digitalWrite(LED_PIN, LOW);
      timingActive = false;
    }
  }

  delay(250);
}
