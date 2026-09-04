/**
 * @file Controller.h
 * @brief Central state machine and command router.
 *
 * Every input source (buttons, cloud, web) funnels into a single
 * requestWake() entry point so that only this class decides when to switch
 * and wake a target device.
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>

/** @brief A target computer. */
enum class Target { WINDOWS = 0, MAC = 1 };

class Controller {
public:
  /** @brief Initializes all subsystems (buttons, BLE, WiFi, web, cloud). */
  static void begin();

  /** @brief Main loop: polls inputs and drives connection/state handling. */
  static void loop();

  /**
   * @brief Requests a switch-and-wake towards the given target.
   * Single entry point for buttons, voice and web inputs.
   */
  static void requestWake(Target t);

  /** @brief Requests a switch-and-sleep towards the given target. */
  static void requestSleep(Target t);

  /**
   * @brief Switches to the target slot and advertises for pairing (no wake key).
   */
  static void requestPair(Target t);

  /** @brief Current active slot (0 = Windows, 1 = Mac). */
  static uint8_t currentSlot();

private:
  static void onButton(uint8_t index);
  static void onButtonLongPress(uint8_t index);
  static void onCloud(uint8_t target, bool wake);
  static void handleBleConnection();
  static void blinkTimes(int n);
  static void switchToSlot(uint8_t slot, uint8_t pendingAction);
  static void doSleep(uint8_t slot);
  static void applyAdvertisingMode();
};

#endif // CONTROLLER_H
