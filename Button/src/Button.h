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

/**
 * AdvancedButton
 * Advanced button with Single Click, Double Click, and Long Press detection.
 * Uses millis()-based timing for non-blocking operation.
 * 
 * : Interrupts + Advanced Button Control
 */
class AdvancedButton {
  private:
    uint8_t pin;
    bool usePullup;
    
    // Timing
    unsigned long lastChangeTime;
    unsigned long pressedTime;
    unsigned long lastReleaseTime;
    
    // State tracking
    bool stableState;
    bool lastStableState;
    bool waitingSecondClick;
    bool longPressFired;
    
    // Configuration (ms)
    unsigned long debounceMs;
    unsigned long doubleClickGap;
    unsigned long longPressTime;
    
  public:
    // Event flags (cleared after each update())
    bool singleClick;
    bool doubleClick;
    bool longPress;
    bool pressed;    // fires once on press
    bool released;   // fires once on release
    
    /**
     * Constructor
     * @param pinNumber GPIO pin number
     * @param enablePullup Use internal pull-up resistor (default true)
     * @param debounce Debounce time in ms (default 40)
     * @param doubleClickWindow Max time between clicks for double-click in ms (default 300)
     * @param longPressThreshold Time to hold for long press in ms (default 800)
     */
    AdvancedButton(
      uint8_t pinNumber, 
      bool enablePullup = true,
      unsigned long debounce = 40,
      unsigned long doubleClickWindow = 300,
      unsigned long longPressThreshold = 800
    );
    
    // Initialize pin mode; call in setup()
    void begin();
    
    // Call in loop() to update state and detect events
    void update();
    
    // Returns current stable pressed state
    bool isPressed() const;
    
    // Get time button has been held (ms), returns 0 if not pressed
    unsigned long getPressedDuration() const;
    
    // Configuration setters
    void setDebounceTime(unsigned long ms);
    void setDoubleClickWindow(unsigned long ms);
    void setLongPressThreshold(unsigned long ms);
};

#endif // BUTTON_H
