#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// --- Pin configuration ---
constexpr uint8_t DHT_PIN = 4;    // DHT11 DATA -> GPIO4 (D4)
constexpr uint8_t DHT_TYPE = DHT11;
constexpr uint8_t OLED_SDA = 21;  // I2C data  -> GPIO21 (D21)
constexpr uint8_t OLED_SCL = 22;  // I2C clock -> GPIO22 (D22)
constexpr uint8_t LED_PIN = 18;   // Heat alert LED -> GPIO18 (D18)

// --- Display configuration ---
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C; // Some modules use 0x3D
constexpr int8_t OLED_RESET = -1;      // Our module has no reset pin

// The DHT11 can't be read more than about once per second.
constexpr unsigned long READ_INTERVAL_MS = 2000;

// --- Heat alert (with hysteresis) ---
// The alert turns on at HOT_ON_C and only turns off again at HOT_OFF_C.
// This gap prevents the LED from flickering when the temperature
// hovers right around a single threshold.
constexpr float HOT_ON_C = 24.0;
constexpr float HOT_OFF_C = 23.0;

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastReadMs = 0;
bool isHot = false;

void drawHeader() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Weather station");
  display.drawFastHLine(0, 11, SCREEN_WIDTH, SSD1306_WHITE);
}

void showReading(float temperature, float humidity, bool hot) {
  display.clearDisplay();
  drawHeader();

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(temperature, 1);
  display.print((char)248); // Degree symbol in the CP437 charset
  display.print("C");

  display.setCursor(0, 44);
  display.print(humidity, 0);
  display.print(" %");

  if (hot) {
    display.setTextSize(1);
    display.setCursor(100, 24);
    display.print("HOT!");
  }

  display.display(); // Nothing appears until we push the buffer to the screen
}

void showError() {
  display.clearDisplay();
  drawHeader();
  display.setTextSize(1);
  display.setCursor(0, 28);
  display.print("Sensor error");
  display.display();
}

void updateHeatAlert(float temperature) {
  if (!isHot && temperature >= HOT_ON_C) {
    isHot = true;
  } else if (isHot && temperature <= HOT_OFF_C) {
    isHot = false;
  }
  digitalWrite(LED_PIN, isHot ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found. Check wiring and I2C address.");
    for (;;) {
      delay(1000); // Stop here: without a display there's nothing to do
    }
  }

  display.cp437(true); // Use the standard charset so the degree symbol works
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  drawHeader();
  display.setCursor(0, 28);
  display.print("Starting...");
  display.display();

  Serial.println("Weather station - step 3: DHT11 + OLED + heat LED");
}

void loop() {
  unsigned long now = millis();
  if (now - lastReadMs < READ_INTERVAL_MS) {
    return;
  }
  lastReadMs = now;

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11. Check the wiring.");
    showError();
    return;
  }

  updateHeatAlert(temperature);

  Serial.printf("Temperature: %.1f C | Humidity: %.0f %% | Hot: %s\n",
                temperature, humidity, isHot ? "yes" : "no");
  showReading(temperature, humidity, isHot);
}