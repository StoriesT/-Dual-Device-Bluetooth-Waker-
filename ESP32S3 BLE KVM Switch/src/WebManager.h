/**
 * @file WebManager.h
 * @brief Web UI + HTTP API (wake buttons and Wi-Fi config form).
 */

#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>

class WebManager {
public:
  static void begin();
  static void loop();

private:
  static void handleRoot();
  static void handleWake();
  static void handleSleep();
  static void handlePair();
  static void handleConfig();
  static void handleStatus();
};

#endif // WEB_MANAGER_H
