#include "Controller.h"
#include "Config.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("BLE KVM Switch Starting!");

  Serial.println();
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║  Dual-Device Bluetooth Waker                  ║");
  Serial.println("║  buttons + cloud + web                       ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println();

  Controller::begin();

  Serial.println();
  Serial.println("[System] READY - press a button");
  Serial.println();
}

void loop() {
  Controller::loop();
}
