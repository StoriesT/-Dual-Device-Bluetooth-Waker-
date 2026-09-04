/**
 * @file WiFiManager.h
 * @brief AP+STA hybrid networking: connect to saved home Wi-Fi, fall back to a
 * soft-AP so the configuration page is always reachable.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
  static void begin();
  static void loop();

  /** @brief Persists credentials and reconnects as a station. */
  static void saveCredentials(const String &ssid, const String &pass);
  static void loadCredentials(String &ssid, String &pass);

  /** @brief Human-readable status (IP / AP) for the web UI. */
  static String statusString();

private:
  static bool _staMode;
  static unsigned long _staAttemptStartMs;

  static void startAp();
  static void startMdns();
};

#endif // WIFI_MANAGER_H
