#include <Wiegand.h>

#define WIEGAND_D0 16
#define WIEGAND_D1 17
#define RELAY1_PIN 13

WIEGAND wg;

const unsigned long debounceTime = 2000;
const unsigned long relayOpenTime = 3000; // 3 detik
unsigned long lastReadTime = 0;
unsigned long relayTriggerTime = 0;
uint32_t lastCard = 0;
bool isRelayActive = false;

uint32_t convertWiegand(uint32_t rawCode, uint8_t bitLength) {
  if (bitLength == 34) return (rawCode >> 1) & 0xFFFFFFFF;
  if (bitLength == 26) return (rawCode >> 1) & 0x00FFFFFF;
  return rawCode;
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW);

  wg.begin(WIEGAND_D0, WIEGAND_D1);
  Serial.println("[GATE-IN] Device Ready");
}

void loop() {
  // Handle relay timeout
  if (isRelayActive && (millis() - relayTriggerTime >= relayOpenTime)) {
    digitalWrite(RELAY1_PIN, LOW);
    isRelayActive = false;
    Serial.println("[GATE-IN] RELAY_DEACTIVATED");
  }

  // Baca perintah serial dari Node.js
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Support format: "[GATE1] OPEN_RELAY" atau "OPEN_RELAY"
    if (command.startsWith("[GATE1] OPEN_RELAY") || command == "OPEN_RELAY") {
      digitalWrite(RELAY1_PIN, HIGH);
      isRelayActive = true;
      relayTriggerTime = millis();
      Serial.println("[GATE-IN] RELAY_ACTIVATED");
    }
  }

  // Baca kartu RFID
  if (wg.available()) {
    uint32_t rawCode = wg.getCode();
    uint8_t bitLength = wg.getWiegandType();

    if (bitLength == 26 || bitLength == 34) {
      uint32_t cardNumber = convertWiegand(rawCode, bitLength);
      unsigned long currentMillis = millis();

      if (cardNumber != lastCard || (currentMillis - lastReadTime) >= debounceTime) {
        lastCard = cardNumber;
        lastReadTime = currentMillis;
        Serial.print("[GATE-IN] ");
        Serial.println(cardNumber);
      }
    }
  }
}