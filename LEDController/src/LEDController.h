#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

/**
 * LEDController
 * A small, single-pin LED controller that supports on/off/toggle and blinking.
 * Works on Arduino and ESP32.
 */
class LEDController {
  private:
    uint8_t pin;
    bool state;               // current ON/OFF
    bool blinking;            // whether blinking mode is active
    unsigned long interval;   // blink interval in ms
    unsigned long lastToggle; // last toggle timestamp

  public:
    // Create controller for a given pin
    explicit LEDController(uint8_t pinNumber);

    // Initialize hardware (call in setup())
    void begin();

    // Turn LED on (stops blinking)
    void on();

    // Turn LED off (stops blinking)
    void off();

    // Toggle the LED (stops blinking)
    void toggle();

    // Start blinking with given interval in milliseconds
    void startBlink(unsigned long ms);

    // Stop blinking, preserve current state
    void stopBlink();

    // Must be called regularly in loop() to handle blinking
    void update();

    // Query whether LED is currently on
    bool isOn() const;
};

#endif // LED_CONTROLLER_H
