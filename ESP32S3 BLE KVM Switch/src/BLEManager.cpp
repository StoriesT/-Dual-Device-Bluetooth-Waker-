#include "BLEManager.h"
#include "Config.h"
#include <esp_mac.h>

BLEManager::BLEManager() : _bleCombo(nullptr) {}

void BLEManager::begin(uint8_t slot) {
  const char *deviceNames[NUM_DEVICE_SLOTS] = {DEVICE_NAME_1, DEVICE_NAME_2};
  setUniqueMac(slot);

  Serial.printf("[BLE] Initializing slot %d: '%s'\n", slot + 1,
                deviceNames[slot]);
  _bleCombo = new BleCombo(deviceNames[slot], DEVICE_MANUFACTURER, BATTERY_LEVEL);
  _bleCombo->begin();

  Serial.printf("[BLE] Advertising as '%s'\n", deviceNames[slot]);
}

bool BLEManager::isConnected() {
  return (_bleCombo != nullptr && _bleCombo->isConnected());
}

void BLEManager::sendKeyboardReport(const uint8_t *keys, uint8_t modifiers) {
  if (!isConnected())
    return;

  KeyReport report;
  report.modifiers = modifiers;
  report.reserved = 0;
  memcpy(report.keys, keys, 6);
  _bleCombo->sendReport(&report);
}

void BLEManager::sendWake() {
  if (!isConnected())
    return;

  // 唤醒信号 = 连点 N 次鼠标左键（任何 HID 输入都能唤醒睡眠中的电脑）。
  for (int i = 0; i < WAKE_CLICK_COUNT; i++) {
    _bleCombo->click(MOUSE_LEFT);
    delay(WAKE_CLICK_INTERVAL_MS);
  }

  Serial.printf("[BLE] wake signal sent (%d mouse left-clicks)\n",
                WAKE_CLICK_COUNT);
}

void BLEManager::sleep() {
  if (!isConnected())
    return;

  const uint8_t none[6] = {0, 0, 0, 0, 0, 0};
  uint8_t k[6] = {0, 0, 0, 0, 0, 0};

  // Control + Command + Q (lock screen)
  k[0] = 0x14; // 'q'
  sendKeyboardReport(k, 0x01 | 0x08); // Left Ctrl + Left GUI
  delay(60);
  sendKeyboardReport(none, 0);
  delay(100);

  // Escape (sleeps from the lock screen)
  k[0] = 0x29; // Escape
  sendKeyboardReport(k, 0);
  delay(60);
  sendKeyboardReport(none, 0);

  Serial.println("[BLE] mac sleep sequence sent (Ctrl+Cmd+Q, Esc)");
}

void BLEManager::startAdvertising() {
  if (_bleCombo != nullptr) {
    _bleCombo->startAdvertising();
    Serial.println("[BLE] advertising started");
  }
}

void BLEManager::startAdvertisingDirected(const uint8_t *mac) {
  if (_bleCombo != nullptr) {
    _bleCombo->startAdvertisingDirected(mac);
    Serial.println("[BLE] directed advertising started");
  }
}

void BLEManager::stopAdvertising() {
  if (_bleCombo != nullptr) {
    _bleCombo->stopAdvertising();
    Serial.println("[BLE] advertising stopped");
  }
}

const uint8_t *BLEManager::getPeerMac() {
  return (_bleCombo != nullptr) ? _bleCombo->getPeerMac() : nullptr;
}

bool BLEManager::hasPeerMac() {
  return (_bleCombo != nullptr) && _bleCombo->hasPeerMac();
}

void BLEManager::sendWindowsSleep() {
  if (_bleCombo == nullptr)
    return;
  _bleCombo->sleep(); // System Sleep key (0x82)
  Serial.println("[BLE] system sleep sent (Windows)");
}

void BLEManager::setUniqueMac(uint8_t slot) {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  // Change the last byte so each slot appears as a distinct device to hosts.
  mac[5] = (mac[5] & 0xF0) | (slot & 0x0F);

  esp_err_t err = esp_base_mac_addr_set(mac);
  if (err != ESP_OK) {
    Serial.printf("[BLE] Failed to set MAC: %s\n", esp_err_to_name(err));
  } else {
    Serial.printf("[BLE] Set MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],
                  mac[1], mac[2], mac[3], mac[4], mac[5]);
  }
}
