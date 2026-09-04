#include "CloudManager.h"
#include "Config.h"
#include <PubSubClient.h>
#include <WiFi.h>

CloudManager::CommandCallback CloudManager::_cb = nullptr;

namespace {
WiFiClient _wifiClient;                        // MQTT 用的 TCP 连接
PubSubClient _mqtt(_wifiClient);               // 巴法云 MQTT 客户端
unsigned long _lastReconnectMs = 0;            // 上次尝试重连的时间
constexpr unsigned long RECONNECT_INTERVAL_MS = 10000; // 断线后每 10 秒重连一次
} // namespace

void CloudManager::begin() {
  _mqtt.setServer(BEMFA_HOST, BEMFA_PORT);
  _mqtt.setCallback(onMqttMessage);
  Serial.printf("[Cloud] MQTT broker %s:%d\n", BEMFA_HOST, BEMFA_PORT);
}

void CloudManager::loop() {
  // Needs internet (STA mode) to reach the cloud.
  if (WiFi.status() != WL_CONNECTED)
    return;

  if (_mqtt.connected()) {
    _mqtt.loop();
    return;
  }

  if (millis() - _lastReconnectMs < RECONNECT_INTERVAL_MS)
    return;
  _lastReconnectMs = millis();

  Serial.print("[Cloud] connecting... ");
  if (_mqtt.connect(BEMFA_UID)) {
    Serial.println("ok");
    _mqtt.subscribe(BEMFA_TOPIC_WINDOWS);
    _mqtt.subscribe(BEMFA_TOPIC_MAC);
    Serial.printf("[Cloud] subscribed '%s' + '%s'\n", BEMFA_TOPIC_WINDOWS,
                  BEMFA_TOPIC_MAC);
  } else {
    Serial.printf("failed rc=%d\n", _mqtt.state());
  }
}

void CloudManager::onMqttMessage(char *topic, byte *payload,
                                 unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();

  Serial.printf("[Cloud] topic='%s' msg='%s'\n", topic, msg.c_str());

  uint8_t target;
  if (strcmp(topic, BEMFA_TOPIC_WINDOWS) == 0) {
    target = 0;
  } else if (strcmp(topic, BEMFA_TOPIC_MAC) == 0) {
    target = 1;
  } else {
    return;
  }

  bool wake;
  if (msg == "on") {
    wake = true;
  } else if (msg == "off") {
    wake = false;
  } else {
    return;
  }

  if (_cb)
    _cb(target, wake);
}
