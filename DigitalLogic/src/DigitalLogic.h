/**
 * @file DigitalLogic.h
 * @brief Library for interfacing (ESP32) with Digital Logic ICs
 * @version 1.0.0
 * @date 2025-11-13
 * 
 * This library provides classes for controlling and interfacing with
 * digital logic ICs (74xx series, CD4xxx series) using ESP32.
 * 
 * Supported ICs:
 * - Logic Gates: 7400, 7404, 7408, 7432, 7486
 * - Counters: 7473, 7493
 * - Decoders: CD4511 (BCD to 7-segment)
 * 
 * @note operates at 3.3V logic level. Most TTL ICs (74xx) 
 *       are 5V tolerant but work with 3.3V inputs. Use level shifters
 *       if needed for reliability.
 */

#ifndef DIGITALLOGIC_H
#define DIGITALLOGIC_H

#include <Arduino.h>

/**
 * @enum LogicLevel
 * @brief Digital logic levels
 */
enum LogicLevel {
    LOGIC_LOW = LOW,    ///< Logic 0 (0V)
    LOGIC_HIGH = HIGH   ///< Logic 1 (3.3V for ESP32)
};

/**
 * @enum LogicGateType
 * @brief Types of logic gates
 */
enum LogicGateType {
    GATE_AND,   ///< AND gate (7408)
    GATE_OR,    ///< OR gate (7432)
    GATE_NOT,   ///< NOT gate (7404)
    GATE_NAND,  ///< NAND gate (7400)
    GATE_XOR    ///< XOR gate (7486)
};

/**
 * @class LogicGate
 * @brief Control and test logic gates using ESP32 GPIO
 * 
 * This class allows you to use ESP32 as a signal source to test
 * logic gates like 7400, 7404, 7408, 7432, 7486.
 */
class LogicGate {
private:
    int8_t _pinA;      ///< First input pin (or single input for NOT)
    int8_t _pinB;      ///< Second input pin (unused for NOT gate)
    LogicGateType _type;
    bool _stateA;
    bool _stateB;

public:
    /**
     * @brief Constructor for single-input gate (NOT)
     * @param pinA GPIO pin connected to gate input
     * @param type Gate type (should be GATE_NOT)
     */
    LogicGate(int pinA, LogicGateType type);

    /**
     * @brief Constructor for two-input gates (AND, OR, NAND, XOR)
     * @param pinA GPIO pin connected to first gate input
     * @param pinB GPIO pin connected to second gate input
     * @param type Gate type
     */
    LogicGate(int pinA, int pinB, LogicGateType type);

    /**
     * @brief Initialize GPIO pins
     */
    void begin();

    /**
     * @brief Set logic levels on gate inputs
     * @param a Logic level for input A
     * @param b Logic level for input B (ignored for NOT gate)
     */
    void setInputs(bool a, bool b = false);

    /**
     * @brief Set input A only
     * @param level Logic level (true = HIGH, false = LOW)
     */
    void setA(bool level);

    /**
     * @brief Set input B only
     * @param level Logic level (true = HIGH, false = LOW)
     */
    void setB(bool level);

    /**
     * @brief Get current state of input A
     * @return Current logic level of A
     */
    bool getA() const { return _stateA; }

    /**
     * @brief Get current state of input B
     * @return Current logic level of B
     */
    bool getB() const { return _stateB; }

    /**
     * @brief Calculate expected output based on gate type
     * @return Expected logic output
     */
    bool calculateOutput() const;
};

/**
 * @class ClockGenerator
 * @brief Generate clock pulses for counter ICs (7473, 7493)
 * 
 * This class generates square wave clock signals that can drive
 * counter ICs and other sequential logic circuits.
 */
class ClockGenerator {
private:
    int8_t _pin;
    unsigned long _frequency;    // Hz
    unsigned long _period;       // microseconds
    unsigned long _lastToggle;
    bool _state;
    bool _enabled;

public:
    /**
     * @brief Constructor
     * @param pin GPIO pin for clock output
     */
    ClockGenerator(int pin);

    /**
     * @brief Initialize clock generator
     * @param frequency Clock frequency in Hz (default: 1 Hz)
     */
    void begin(unsigned long frequency = 1);

    /**
     * @brief Set clock frequency
     * @param frequency Frequency in Hz (1 Hz to 1000 Hz recommended)
     */
    void setFrequency(unsigned long frequency);

    /**
     * @brief Get current frequency
     * @return Frequency in Hz
     */
    unsigned long getFrequency() const { return _frequency; }

    /**
     * @brief Start clock generation
     */
    void start();

    /**
     * @brief Stop clock generation
     */
    void stop();

    /**
     * @brief Check if clock is running
     * @return true if enabled, false otherwise
     */
    bool isRunning() const { return _enabled; }

    /**
     * @brief Generate single clock pulse
     */
    void pulse();

    /**
     * @brief Update clock state (call in loop())
     * Must be called frequently for accurate timing
     */
    void update();
};

/**
 * @class BCDEncoder
 * @brief Control CD4511 BCD-to-7-segment decoder
 * 
 * This class interfaces with CD4511 IC to display digits 0-9
 * on a 7-segment display.
 */
class BCDEncoder {
private:
    int8_t _pinA;  // LSB (bit 0)
    int8_t _pinB;  // bit 1
    int8_t _pinC;  // bit 2
    int8_t _pinD;  // MSB (bit 3)
    int8_t _pinLT; // Lamp Test (optional, -1 if not used)
    int8_t _pinBI; // Blanking Input (optional, -1 if not used)
    uint8_t _currentDigit;

public:
    /**
     * @brief Constructor with BCD pins only
     * @param pinA GPIO for BCD bit A (LSB)
     * @param pinB GPIO for BCD bit B
     * @param pinC GPIO for BCD bit C
     * @param pinD GPIO for BCD bit D (MSB)
     */
    BCDEncoder(int pinA, int pinB, int pinC, int pinD);

    /**
     * @brief Constructor with all control pins
     * @param pinA GPIO for BCD bit A (LSB)
     * @param pinB GPIO for BCD bit B
     * @param pinC GPIO for BCD bit C
     * @param pinD GPIO for BCD bit D (MSB)
     * @param pinLT GPIO for Lamp Test (active LOW)
     * @param pinBI GPIO for Blanking Input (active LOW)
     */
    BCDEncoder(int pinA, int pinB, int pinC, int pinD, int pinLT, int pinBI);

    /**
     * @brief Initialize BCD encoder pins
     */
    void begin();

    /**
     * @brief Display a digit (0-9) on 7-segment
     * @param digit Digit to display (0-9, values >9 show blank)
     */
    void displayDigit(uint8_t digit);

    /**
     * @brief Get currently displayed digit
     * @return Current digit value
     */
    uint8_t getCurrentDigit() const { return _currentDigit; }

    /**
     * @brief Clear display (blank)
     */
    void clear();

    /**
     * @brief Test all segments (if LT pin available)
     * Lights up all segments for testing
     */
    void lampTest();

    /**
     * @brief End lamp test, return to normal
     */
    void endLampTest();
};

/**
 * @class CounterController
 * @brief Control 7493/7473 counter ICs with RESET functionality
 * 
 * This class provides clock generation and reset control for
 * binary counter ICs.
 */
class CounterController {
private:
    ClockGenerator* _clock;
    int8_t _resetPin;
    bool _ownsClock;

public:
    /**
     * @brief Constructor with clock generator
     * @param clockPin GPIO pin for clock output
     * @param resetPin GPIO pin for counter reset
     */
    CounterController(int clockPin, int resetPin);

    /**
     * @brief Constructor with external clock generator
     * @param clockGen Pointer to existing ClockGenerator
     * @param resetPin GPIO pin for counter reset
     */
    CounterController(ClockGenerator* clockGen, int resetPin);

    /**
     * @brief Destructor
     */
    ~CounterController();

    /**
     * @brief Initialize counter controller
     * @param frequency Initial clock frequency in Hz
     */
    void begin(unsigned long frequency = 1);

    /**
     * @brief Reset the counter to 0
     * Sends a pulse to the reset pin
     */
    void reset();

    /**
     * @brief Start counting
     */
    void start();

    /**
     * @brief Stop counting
     */
    void stop();

    /**
     * @brief Generate single count pulse
     */
    void singleStep();

    /**
     * @brief Set count frequency
     * @param frequency Frequency in Hz
     */
    void setFrequency(unsigned long frequency);

    /**
     * @brief Get clock generator
     * @return Pointer to ClockGenerator
     */
    ClockGenerator* getClock() { return _clock; }

    /**
     * @brief Update clock (call in loop())
     */
    void update();
};

/**
 * @class SevenSegmentMultiplex
 * @brief Multiplex control for 2-digit or 4-digit 7-segment displays
 * 
 * This class handles time-multiplexed display of multiple 7-segment digits,
 * allowing control of 2 or 4 digits using only 7 segment pins + N digit control pins.
 * Uses non-blocking scanning for smooth display.
 */
class SevenSegmentMultiplex {
private:
    // Segment patterns for digits 0-9
    static const uint8_t SEGMENT_PATTERNS[10][7];
    
    int8_t _segmentPins[7];   // Pins for segments a-g
    int8_t* _digitPins;       // Pins for digit control
    uint8_t _numDigits;       // Number of digits (2 or 4)
    
    uint16_t _displayValue;   // Current value to display
    uint8_t _currentDigit;    // Currently scanning digit
    unsigned long _lastScan;  // Last scan time (microseconds)
    uint16_t _scanInterval;   // Microseconds between digit scans
    
    bool _leadingZeros;       // Show leading zeros
    bool _decimalPoint[4];    // Decimal point state for each digit
    int8_t _dpPin;            // Decimal point pin (if separate)
    
    bool _commonCathode;      // true = Common Cathode, false = Common Anode
    
    /**
     * @brief Display a single digit on current position
     * @param digit Digit to display (0-9, or -1 for blank)
     */
    void displayDigit(int8_t digit);
    
    /**
     * @brief Turn off all digits
     */
    void clearDisplay();

public:
    /**
     * @brief Constructor for 2-digit display
     * @param segA GPIO for segment A
     * @param segB GPIO for segment B
     * @param segC GPIO for segment C
     * @param segD GPIO for segment D
     * @param segE GPIO for segment E
     * @param segF GPIO for segment F
     * @param segG GPIO for segment G
     * @param dig1 GPIO for digit 1 control
     * @param dig2 GPIO for digit 2 control
     * @param commonCathode true for Common Cathode, false for Common Anode
     */
    SevenSegmentMultiplex(int segA, int segB, int segC, int segD, 
                          int segE, int segF, int segG,
                          int dig1, int dig2, 
                          bool commonCathode = true);
    
    /**
     * @brief Constructor for 4-digit display
     * @param segA GPIO for segment A
     * @param segB GPIO for segment B
     * @param segC GPIO for segment C
     * @param segD GPIO for segment D
     * @param segE GPIO for segment E
     * @param segF GPIO for segment F
     * @param segG GPIO for segment G
     * @param dig1 GPIO for digit 1 control
     * @param dig2 GPIO for digit 2 control
     * @param dig3 GPIO for digit 3 control
     * @param dig4 GPIO for digit 4 control
     * @param commonCathode true for Common Cathode, false for Common Anode
     */
    SevenSegmentMultiplex(int segA, int segB, int segC, int segD, 
                          int segE, int segF, int segG,
                          int dig1, int dig2, int dig3, int dig4,
                          bool commonCathode = true);
    
    /**
     * @brief Destructor
     */
    ~SevenSegmentMultiplex();
    
    /**
     * @brief Initialize display pins
     */
    void begin();
    
    /**
     * @brief Update display scanning (call in loop())
     * Must be called frequently for smooth display
     */
    void update();
    
    /**
     * @brief Set number to display
     * @param value Number to display (0-99 for 2-digit, 0-9999 for 4-digit)
     */
    void showNumber(uint16_t value);
    
    /**
     * @brief Get current displayed value
     * @return Current number being displayed
     */
    uint16_t getNumber() const { return _displayValue; }
    
    /**
     * @brief Enable/disable leading zeros
     * @param enable true to show leading zeros, false to blank them
     */
    void setLeadingZeros(bool enable);
    
    /**
     * @brief Set decimal point for a specific digit
     * @param digitIndex Digit index (0-3, left to right)
     * @param enable true to show decimal point
     */
    void setDecimalPoint(uint8_t digitIndex, bool enable);
    
    /**
     * @brief Clear all decimal points
     */
    void clearDecimalPoints();
    
    /**
     * @brief Set scan speed
     * @param intervalMicros Microseconds between digit scans (default 1000)
     */
    void setScanInterval(uint16_t intervalMicros);
    
    /**
     * @brief Clear display (all digits off)
     */
    void clear();
    
    /**
     * @brief Display time in HH:MM format (4-digit only)
     * @param hours Hours (0-23)
     * @param minutes Minutes (0-59)
     */
    void showTime(uint8_t hours, uint8_t minutes);
    
    /**
     * @brief Display temperature with decimal (e.g., 27.5)
     * @param temperature Temperature value
     * @param decimals Number of decimal places (0-2)
     */
    void showTemperature(float temperature, uint8_t decimals = 1);
};

#endif // DIGITALLOGIC_H
