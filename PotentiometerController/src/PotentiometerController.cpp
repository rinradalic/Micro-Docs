#include "PotentiometerController.h"

PotentiometerController::PotentiometerController(uint8_t adcPin)
: pin(adcPin), rawValue(0), minRaw(0) {
  // Detect platform
  #if defined(ESP32)
    isESP32 = true;
    maxRaw = 4095; // ESP32 has 12-bit ADC
  #else
    isESP32 = false;
    maxRaw = 1023; // Most Arduino boards have 10-bit ADC
  #endif
}

void PotentiometerController::begin() {
  // ADC pins don't need pinMode on most platforms
  // This is here for API consistency
  #if defined(ESP32)
    // On ESP32, ADC pins are automatically configured
  #else
    pinMode(pin, INPUT);
  #endif
}

int PotentiometerController::read() {
  rawValue = analogRead(pin);
  return rawValue;
}

int PotentiometerController::readPercent() {
  read();
  return map(rawValue, minRaw, maxRaw, 0, 100);
}

int PotentiometerController::readMapped(int outMin, int outMax) {
  read();
  return map(rawValue, minRaw, maxRaw, outMin, outMax);
}

int PotentiometerController::getRaw() const {
  return rawValue;
}

int PotentiometerController::getMaxRaw() const {
  return maxRaw;
}
