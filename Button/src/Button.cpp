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
