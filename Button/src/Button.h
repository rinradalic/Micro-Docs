#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/**
 * Button
 * Debounced button helper with optional internal pull-up support.
 */
class Button {
  private:
    uint8_t pin;
    bool usePullup;
    unsigned long lastChangeTime;
    unsigned long debounceMs;
    bool lastStableState;
    bool currentState;
    bool pressedEvent;

  public:
    // Constructor: pin number, enable internal pullup (default true), debounce ms (default 50)
    Button(uint8_t pinNumber, bool enablePullup = true, unsigned long debounce = 50);

    // Initialize pin mode; call in setup()
    void begin();

    // Call in loop() to update internal state
    void update();

    // Returns true once immediately after a debounced press is detected
    bool wasPressed();

    // Returns the stable pressed state (true when pressed)
    bool isPressed() const;
};

#endif // BUTTON_H
