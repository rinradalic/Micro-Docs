#include "RelayController.h"

RelayController::RelayController(uint8_t pinNumber, bool invertLogic)
: pin(pinNumber), state(false), inverted(invertLogic) {
}

void RelayController::begin() {
  pinMode(pin, OUTPUT);
  off(); // Start with relay off (safe default)
}

void RelayController::on() {
  state = true;
  if (inverted) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
}

void RelayController::off() {
  state = false;
  if (inverted) {
    digitalWrite(pin, HIGH);
  } else {
    digitalWrite(pin, LOW);
  }
}

void RelayController::toggle() {
  if (state) {
    off();
  } else {
    on();
  }
}

bool RelayController::isOn() const {
  return state;
}

void RelayController::setState(bool newState) {
  if (newState) {
    on();
  } else {
    off();
  }
}
