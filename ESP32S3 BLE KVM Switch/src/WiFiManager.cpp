#include "WiFiManager.h"
#include "Config.h"
#include <Preferences.h>
#include <ESPmDNS.h>

bool WiFiManager::_staMode = false;
unsigned long WiFiManager::_staAttemptStartMs = 0;

namespace {
Preferences _prefs; // 存储 Wi-Fi 的 ssid / pass
}

void WiFiManager::begin() {
  String ssid, pass;
  loadCredentials(ssid, pass);

  if (ssid.length() == 0) {
    Serial.println("[WiFi] no saved credentials -> AP mode");
    startAp();
    return;
  }

  Serial.printf("[WiFi] connecting to '%s'...\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  _staMode = true;
  _staAttemptStartMs = millis();

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < STA_CONNECT_TIMEOUT_MS) {
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] connected, IP=%s\n",
                  WiFi.localIP().toString().c_str());
    startMdns();
  } else {
    Serial.println("[WiFi] connect failed -> AP mode");
    startAp();
  }
}

void WiFiManager::loop() {
  // Runtime reconnect failure -> fall back to AP so the config page stays
  // reachable (e.g. wrong password entered on the web form).
  if (_staMode && WiFi.status() != WL_CONNECTED &&
      millis() - _staAttemptStartMs > STA_CONNECT_TIMEOUT_MS) {
    Serial.println("[WiFi] STA timed out -> AP mode");
    startAp();
  }
}

void WiFiManager::saveCredentials(const String &ssid, const String &pass) {
  _prefs.begin("wifi", false);
  _prefs.putString("ssid", ssid);
  _prefs.putString("pass", pass);
  _prefs.end();

  Serial.printf("[WiFi] saved credentials, reconnecting to '%s'...\n",
                ssid.c_str());
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  _staMode = true;
  _staAttemptStartMs = millis();
}

void WiFiManager::loadCredentials(String &ssid, String &pass) {
  _prefs.begin("wifi", true);
  ssid = _prefs.getString("ssid", "");
  pass = _prefs.getString("pass", "");
  _prefs.end();
}

String WiFiManager::statusString() {
  if (!_staMode) {
    return "AP: " + WiFi.softAPIP().toString();
  }
  if (WiFi.status() == WL_CONNECTED) {
    return "STA: " + WiFi.localIP().toString() + " (" MDNS_HOSTNAME ".local)";
  }
  return "STA: connecting...";
}

void WiFiManager::startAp() {
  _staMode = false;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.printf("[WiFi] AP '%s' IP=%s\n", AP_SSID,
                WiFi.softAPIP().toString().c_str());
  startMdns();
}

void WiFiManager::startMdns() {
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[WiFi] mDNS: http://%s.local\n", MDNS_HOSTNAME);
  }
}
