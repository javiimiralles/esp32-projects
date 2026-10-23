#include <Arduino.h>

const int POT_PIN = 34;
const int LED_PIN = 25;

void setup() {
  Serial.begin(115200);
  Serial.printf("[setup] LED dimmer started (POT_PIN=%d, LED_PIN=%d)\n", POT_PIN, LED_PIN);
}

void loop() {
  int reading = analogRead(POT_PIN);             // 0 to 4095 (12-bit ADC)
  int brightness = map(reading, 0, 4095, 0, 255); // 8-bit PWM
  analogWrite(LED_PIN, brightness);
  Serial.printf("[loop] reading=%d brightness=%d\n", reading, brightness);
  delay(10);
}
