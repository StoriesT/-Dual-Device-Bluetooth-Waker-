/**
 * @file BLEManager.h
 * @brief Manages Bluetooth Low Energy (BLE) HID keyboard functionality.
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BleCombo.h>

class BLEManager {
public:
  BLEManager();

  /**
   * @brief Initializes BLE for a specific slot (0-based) and starts
   * advertising under that slot's device name + unique MAC.
   */
  void begin(uint8_t slot);

  /** @brief True if a BLE host is currently connected. */
  bool isConnected();

  /** @brief Sends a raw HID keyboard report. */
  void sendKeyboardReport(const uint8_t *keys, uint8_t modifiers);

  /** @brief Sends the wake signal (mouse left-clicks) to the connected host. */
  void sendWake();

  /** @brief Sends the Mac sleep sequence (Ctrl+Cmd+Q, Esc) to the host. */
  void sleep();

  /** @brief Sends the System Sleep key (0x82) to the host (Windows). */
  void sendWindowsSleep();

  /** @brief Starts undirected BLE advertising (discoverable, for pairing). */
  void startAdvertising();

  /** @brief Starts directed advertising to a peer MAC (auto-reconnect, not discoverable). */
  void startAdvertisingDirected(const uint8_t *mac);

  /** @brief Stops BLE advertising (device becomes non-discoverable). */
  void stopAdvertising();

  /** @brief Returns the last connected peer MAC (or nullptr if none). */
  const uint8_t *getPeerMac();

  /** @brief True if a peer MAC has been captured. */
  bool hasPeerMac();

private:
  BleCombo *_bleCombo;

  /** @brief Derives a unique MAC from the base MAC + slot index. */
  void setUniqueMac(uint8_t slot);
};

#endif // BLE_MANAGER_H
