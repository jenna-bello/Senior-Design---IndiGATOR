#include <TinyWireM.h>   // I2C for ATtiny USI

// TMP112 I2C address when ADD0 = GND
static const uint8_t TMP112_ADDR = 0x48;

// Pick a pin NOT used for I2C.
// If you rewire I2C correctly: SDA=PB0, SCL=PB2, then PB1 is free for LED.
const uint8_t LED_PIN = 1;  // Arduino "1" = PB1 on ATtiny85

// Thresholds (°C)
const float ON_THRESH  = 25;
const float OFF_THRESH = 24.5;

bool ledOn = false;

float readTMP112_C() {
  // Point to temperature register (0x00)
  TinyWireM.beginTransmission(TMP112_ADDR);
  TinyWireM.send(0x00);
  TinyWireM.endTransmission();

  // Read 2 bytes
  TinyWireM.requestFrom(TMP112_ADDR, (uint8_t)2);
  if (TinyWireM.available() < 2) return NAN;

  uint8_t msb = TinyWireM.receive();
  uint8_t lsb = TinyWireM.receive();

  // TMP112 temp is a 12-bit signed value in the top bits of the 16-bit register
  int16_t raw = (int16_t)((msb << 8) | lsb);
  raw >>= 4; // keep top 12 bits

  // Sign extend 12-bit to 16-bit
  if (raw & 0x0800) raw |= 0xF000;

  return raw * 0.0625f; // 0.0625°C per LSB
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  TinyWireM.begin();
}

void loop() {
  float tempC = readTMP112_C();

  if (!isnan(tempC)) {
    if (!ledOn && tempC >= ON_THRESH) {
      ledOn = true;
      digitalWrite(LED_PIN, HIGH);  // if LED wired pin->res->LED->GND
    } 
    else if (ledOn && tempC < OFF_THRESH) {
      ledOn = false;
      digitalWrite(LED_PIN, LOW);
    }
  }

  delay(250);
}
