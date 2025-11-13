#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

/**
 * TemperatureSensor
 * Wrapper class for DHT11/DHT22 temperature and humidity sensors.
 * Provides convenient methods for reading, status checking, and threshold management.
 * 
 * Hardware requirements:
 * - DHT11 or DHT22 sensor
 * - Connection: VCC → 3.3V, GND → GND, DATA → GPIO pin
 * 
 * Dependencies:
 * - DHT sensor library by Adafruit
 * - Adafruit Unified Sensor library
 * 
 * Install via Arduino Library Manager:
 * 1. Tools → Manage Libraries
 * 2. Search "DHT sensor library"
 * 3. Install both "DHT sensor library" and "Adafruit Unified Sensor"
 */
class TemperatureSensor {
  public:
    enum TempLevel {
      TEMP_COOL = 0,
      TEMP_WARM,
      TEMP_HOT
    };

  private:
    DHT* dht;
    uint8_t pin;
    uint8_t type;
    float lastTemp;
    float lastHumidity;
    bool lastReadValid;
    
    // Threshold values
    float tempHigh;
    float tempLow;

  public:
    /**
     * Constructor
     * @param dhtPin GPIO pin connected to DHT sensor DATA pin
     * @param dhtType Sensor type: DHT11 or DHT22
     */
    TemperatureSensor(uint8_t dhtPin, uint8_t dhtType = DHT11);

    /**
     * Destructor
     */
    ~TemperatureSensor();

    /**
     * Initialize sensor (call in setup())
     */
    void begin();

    /**
     * Read temperature and humidity
     * @return true if read successful, false if error
     */
    bool read();

    /**
     * Get last temperature reading in Celsius
     * @return Temperature in °C
     */
    float getTemperature() const;

    /**
     * Get last temperature reading in Fahrenheit
     * @return Temperature in °F
     */
    float getTemperatureF() const;

    /**
     * Get last humidity reading
     * @return Humidity in %RH
     */
    float getHumidity() const;

    /**
     * Check if last reading was valid
     * @return true if valid, false if error
     */
    bool isValid() const;

    /**
     * Set temperature thresholds for hysteresis control
     * @param high Temperature to turn ON (e.g., 30°C)
     * @param low Temperature to turn OFF (e.g., 28°C)
     */
    void setThresholds(float high, float low);

    /**
     * Check if temperature exceeds high threshold
     * @return true if temp >= high threshold
     */
    bool isAboveHighThreshold() const;

    /**
     * Check if temperature is below low threshold
     * @return true if temp <= low threshold
     */
    bool isBelowLowThreshold() const;

    /**
     * Get temperature level category
     * @param coolMax Maximum temp for COOL (default 28°C)
     * @param warmMax Maximum temp for WARM (default 32°C)
     * @return TempLevel enum (COOL, WARM, or HOT)
     */
    TempLevel getTemperatureLevel(float coolMax = 28.0, float warmMax = 32.0) const;

    /**
     * Get temperature level as string
     * @param coolMax Maximum temp for COOL
     * @param warmMax Maximum temp for WARM
     * @return "COOL", "WARM", or "HOT"
     */
    const char* getTemperatureLevelString(float coolMax = 28.0, float warmMax = 32.0) const;
};

#endif // TEMPERATURE_SENSOR_H
