#include "LEDController.h"

LEDController::LEDController(uint8_t pinNumber)
: pin(pinNumber), state(false), blinking(false), interval(500), lastToggle(0) {
}

void LEDController::begin() {
  pinMode(pin, OUTPUT);
  off();
}

void LEDController::on() {
  state = true;
  blinking = false;
  digitalWrite(pin, HIGH);
}

void LEDController::off() {
  state = false;
  blinking = false;
  digitalWrite(pin, LOW);
}

void LEDController::toggle() {
  state = !state;
  blinking = false;
  digitalWrite(pin, state ? HIGH : LOW);
}

void LEDController::startBlink(unsigned long ms) {
  interval = ms;
  blinking = true;
  lastToggle = millis();
}

void LEDController::stopBlink() {
  blinking = false;
  digitalWrite(pin, state ? HIGH : LOW);
}

void LEDController::update() {
  if (blinking) {
    unsigned long now = millis();
    if (now - lastToggle >= interval) {
      lastToggle = now;
      state = !state;
      digitalWrite(pin, state ? HIGH : LOW);
    }
  }
}

bool LEDController::isOn() const {
  return state;
}
