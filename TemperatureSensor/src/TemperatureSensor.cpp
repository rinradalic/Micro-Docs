#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(uint8_t dhtPin, uint8_t dhtType)
: pin(dhtPin), type(dhtType), lastTemp(0), lastHumidity(0), lastReadValid(false), tempHigh(30.0), tempLow(28.0) {
  dht = new DHT(pin, type);
}

TemperatureSensor::~TemperatureSensor() {
  delete dht;
}

void TemperatureSensor::begin() {
  dht->begin();
  delay(2000); // DHT sensors need time to stabilize
}

bool TemperatureSensor::read() {
  lastHumidity = dht->readHumidity();
  lastTemp = dht->readTemperature();
  
  if (isnan(lastHumidity) || isnan(lastTemp)) {
    lastReadValid = false;
    return false;
  }
  
  lastReadValid = true;
  return true;
}

float TemperatureSensor::getTemperature() const {
  return lastTemp;
}

float TemperatureSensor::getTemperatureF() const {
  return (lastTemp * 9.0 / 5.0) + 32.0;
}

float TemperatureSensor::getHumidity() const {
  return lastHumidity;
}

bool TemperatureSensor::isValid() const {
  return lastReadValid;
}

void TemperatureSensor::setThresholds(float high, float low) {
  tempHigh = high;
  tempLow = low;
}

bool TemperatureSensor::isAboveHighThreshold() const {
  return lastReadValid && (lastTemp >= tempHigh);
}

bool TemperatureSensor::isBelowLowThreshold() const {
  return lastReadValid && (lastTemp <= tempLow);
}

TemperatureSensor::TempLevel TemperatureSensor::getTemperatureLevel(float coolMax, float warmMax) const {
  if (lastTemp < coolMax) {
    return TEMP_COOL;
  } else if (lastTemp < warmMax) {
    return TEMP_WARM;
  } else {
    return TEMP_HOT;
  }
}

const char* TemperatureSensor::getTemperatureLevelString(float coolMax, float warmMax) const {
  switch (getTemperatureLevel(coolMax, warmMax)) {
    case TEMP_COOL: return "COOL";
    case TEMP_WARM: return "WARM";
    case TEMP_HOT: return "HOT";
    default: return "UNKNOWN";
  }
}
