/**
 * @file DigitalLogic.cpp
 * @brief Implementation of DigitalLogic library
 */

#include "DigitalLogic.h"

// ============================================================================
// LogicGate Implementation
// ============================================================================

LogicGate::LogicGate(int pinA, LogicGateType type)
    : _pinA(pinA), _pinB(-1), _type(type), _stateA(false), _stateB(false) {
    // Constructor for single-input gates (NOT)
}

LogicGate::LogicGate(int pinA, int pinB, LogicGateType type)
    : _pinA(pinA), _pinB(pinB), _type(type), _stateA(false), _stateB(false) {
    // Constructor for two-input gates
}

void LogicGate::begin() {
    pinMode(_pinA, OUTPUT);
    digitalWrite(_pinA, LOW);
    _stateA = false;

    if (_pinB >= 0) {
        pinMode(_pinB, OUTPUT);
        digitalWrite(_pinB, LOW);
        _stateB = false;
    }
}

void LogicGate::setInputs(bool a, bool b) {
    _stateA = a;
    digitalWrite(_pinA, a ? HIGH : LOW);

    if (_pinB >= 0) {
        _stateB = b;
        digitalWrite(_pinB, b ? HIGH : LOW);
    }
}

void LogicGate::setA(bool level) {
    _stateA = level;
    digitalWrite(_pinA, level ? HIGH : LOW);
}

void LogicGate::setB(bool level) {
    if (_pinB >= 0) {
        _stateB = level;
        digitalWrite(_pinB, level ? HIGH : LOW);
    }
}

bool LogicGate::calculateOutput() const {
    switch (_type) {
        case GATE_AND:
            return _stateA && _stateB;
        
        case GATE_OR:
            return _stateA || _stateB;
        
        case GATE_NOT:
            return !_stateA;
        
        case GATE_NAND:
            return !(_stateA && _stateB);
        
        case GATE_XOR:
            return _stateA != _stateB;
        
        default:
            return false;
    }
}

// ============================================================================
// ClockGenerator Implementation
// ============================================================================

ClockGenerator::ClockGenerator(int pin)
    : _pin(pin), _frequency(1), _period(1000000), 
      _lastToggle(0), _state(false), _enabled(false) {
}

void ClockGenerator::begin(unsigned long frequency) {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    _state = false;
    setFrequency(frequency);
}

void ClockGenerator::setFrequency(unsigned long frequency) {
    if (frequency == 0) frequency = 1;
    if (frequency > 1000) frequency = 1000; // Cap at 1kHz for reliability
    
    _frequency = frequency;
    // Period is time for one complete cycle (HIGH + LOW)
    // Half period is time for each state
    _period = 1000000UL / frequency; // Full period in microseconds
}

void ClockGenerator::start() {
    _enabled = true;
    _lastToggle = micros();
    digitalWrite(_pin, LOW);
    _state = false;
}

void ClockGenerator::stop() {
    _enabled = false;
    digitalWrite(_pin, LOW);
    _state = false;
}

void ClockGenerator::pulse() {
    digitalWrite(_pin, HIGH);
    delayMicroseconds(100); // Short high pulse
    digitalWrite(_pin, LOW);
    _state = false;
}

void ClockGenerator::update() {
    if (!_enabled) return;

    unsigned long currentTime = micros();
    unsigned long halfPeriod = _period / 2;

    if (currentTime - _lastToggle >= halfPeriod) {
        _state = !_state;
        digitalWrite(_pin, _state ? HIGH : LOW);
        _lastToggle = currentTime;
    }
}

// ============================================================================
// BCDEncoder Implementation
// ============================================================================

BCDEncoder::BCDEncoder(int pinA, int pinB, int pinC, int pinD)
    : _pinA(pinA), _pinB(pinB), _pinC(pinC), _pinD(pinD),
      _pinLT(-1), _pinBI(-1), _currentDigit(0) {
}

BCDEncoder::BCDEncoder(int pinA, int pinB, int pinC, int pinD, int pinLT, int pinBI)
    : _pinA(pinA), _pinB(pinB), _pinC(pinC), _pinD(pinD),
      _pinLT(pinLT), _pinBI(pinBI), _currentDigit(0) {
}

void BCDEncoder::begin() {
    pinMode(_pinA, OUTPUT);
    pinMode(_pinB, OUTPUT);
    pinMode(_pinC, OUTPUT);
    pinMode(_pinD, OUTPUT);

    // Initialize all BCD pins to LOW (digit 0)
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, LOW);
    digitalWrite(_pinC, LOW);
    digitalWrite(_pinD, LOW);

    // Initialize control pins if available
    if (_pinLT >= 0) {
        pinMode(_pinLT, OUTPUT);
        digitalWrite(_pinLT, HIGH); // Lamp test inactive (active LOW)
    }

    if (_pinBI >= 0) {
        pinMode(_pinBI, OUTPUT);
        digitalWrite(_pinBI, HIGH); // Blanking inactive (active LOW)
    }

    _currentDigit = 0;
}

void BCDEncoder::displayDigit(uint8_t digit) {
    if (digit > 15) digit = 15; // CD4511 supports 0-15, but >9 shows blank/invalid
    
    _currentDigit = digit;

    // Output BCD value to pins
    digitalWrite(_pinA, (digit & 0x01) ? HIGH : LOW);  // Bit 0 (LSB)
    digitalWrite(_pinB, (digit & 0x02) ? HIGH : LOW);  // Bit 1
    digitalWrite(_pinC, (digit & 0x04) ? HIGH : LOW);  // Bit 2
    digitalWrite(_pinD, (digit & 0x08) ? HIGH : LOW);  // Bit 3 (MSB)
}

void BCDEncoder::clear() {
    // Method 1: Display invalid BCD value (>9 shows blank on CD4511)
    displayDigit(15);
    
    // Method 2: If BI pin available, use blanking
    if (_pinBI >= 0) {
        digitalWrite(_pinBI, LOW); // Active LOW blanking
    }
}

void BCDEncoder::lampTest() {
    if (_pinLT >= 0) {
        digitalWrite(_pinLT, LOW); // Active LOW lamp test
    }
}

void BCDEncoder::endLampTest() {
    if (_pinLT >= 0) {
        digitalWrite(_pinLT, HIGH); // Deactivate lamp test
    }
}

// ============================================================================
// CounterController Implementation
// ============================================================================

CounterController::CounterController(int clockPin, int resetPin)
    : _resetPin(resetPin), _ownsClock(true) {
    _clock = new ClockGenerator(clockPin);
}

CounterController::CounterController(ClockGenerator* clockGen, int resetPin)
    : _clock(clockGen), _resetPin(resetPin), _ownsClock(false) {
}

CounterController::~CounterController() {
    if (_ownsClock && _clock != nullptr) {
        delete _clock;
    }
}

void CounterController::begin(unsigned long frequency) {
    _clock->begin(frequency);
    
    pinMode(_resetPin, OUTPUT);
    digitalWrite(_resetPin, LOW); // Reset inactive
}

void CounterController::reset() {
    // 7493 reset requires R0(1) AND R0(2) to be HIGH
    // Most common connection: both reset pins tied together
    digitalWrite(_resetPin, HIGH);
    delay(10); // Hold reset for 10ms
    digitalWrite(_resetPin, LOW);
}

void CounterController::start() {
    _clock->start();
}

void CounterController::stop() {
    _clock->stop();
}

void CounterController::singleStep() {
    _clock->pulse();
}

void CounterController::setFrequency(unsigned long frequency) {
    _clock->setFrequency(frequency);
}

void CounterController::update() {
    _clock->update();
}

// ============================================================================
// SevenSegmentMultiplex Implementation
// ============================================================================

// Segment patterns: [a, b, c, d, e, f, g]
// 1 = segment ON, 0 = segment OFF
const uint8_t SevenSegmentMultiplex::SEGMENT_PATTERNS[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

SevenSegmentMultiplex::SevenSegmentMultiplex(int segA, int segB, int segC, int segD,
                                             int segE, int segF, int segG,
                                             int dig1, int dig2,
                                             bool commonCathode)
    : _numDigits(2), _displayValue(0), _currentDigit(0), _lastScan(0),
      _scanInterval(1000), _leadingZeros(false), _dpPin(-1), _commonCathode(commonCathode) {
    
    _segmentPins[0] = segA;
    _segmentPins[1] = segB;
    _segmentPins[2] = segC;
    _segmentPins[3] = segD;
    _segmentPins[4] = segE;
    _segmentPins[5] = segF;
    _segmentPins[6] = segG;
    
    _digitPins = new int8_t[2];
    _digitPins[0] = dig1;
    _digitPins[1] = dig2;
    
    for (int i = 0; i < 4; i++) {
        _decimalPoint[i] = false;
    }
}

SevenSegmentMultiplex::SevenSegmentMultiplex(int segA, int segB, int segC, int segD,
                                             int segE, int segF, int segG,
                                             int dig1, int dig2, int dig3, int dig4,
                                             bool commonCathode)
    : _numDigits(4), _displayValue(0), _currentDigit(0), _lastScan(0),
      _scanInterval(1000), _leadingZeros(false), _dpPin(-1), _commonCathode(commonCathode) {
    
    _segmentPins[0] = segA;
    _segmentPins[1] = segB;
    _segmentPins[2] = segC;
    _segmentPins[3] = segD;
    _segmentPins[4] = segE;
    _segmentPins[5] = segF;
    _segmentPins[6] = segG;
    
    _digitPins = new int8_t[4];
    _digitPins[0] = dig1;
    _digitPins[1] = dig2;
    _digitPins[2] = dig3;
    _digitPins[3] = dig4;
    
    for (int i = 0; i < 4; i++) {
        _decimalPoint[i] = false;
    }
}

SevenSegmentMultiplex::~SevenSegmentMultiplex() {
    delete[] _digitPins;
}

void SevenSegmentMultiplex::begin() {
    // Initialize segment pins
    for (int i = 0; i < 7; i++) {
        pinMode(_segmentPins[i], OUTPUT);
        digitalWrite(_segmentPins[i], LOW);
    }
    
    // Initialize digit control pins
    for (int i = 0; i < _numDigits; i++) {
        pinMode(_digitPins[i], OUTPUT);
        // Turn off all digits initially
        digitalWrite(_digitPins[i], _commonCathode ? HIGH : LOW);
    }
}

void SevenSegmentMultiplex::clearDisplay() {
    // Turn off all digits
    for (int i = 0; i < _numDigits; i++) {
        digitalWrite(_digitPins[i], _commonCathode ? HIGH : LOW);
    }
    
    // Turn off all segments
    for (int i = 0; i < 7; i++) {
        digitalWrite(_segmentPins[i], LOW);
    }
}

void SevenSegmentMultiplex::displayDigit(int8_t digit) {
    // Turn off all digits first
    for (int i = 0; i < _numDigits; i++) {
        digitalWrite(_digitPins[i], _commonCathode ? HIGH : LOW);
    }
    
    // If digit is -1 (blank), keep segments off
    if (digit < 0 || digit > 9) {
        for (int i = 0; i < 7; i++) {
            digitalWrite(_segmentPins[i], LOW);
        }
        return;
    }
    
    // Set segment pattern for this digit
    for (int i = 0; i < 7; i++) {
        bool segmentOn = SEGMENT_PATTERNS[digit][i];
        digitalWrite(_segmentPins[i], segmentOn ? HIGH : LOW);
    }
    
    // Turn on current digit
    digitalWrite(_digitPins[_currentDigit], _commonCathode ? LOW : HIGH);
}

void SevenSegmentMultiplex::update() {
    unsigned long currentTime = micros();
    
    if (currentTime - _lastScan < _scanInterval) {
        return; // Not time to scan yet
    }
    
    _lastScan = currentTime;
    
    // Extract digits from display value
    int8_t digits[4];
    uint16_t value = _displayValue;
    
    if (_numDigits == 2) {
        // 2-digit display (00-99)
        digits[1] = value % 10;          // Ones
        digits[0] = (value / 10) % 10;   // Tens
        
        // Handle leading zeros
        if (!_leadingZeros && digits[0] == 0 && value < 10) {
            digits[0] = -1; // Blank
        }
    } else {
        // 4-digit display (0000-9999)
        digits[3] = value % 10;           // Ones
        digits[2] = (value / 10) % 10;    // Tens
        digits[1] = (value / 100) % 10;   // Hundreds
        digits[0] = (value / 1000) % 10;  // Thousands
        
        // Handle leading zeros
        if (!_leadingZeros) {
            if (value < 1000) digits[0] = -1;
            if (value < 100) digits[1] = -1;
            if (value < 10) digits[2] = -1;
        }
    }
    
    // Display current digit
    displayDigit(digits[_currentDigit]);
    
    // Move to next digit
    _currentDigit = (_currentDigit + 1) % _numDigits;
}

void SevenSegmentMultiplex::showNumber(uint16_t value) {
    // Limit value based on number of digits
    if (_numDigits == 2 && value > 99) {
        value = 99;
    } else if (_numDigits == 4 && value > 9999) {
        value = 9999;
    }
    
    _displayValue = value;
}

void SevenSegmentMultiplex::setLeadingZeros(bool enable) {
    _leadingZeros = enable;
}

void SevenSegmentMultiplex::setDecimalPoint(uint8_t digitIndex, bool enable) {
    if (digitIndex < _numDigits) {
        _decimalPoint[digitIndex] = enable;
    }
}

void SevenSegmentMultiplex::clearDecimalPoints() {
    for (int i = 0; i < 4; i++) {
        _decimalPoint[i] = false;
    }
}

void SevenSegmentMultiplex::setScanInterval(uint16_t intervalMicros) {
    _scanInterval = intervalMicros;
}

void SevenSegmentMultiplex::clear() {
    _displayValue = 0;
    clearDisplay();
}

void SevenSegmentMultiplex::showTime(uint8_t hours, uint8_t minutes) {
    if (_numDigits != 4) return; // Only for 4-digit displays
    
    // Limit to valid time ranges
    if (hours > 23) hours = 23;
    if (minutes > 59) minutes = 59;
    
    // Format as HHMM (e.g., 09:27 becomes 0927)
    _displayValue = (hours * 100) + minutes;
    
    // Enable decimal point on digit 1 to show colon (optional)
    // setDecimalPoint(1, true);
}

void SevenSegmentMultiplex::showTemperature(float temperature, uint8_t decimals) {
    if (decimals > 2) decimals = 2;
    
    // Convert temperature to integer representation
    int tempInt = 0;
    
    if (decimals == 0) {
        tempInt = (int)temperature;
    } else if (decimals == 1) {
        tempInt = (int)(temperature * 10);
        if (_numDigits == 4) {
            setDecimalPoint(2, true); // XX.X format
        } else {
            setDecimalPoint(0, true); // X.X format
        }
    } else if (decimals == 2) {
        tempInt = (int)(temperature * 100);
        if (_numDigits == 4) {
            setDecimalPoint(1, true); // X.XX format
        }
    }
    
    _displayValue = (uint16_t)tempInt;
}
