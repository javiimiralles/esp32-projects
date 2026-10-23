#include <Arduino.h>

const int NUM_BUTTONS = 3;
const int BUTTON_PINS[NUM_BUTTONS] = {32, 33, 25};
const int NOTES[NUM_BUTTONS]       = {262, 294, 330};

const int NUM_BUZZERS = 2;
const int BUZZER_PINS[NUM_BUZZERS] = {27, 14};

int currentFrequency[NUM_BUZZERS] = {0, 0};

// --- Thin wrapper over the LEDC (PWM) API, which changed between core versions ---

void buzzerInit(int index) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(BUZZER_PINS[index], 1000, 8);       // pin-based API
#else
  ledcSetup(index, 1000, 8);                     // channel-based API
  ledcAttachPin(BUZZER_PINS[index], index);
#endif
}

void buzzerTone(int index, int frequency) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWriteTone(BUZZER_PINS[index], frequency);  // 0 = silence
#else
  ledcWriteTone(index, frequency);
#endif
}

// --- Main program ---

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    // Internal pull-up enabled: pin reads HIGH when idle, LOW when pressed
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  for (int b = 0; b < NUM_BUZZERS; b++) {
    buzzerInit(b);
    buzzerTone(b, 0); // silence
  }
}

void loop() {
  // 1. Read buttons and assign up to 2 notes (one per buzzer)
  int newFrequency[NUM_BUZZERS] = {0, 0};
  int assigned = 0;

  for (int i = 0; i < NUM_BUTTONS && assigned < NUM_BUZZERS; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      newFrequency[assigned++] = NOTES[i];
    }
  }

  // 2. Only update buzzers whose note changed (avoids clicking sounds)
  for (int b = 0; b < NUM_BUZZERS; b++) {
    if (newFrequency[b] != currentFrequency[b]) {
      buzzerTone(b, newFrequency[b]);
      currentFrequency[b] = newFrequency[b];
      Serial.printf("Buzzer %d -> %d Hz\n", b, newFrequency[b]);
    }
  }

  delay(10); // short pause; also helps with button bounce
}