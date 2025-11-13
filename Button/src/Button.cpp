#include "Button.h"

Button::Button(uint8_t pinNumber, bool enablePullup, unsigned long debounce)
: pin(pinNumber), usePullup(enablePullup), lastChangeTime(0), debounceMs(debounce), lastStableState(HIGH), currentState(HIGH), pressedEvent(false) {
}

void Button::begin() {
  if (usePullup) {
    pinMode(pin, INPUT_PULLUP);
    lastStableState = HIGH;
  } else {
    pinMode(pin, INPUT);
    lastStableState = LOW;
  }
  currentState = lastStableState;
}

void Button::update() {
  int reading = digitalRead(pin);
  unsigned long now = millis();
  pressedEvent = false;

  if (reading != currentState) {
    currentState = reading;
    lastChangeTime = now;
  }

  if (now - lastChangeTime > debounceMs) {
    if (lastStableState != currentState) {
      if (usePullup) {
        if (lastStableState == HIGH && currentState == LOW) {
          pressedEvent = true;
        }
      } else {
        if (lastStableState == LOW && currentState == HIGH) {
          pressedEvent = true;
        }
      }
      lastStableState = currentState;
    }
  }
}

bool Button::wasPressed() {
  return pressedEvent;
}

bool Button::isPressed() const {
  if (usePullup) return (lastStableState == LOW);
  else return (lastStableState == HIGH);
}

// ============================================================================
// AdvancedButton Implementation
// ============================================================================

AdvancedButton::AdvancedButton(
  uint8_t pinNumber,
  bool enablePullup,
  unsigned long debounce,
  unsigned long doubleClickWindow,
  unsigned long longPressThreshold
) : pin(pinNumber),
    usePullup(enablePullup),
    lastChangeTime(0),
    pressedTime(0),
    lastReleaseTime(0),
    stableState(HIGH),
    lastStableState(HIGH),
    waitingSecondClick(false),
    longPressFired(false),
    debounceMs(debounce),
    doubleClickGap(doubleClickWindow),
    longPressTime(longPressThreshold),
    singleClick(false),
    doubleClick(false),
    longPress(false),
    pressed(false),
    released(false) {
}

void AdvancedButton::begin() {
  if (usePullup) {
    pinMode(pin, INPUT_PULLUP);
    stableState = HIGH;
    lastStableState = HIGH;
  } else {
    pinMode(pin, INPUT);
    stableState = LOW;
    lastStableState = LOW;
  }
}

void AdvancedButton::update() {
  // Clear all event flags
  singleClick = false;
  doubleClick = false;
  longPress = false;
  pressed = false;
  released = false;
  
  int reading = digitalRead(pin);
  unsigned long now = millis();
  
  // Debounce logic
  if (reading != stableState && (now - lastChangeTime) > debounceMs) {
    lastChangeTime = now;
    stableState = reading;
    
    // Detect edge changes
    bool currentPressed = usePullup ? (stableState == LOW) : (stableState == HIGH);
    bool wasPressed = usePullup ? (lastStableState == LOW) : (lastStableState == HIGH);
    
    if (currentPressed && !wasPressed) {
      // Just pressed (rising edge of press)
      pressed = true;
      pressedTime = now;
      longPressFired = false;
      
    } else if (!currentPressed && wasPressed) {
      // Just released (falling edge of press)
      released = true;
      unsigned long pressDuration = now - pressedTime;
      
      // Check if it was a long press
      if (pressDuration >= longPressTime && !longPressFired) {
        longPress = true;
        longPressFired = true;
        waitingSecondClick = false; // Long press cancels click detection
      } else {
        // Short press - check for single/double click
        if (waitingSecondClick && (now - lastReleaseTime) <= doubleClickGap) {
          // Second click within gap = double click
          doubleClick = true;
          waitingSecondClick = false;
        } else {
          // Start waiting for potential second click
          waitingSecondClick = true;
        }
        lastReleaseTime = now;
      }
    }
    
    lastStableState = stableState;
  }
  
  // Check for long press while holding
  if (isPressed() && !longPressFired && (now - pressedTime) >= longPressTime) {
    longPress = true;
    longPressFired = true;
    waitingSecondClick = false;
  }
  
  // Timeout for single click detection
  if (waitingSecondClick && (now - lastReleaseTime) > doubleClickGap) {
    singleClick = true;
    waitingSecondClick = false;
  }
}

bool AdvancedButton::isPressed() const {
  return usePullup ? (stableState == LOW) : (stableState == HIGH);
}

unsigned long AdvancedButton::getPressedDuration() const {
  if (!isPressed()) return 0;
  return millis() - pressedTime;
}

void AdvancedButton::setDebounceTime(unsigned long ms) {
  debounceMs = ms;
}

void AdvancedButton::setDoubleClickWindow(unsigned long ms) {
  doubleClickGap = ms;
}

void AdvancedButton::setLongPressThreshold(unsigned long ms) {
  longPressTime = ms;
}
