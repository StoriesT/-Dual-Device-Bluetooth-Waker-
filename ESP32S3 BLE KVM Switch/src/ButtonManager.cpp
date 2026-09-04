#include "ButtonManager.h"
#include "Config.h"

ButtonManager::Btn ButtonManager::_buttons[2];
ButtonManager::PressCallback ButtonManager::_cb = nullptr;
ButtonManager::LongPressCallback ButtonManager::_longCb = nullptr;

namespace {
constexpr unsigned long DEBOUNCE_MS = 30;
constexpr unsigned long LONG_PRESS_MS = 3000;
} // namespace

void ButtonManager::begin() {
  _buttons[0] = {BTN_WINDOWS_PIN, true, true, 0, 0, false};
  _buttons[1] = {BTN_MAC_PIN, true, true, 0, 0, false};

  for (auto &b : _buttons) {
    pinMode(b.pin, INPUT_PULLUP);
  }
}

void ButtonManager::tick() {
  for (uint8_t i = 0; i < 2; i++) {
    Btn &b = _buttons[i];
    bool reading = digitalRead(b.pin);

    if (reading != b.lastReading) {
      b.lastReading = reading;
      b.lastChangeMs = millis();
    }

    if (millis() - b.lastChangeMs < DEBOUNCE_MS)
      continue;

    bool debounced = b.lastReading;
    if (debounced != b.lastStable) {
      b.lastStable = debounced;
      if (debounced == LOW) {
        // Pressed: start the long-press timer.
        b.pressStartMs = millis();
        b.longPressFired = false;
      } else {
        // Released: short press (only if long press didn't already fire).
        if (!b.longPressFired && _cb)
          _cb(i);
      }
    }

    // Long press while held.
    if (b.lastStable == LOW && !b.longPressFired &&
        millis() - b.pressStartMs >= LONG_PRESS_MS) {
      b.longPressFired = true;
      if (_longCb)
        _longCb(i);
    }
  }
}
