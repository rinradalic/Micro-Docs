#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

/**
 * RelayController
 * Controls a relay module or transistor-driven relay circuit.
 * Suitable for controlling external loads via ESP32/Arduino GPIO.
 * 
 * Hardware requirements:
 * - Relay module (5V or 3.3V) OR
 * - NPN transistor (2N2222/BC547) + relay coil + 1N4007 flyback diode + 1kΩ resistor
 * 
 * Safety:
 * - For AC loads (220V), use opto-isolated relay modules
 * - Never connect 220V to breadboards
 * - Always use proper fuses
 * - Ensure common GND between MCU and relay circuit
 */
class RelayController {
  private:
    uint8_t pin;
    bool state;           // Current relay state (true = ON)
    bool inverted;        // Invert logic (for active-low modules)

  public:
    /**
     * Constructor
     * @param pinNumber GPIO pin connected to relay (or transistor base via resistor)
     * @param invertLogic Set true for active-low relay modules (default false)
     */
    RelayController(uint8_t pinNumber, bool invertLogic = false);

    /**
     * Initialize the relay pin (call in setup())
     */
    void begin();

    /**
     * Turn relay ON (energize coil, close NO contact)
     */
    void on();

    /**
     * Turn relay OFF (de-energize coil, open NO contact)
     */
    void off();

    /**
     * Toggle relay state
     */
    void toggle();

    /**
     * Get current relay state
     * @return true if relay is ON, false if OFF
     */
    bool isOn() const;

    /**
     * Set relay to specific state
     * @param newState true = ON, false = OFF
     */
    void setState(bool newState);
};

#endif // RELAY_CONTROLLER_H
