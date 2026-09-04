#include "Controller.h"
#include "BLEManager.h"
#include "ButtonManager.h"
#include "Config.h"
#include "NVSUtils.h"
#include "WebManager.h"
#include "WiFiManager.h"
#include "CloudManager.h"
#include <Preferences.h>

namespace {
Preferences _preferences;      // NVS 存储：slot / pending / 各槽位对端 MAC
BLEManager _bleManager;        // BLE HID 键盘
uint8_t _currentSlot = 0;      // 当前槽位（0 = Windows，1 = Mac）
uint8_t _pendingAction = 0;    // 重连成功后要执行的动作（见下方 ACTION_*）
bool _wasConnected = false;    // 上一轮连接状态，用于边沿检测（刚连上 / 刚断开）

// 待执行动作码（存入 NVS "pending"，跨重启保持）：
constexpr uint8_t ACTION_NONE = 0;  // 无待执行动作
constexpr uint8_t ACTION_WAKE = 1;  // 重连后发送唤醒
constexpr uint8_t ACTION_SLEEP = 2; // 重连后发送睡眠
constexpr uint8_t ACTION_PAIR = 3;  // 仅广播等待配对，不发送动作
} // namespace

void Controller::begin() {
  if (LED_FEEDBACK_PIN >= 0) {
    pinMode(LED_FEEDBACK_PIN, OUTPUT);
    digitalWrite(LED_FEEDBACK_PIN, LOW);
  }

  // 1. Restore saved slot and pending-wake flag.
  _preferences.begin("waker", false);
  _currentSlot = _preferences.getUChar("slot", 0);
  if (_currentSlot >= NUM_DEVICE_SLOTS)
    _currentSlot = 0;
  _pendingAction = _preferences.getUChar("pending", 0);
  _preferences.end();
  Serial.printf("[Config] starting on slot %d\n", _currentSlot + 1);

  // 2. Load BLE bonds for this slot, then init BLE.
  NVSUtils::loadSlotBonds(_currentSlot);
  _bleManager.begin(_currentSlot);
  applyAdvertisingMode();

  // 3. Input sources.
  ButtonManager::setCallback(onButton);
  ButtonManager::setLongPressCallback(onButtonLongPress);
  ButtonManager::begin();

  // 4. Network + web UI.
  WiFiManager::begin();
  WebManager::begin();

  // 5. Cloud (Bemfa / 米家 MQTT).
  CloudManager::setCallback(onCloud);
  CloudManager::begin();

  Serial.println("[Controller] ready");
}

void Controller::loop() {
  ButtonManager::tick();
  handleBleConnection();
  WiFiManager::loop();
  WebManager::loop();
  CloudManager::loop();
}

void Controller::requestWake(Target t) {
  uint8_t slot = (uint8_t)t;
  Serial.printf("[Wake] request -> %s\n", slot == 0 ? "Windows" : "Mac");

  // Already on the target and connected: just send the wake signal.
  if (slot == _currentSlot && _bleManager.isConnected()) {
    Serial.println("[Wake] already connected, sending wake signal");
    _bleManager.sendWake();
    blinkTimes(slot + 1);
    return;
  }

  // Otherwise switch slot and send the wake signal once reconnected.
  switchToSlot(slot, ACTION_WAKE);
}

void Controller::requestSleep(Target t) {
  uint8_t slot = (uint8_t)t;
  Serial.printf("[Sleep] request -> %s\n", slot == 0 ? "Windows" : "Mac");

  // Already on the target and connected: just send the sleep key.
  if (slot == _currentSlot && _bleManager.isConnected()) {
    Serial.println("[Sleep] already connected, sending sleep key");
    doSleep(slot);
    blinkTimes(slot + 1);
    return;
  }

  // Otherwise switch slot and send the sleep key once reconnected.
  switchToSlot(slot, ACTION_SLEEP);
}

void Controller::requestPair(Target t) {
  uint8_t slot = (uint8_t)t;
  Serial.printf("[Pair] request -> %s\n", slot == 0 ? "Windows" : "Mac");

  if (slot == _currentSlot) {
    Serial.println("[Pair] starting advertising on current slot");
    _bleManager.startAdvertising();
    blinkTimes(slot + 1);
    return;
  }

  // Switch slot and advertise for pairing.
  switchToSlot(slot, ACTION_PAIR);
}

// 切换槽位：换 MAC + 换绑定必须重启才能重新初始化 BLE 栈。
// 先存当前槽位绑定，再写新槽位号 + 待执行动作，然后重启重连。
void Controller::switchToSlot(uint8_t slot, uint8_t pendingAction) {
  NVSUtils::saveSlotBonds(_currentSlot);
  _preferences.begin("waker", false);
  _preferences.putUChar("slot", slot);
  _preferences.putUChar("pending", pendingAction);
  _preferences.end();

  Serial.printf("[System] switching slot %d -> %d, restarting...\n",
                _currentSlot + 1, slot + 1);
  blinkTimes(slot + 1);
  delay(300);
  ESP.restart();
}

uint8_t Controller::currentSlot() {
  return _currentSlot;
}

void Controller::onButton(uint8_t index) {
  requestWake(index == 0 ? Target::WINDOWS : Target::MAC);
}

void Controller::onButtonLongPress(uint8_t index) {
  requestPair(index == 0 ? Target::WINDOWS : Target::MAC);
}

void Controller::onCloud(uint8_t target, bool wake) {
  Target t = (target == 0) ? Target::WINDOWS : Target::MAC;
  if (wake)
    requestWake(t);
  else
    requestSleep(t);
}

void Controller::doSleep(uint8_t slot) {
  if (slot == 0) {
    _bleManager.sendWindowsSleep(); // Windows: Win+X, U, S
  } else {
    _bleManager.sleep(); // Mac: Ctrl+Cmd+Q, Esc
  }
}

void Controller::applyAdvertisingMode() {
  // Read the saved peer MAC for this slot.
  uint8_t mac[6];
  bool hasMac = false;
  _preferences.begin("waker", true);
  char key[16];
  snprintf(key, sizeof(key), "pmac%d", _currentSlot);
  hasMac = (_preferences.getBytes(key, mac, 6) == 6);
  _preferences.end();

  if (_pendingAction == ACTION_PAIR) {
    _bleManager.startAdvertising(); // pairing: undirected, discoverable
  } else if (_pendingAction != ACTION_NONE) {
    _bleManager.startAdvertising(); // wake/sleep reconnect: undirected (reliable)
  } else if (hasMac) {
    _bleManager.startAdvertisingDirected(mac); // idle: directed auto-reconnect
  } else {
    _bleManager.stopAdvertising(); // idle, no peer
  }
}

void Controller::handleBleConnection() {
  bool connected = _bleManager.isConnected();

  if (connected && !_wasConnected) {
    Serial.printf("[BLE] connected on slot %d\n", _currentSlot + 1);

    // Remember the peer MAC for future directed advertising (auto-reconnect).
    if (_bleManager.hasPeerMac()) {
      const uint8_t *mac = _bleManager.getPeerMac();
      _preferences.begin("waker", false);
      char key[16];
      snprintf(key, sizeof(key), "pmac%d", _currentSlot);
      _preferences.putBytes(key, mac, 6);
      _preferences.end();
    }

    if (_pendingAction != ACTION_NONE) {
      uint8_t action = _pendingAction;
      _pendingAction = ACTION_NONE;
      _preferences.begin("waker", false);
      _preferences.putUChar("pending", ACTION_NONE);
      _preferences.end();

      delay(200); // let the connection settle before sending
      if (action == ACTION_WAKE) {
        _bleManager.sendWake();
      } else if (action == ACTION_SLEEP) {
        doSleep(_currentSlot);
      } else if (action == ACTION_PAIR) {
        Serial.println("[Pair] paired");
      }
      blinkTimes(3);
      _bleManager.stopAdvertising(); // done: stay connected but quiet
    }
  } else if (!connected && _wasConnected) {
    Serial.println("[BLE] disconnected");
  }

  _wasConnected = connected;
}

void Controller::blinkTimes(int n) {
  if (LED_FEEDBACK_PIN < 0)
    return;
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_FEEDBACK_PIN, HIGH);
    delay(120);
    digitalWrite(LED_FEEDBACK_PIN, LOW);
    delay(120);
  }
}
