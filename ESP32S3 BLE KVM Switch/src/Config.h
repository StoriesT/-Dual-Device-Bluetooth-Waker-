/**
 * @file Config.h
 * @brief Global configuration constants for the Dual-Device Bluetooth Waker.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// DEVICE SLOTS (one per target computer)
// ============================================================================

/** @brief Number of target devices (Windows + Mac) */
#define NUM_DEVICE_SLOTS 2

/** @brief Bluetooth display name for each slot */
#define DEVICE_NAME_1 "Waker-Win" // Slot 0 -> Windows
#define DEVICE_NAME_2 "Waker-Mac" // Slot 1 -> Mac

/** @brief Manufacturer name reported over BLE */
#define DEVICE_MANUFACTURER "ESP32-S3"

/** @brief Battery level reported over BLE (0-100) */
#define BATTERY_LEVEL 100

// ============================================================================
// WAKE KEY
// ============================================================================

/**
 * @brief Wake signal: number of mouse left-clicks to send.
 */
#define WAKE_CLICK_COUNT 5

/** @brief Gap (ms) between consecutive wake clicks. */
#define WAKE_CLICK_INTERVAL_MS 80

// ============================================================================
// GPIO ASSIGNMENTS
// ============================================================================

/** @brief Physical buttons (connect button between GPIO and GND, active LOW). */
#define BTN_WINDOWS_PIN 4
#define BTN_MAC_PIN 5

/** @brief LED feedback pin (-1 to disable). */
#define LED_FEEDBACK_PIN 2

// ============================================================================
// WI-FI
// ============================================================================

/** @brief Soft-AP used when no saved Wi-Fi is reachable (serves config page). */
#define AP_SSID "Waker-Config"
#define AP_PASSWORD "12345678" // WPA2 requires >= 8 chars

/** @brief mDNS hostname (access via http://waker.local in STA mode). */
#define MDNS_HOSTNAME "waker"

/** @brief How long to wait for a Wi-Fi connection before falling back to AP. */
#define STA_CONNECT_TIMEOUT_MS 15000

// ============================================================================
// BEMFA CLOUD (米家 integration via MQTT)
// ============================================================================

/** @brief Bemfa Cloud MQTT broker. */
#define BEMFA_HOST "bemfa.com"
#define BEMFA_PORT 9501

/** @brief Bemfa Cloud UID (私钥), used as the MQTT client ID. */
#define BEMFA_UID ""

/** @brief Device topics: switch "on" wakes the corresponding computer. */
#define BEMFA_TOPIC_WINDOWS ""
#define BEMFA_TOPIC_MAC ""

#endif // CONFIG_H
