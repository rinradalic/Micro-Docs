#ifndef POTENTIOMETER_CONTROLLER_H
#define POTENTIOMETER_CONTROLLER_H

#include <Arduino.h>

/**
 * PotentiometerController
 * Reads analog values from a potentiometer (VR) connected to ESP32/Arduino ADC pin.
 * Provides convenient methods for reading, mapping, and converting analog values.
 * 
 * Hardware connection (ESP32):
 * - VR Pin 1 → 3.3V (NOT 5V on ESP32!)
 * - VR Pin 2 (Wiper/Center) → GPIO ADC pin (34, 35, 36, 39, etc.)
 * - VR Pin 3 → GND
 * 
 * Theory: Voltage Divider
 * When you rotate the VR, the voltage at the wiper (center pin) changes
 * from 0V to 3.3V, which is read by the ADC as 0-4095 (12-bit on ESP32)
 * or 0-1023 (10-bit on most Arduino boards).
 */
class PotentiometerController {
  private:
    uint8_t pin;
    int rawValue;
    int minRaw;
    int maxRaw;
    bool isESP32;

  public:
    /**
     * Constructor
     * @param adcPin ADC-capable GPIO pin (e.g., 34, 35, 36, 39 on ESP32)
     */
    PotentiometerController(uint8_t adcPin);

    /**
     * Initialize the potentiometer (optional - ADC pins don't need pinMode)
     */
    void begin();

    /**
     * Read raw ADC value
     * @return Raw ADC value (0-4095 on ESP32, 0-1023 on Arduino)
     */
    int read();

    /**
     * Read and return as percentage (0-100%)
     * @return Percentage value 0-100
     */
    int readPercent();

    /**
     * Read and map to custom range
     * @param outMin Minimum output value
     * @param outMax Maximum output value
     * @return Mapped value
     */
    int readMapped(int outMin, int outMax);

    /**
     * Get last raw value (without reading again)
     * @return Last raw ADC value
     */
    int getRaw() const;

    /**
     * Get ADC resolution (4095 for ESP32, 1023 for Arduino)
     * @return Maximum ADC value
     */
    int getMaxRaw() const;
};

#endif // POTENTIOMETER_CONTROLLER_H
