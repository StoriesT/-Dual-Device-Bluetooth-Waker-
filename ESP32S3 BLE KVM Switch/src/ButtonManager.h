/**
 * @file ButtonManager.h
 * @brief Non-blocking debounced scanning of the two physical buttons.
 */

#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>

class ButtonManager {
public:
  /** @brief Callback on a short press (fires on release). */
  typedef void (*PressCallback)(uint8_t index);

  /** @brief Callback on a long press (held >= 3 s). */
  typedef void (*LongPressCallback)(uint8_t index);

  /** @brief Initializes button pins as INPUT_PULLUP. */
  static void begin();

  /** @brief Non-blocking poll; call from the main loop. */
  static void tick();

  /** @brief Registers the short-press callback. */
  static void setCallback(PressCallback cb) { _cb = cb; }

  /** @brief Registers the long-press callback. */
  static void setLongPressCallback(LongPressCallback cb) { _longCb = cb; }

private:
  struct Btn {
    uint8_t pin;
    bool lastReading;
    bool lastStable;      // debounced state (true = released)
    unsigned long lastChangeMs;
    unsigned long pressStartMs;
    bool longPressFired;
  };

  static Btn _buttons[2];
  static PressCallback _cb;
  static LongPressCallback _longCb;
};

#endif // BUTTON_MANAGER_H
