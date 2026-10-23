#include <Arduino.h>

const int POT_PIN = 34;
const int R_PIN = 25, G_PIN = 26, B_PIN = 27;
const bool COMMON_ANODE = false;
const unsigned long LOG_INTERVAL_MS = 200;  // throttle logs; loop runs every 10 ms

void writeChannel(int pin, int value) {
  analogWrite(pin, COMMON_ANODE ? 255 - value : value);
}

void loop() {
  int raw = analogRead(POT_PIN);
  int h = map(raw, 0, 4095, 0, 1535); // color wheel of 6 segments of 256
  int segment = h / 256, x = h % 256;
  int r = 0, g = 0, b = 0;

  switch (segment) {
    case 0: r = 255;     g = x;       break; // red → yellow
    case 1: r = 255 - x; g = 255;     break; // yellow → green
    case 2: g = 255;     b = x;       break; // green → cyan
    case 3: g = 255 - x; b = 255;     break; // cyan → blue
    case 4: r = x;       b = 255;     break; // blue → magenta
    case 5: r = 255;     b = 255 - x; break; // magenta → red
  }
  writeChannel(R_PIN, r); writeChannel(G_PIN, g); writeChannel(B_PIN, b);

  static unsigned long lastLog = 0;
  if (millis() - lastLog >= LOG_INTERVAL_MS) {
    lastLog = millis();
    Serial.printf("pot=%4d hue=%4d segment=%d -> R=%3d G=%3d B=%3d\n", raw, h, segment, r, g, b);
  }
  delay(10);
}

void setup() {
  Serial.begin(115200);
  Serial.printf("RGB mixer started (pot=GPIO%d, R=GPIO%d, G=GPIO%d, B=GPIO%d, common anode=%s)\n",
                POT_PIN, R_PIN, G_PIN, B_PIN, COMMON_ANODE ? "yes" : "no");
}
