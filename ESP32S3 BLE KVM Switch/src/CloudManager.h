/**
 * @file CloudManager.h
 * @brief Bemfa Cloud (巴法云) MQTT integration for 米家 voice/app control.
 */

#ifndef CLOUD_MANAGER_H
#define CLOUD_MANAGER_H

#include <Arduino.h>

class CloudManager {
public:
  /** @brief Callback with target index (0 = Windows, 1 = Mac) and wake flag. */
  typedef void (*CommandCallback)(uint8_t target, bool wake);

  static void begin();
  static void loop();
  static void setCallback(CommandCallback cb) { _cb = cb; }

private:
  static CommandCallback _cb;
  static void onMqttMessage(char *topic, byte *payload, unsigned int length);
};

#endif // CLOUD_MANAGER_H
