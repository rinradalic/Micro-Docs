# TemperatureSensor

DHT11/DHT22 temperature and humidity sensor library with smart control features. Designed for EP5 curriculum - Smart Fan Control System.

## Features

- ✅ Simple OOP interface for DHT sensors
- ✅ Temperature reading (Celsius/Fahrenheit)
- ✅ Humidity reading (%RH)
- ✅ Hysteresis threshold management
- ✅ Temperature level detection (COOL/WARM/HOT)
- ✅ Integration with RelayController for automatic control
- ✅ Error handling and validation

## What is DHT11?

**DHT11** is a digital temperature and humidity sensor that measures:
- **Temperature**: 0-50°C (±2°C accuracy)
- **Humidity**: 20-90% RH (±5% accuracy)

### How it Works

The DHT11 sensor contains:
1. A capacitive humidity sensor
2. A thermistor (temperature sensor)
3. An 8-bit microcontroller

It communicates digitally via a single data pin using a proprietary protocol.

## Hardware Connection (ESP32/ICON-32)

### ⚠️ Important: Use 3.3V or 5V (both work with DHT11)

```
ICON-32          DHT11
┌─────────┐      ┌────┐
│ 3.3V ───┼──────┤ VCC│
│         │      │    │
│ GPIO4 ──┼──────┤DATA│
│         │      │    │
│ GND ────┼──────┤ GND│
└─────────┘      └────┘
```

### Recommended Pins (ESP32)

- ✅ GPIO 4 (default in examples)
- ✅ GPIO 5, 16, 17, 18, 19, 21, 22, 23
- ⚠️ Avoid boot pins: GPIO 0, 2, 12, 15

## Required Libraries

This library is a **wrapper** for the Adafruit DHT library. You must install:

1. **DHT sensor library** by Adafruit
2. **Adafruit Unified Sensor**

### Installation Steps

1. Open Arduino IDE
2. Go to **Tools → Manage Libraries**
3. Search for **"DHT sensor library"**
4. Install both libraries shown

## Installation

### Arduino IDE

```bash
cp -r TemperatureSensor ~/Arduino/libraries/
```

### PlatformIO

```ini
lib_deps = 
    file:///path/to/Micro-Docs/TemperatureSensor
    DHT sensor library
    Adafruit Unified Sensor
```

## Quick Start

### Example 1: Basic Reading

```cpp
#include <TemperatureSensor.h>

TemperatureSensor sensor(4, DHT11); // GPIO 4

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  if (sensor.read()) {
    float temp = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }
  delay(1000);
}
```

### Example 2: Smart Fan with Hysteresis

```cpp
#include <TemperatureSensor.h>
#include <RelayController.h>

TemperatureSensor sensor(4, DHT11);
RelayController fan(26);

bool fanRunning = false;

void setup() {
  sensor.begin();
  sensor.setThresholds(30.0, 28.0); // ON at 30°C, OFF at 28°C
  fan.begin();
}

void loop() {
  if (sensor.read()) {
    if (!fanRunning && sensor.isAboveHighThreshold()) {
      fanRunning = true;
      fan.on();
    } 
    else if (fanRunning && sensor.isBelowLowThreshold()) {
      fanRunning = false;
      fan.off();
    }
  }
  delay(1000);
}
```

## Understanding Hysteresis

**Hysteresis** prevents rapid on/off cycling by using two different thresholds:

```
Temperature (°C)
    │
35  │
    │         ╔══════════╗
30  │  ON ───►║          ║
    │         ║          ║
28  │  OFF ◄──╚══════════╝
    │
25  │
    └─────────────────────► Time
```

### Without Hysteresis (Bad)
```
Temp = 30°C → Fan ON
Temp = 29.9°C → Fan OFF
Temp = 30.0°C → Fan ON  ← Rapid cycling!
Temp = 29.9°C → Fan OFF
```

### With Hysteresis (Good)
```
Temp = 30°C → Fan ON
Temp = 29°C → Fan still ON
Temp = 28°C → Fan OFF
Temp = 29°C → Fan still OFF
Temp = 30°C → Fan ON
```

**Benefits:**
- Extends relay lifespan
- Reduces noise
- Saves energy
- More stable operation

## API Reference

### Constructor

| Method | Description |
|--------|-------------|
| `TemperatureSensor(pin, type=DHT11)` | Create sensor instance |

### Basic Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `void begin()` | - | Initialize sensor (call in `setup()`) |
| `bool read()` | true/false | Read temperature and humidity |
| `float getTemperature()` | °C | Get last temperature in Celsius |
| `float getTemperatureF()` | °F | Get last temperature in Fahrenheit |
| `float getHumidity()` | %RH | Get last humidity reading |
| `bool isValid()` | true/false | Check if last reading was successful |

### Threshold Methods

| Method | Description |
|--------|-------------|
| `void setThresholds(high, low)` | Set hysteresis thresholds |
| `bool isAboveHighThreshold()` | Check if temp >= high threshold |
| `bool isBelowLowThreshold()` | Check if temp <= low threshold |

### Temperature Level Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `TempLevel getTemperatureLevel(coolMax=28, warmMax=32)` | Enum | Get COOL/WARM/HOT |
| `const char* getTemperatureLevelString(...)` | String | Get level as text |

### Temperature Levels

| Enum | Value | Meaning |
|------|-------|---------|
| `TEMP_COOL` | 0 | Temperature < coolMax |
| `TEMP_WARM` | 1 | coolMax <= temp < warmMax |
| `TEMP_HOT` | 2 | Temperature >= warmMax |

## Complete Smart Fan System (EP5)

### Hardware Requirements

1. **ICON-32 (ESP32)** - Main controller
2. **DHT11** - Temperature sensor
3. **NPN Transistor** (2N2222/BC547) - Relay driver
4. **5V/6V Relay** - Switch for fan
5. **1N4007 Diode** - Flyback protection
6. **1kΩ Resistor** - Base resistor
7. **6V Fan** - Load
8. **6V 2A Adapter** - Power supply
9. **LED + 220Ω** - Status indicator

### Complete Circuit Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    ICON-32 (ESP32)                      │
│                                                          │
│  3.3V ──────────────► DHT11 VCC                         │
│  GPIO4 ─────────────► DHT11 DATA                        │
│  GND ───────────────► DHT11 GND                         │
│                                                          │
│  GPIO26 ──[1kΩ]───► Base (NPN Transistor)              │
│                      Collector ──┐                      │
│                                  │                      │
│  GPIO2 ──[220Ω]──►|─┐ (LED)     │                      │
│                     │            │                      │
│  GND ◄──────────────┴────────────┴─ Emitter            │
└──────────────────────────────────────────────────────────┘
                                   │
                              Relay Coil (-)
                                   │
                        ┌──────────┴──────────┐
                        │                     │
                       |>| 1N4007            +6V
                        │                  (Adapter)
                       GND                    │
                                         Relay Coil (+)

Fan Circuit (via Relay):
  +6V (Adapter) ──► COM
  NO ───────────► Fan (+)
  Fan (-) ──────► GND (Adapter)

⚠️ IMPORTANT: Common GND between ICON-32 and 6V Adapter!
```

## Examples

### 1. DHT_BasicReading
Simple temperature and humidity monitoring.

**Skills**: DHT sensor reading, error handling

### 2. SmartFan_Hysteresis
Automatic fan control with hysteresis (EP5 main project).

**Skills**: Hysteresis logic, relay control, system integration

### 3. SmartFan_ManualOverride
Fan control with AUTO/MANUAL mode switching via button.

**Skills**: Mode management, user override, state machines

### 4. TempLevel_Indicator
Visual temperature indicator with 3 LEDs (COOL/WARM/HOT).

**Skills**: Multi-threshold detection, visual feedback

## Learning Objectives (EP5)

After completing EP5, students will be able to:

- ✅ Explain how DHT11 sensor works
- ✅ Read temperature and humidity from DHT11
- ✅ Use if/else logic for automatic control
- ✅ Apply transistor driver circuit from EP3
- ✅ Understand and implement hysteresis
- ✅ Build a complete Smart Fan project
- ✅ Integrate multiple subsystems

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Failed to read sensor" | Check connections, wait 2 seconds after begin() |
| Always reads NaN | Verify VCC/GND connections, try different GPIO pin |
| Erratic readings | Add 10kΩ pull-up resistor on DATA pin |
| Fan doesn't turn on | Check relay circuit, verify transistor orientation |
| Fan cycles rapidly | Increase hysteresis gap (TEMP_ON - TEMP_OFF) |

## Advanced: Sensor Stability

For more stable readings, implement averaging:

```cpp
float getAverageTemp(int samples = 5) {
  float sum = 0;
  int validReads = 0;
  
  for (int i = 0; i < samples; i++) {
    if (sensor.read()) {
      sum += sensor.getTemperature();
      validReads++;
    }
    delay(200);
  }
  
  return validReads > 0 ? sum / validReads : 0;
}
```

## Safety Notes

- ⚠️ DHT11 is for ambient temperature only (not for high-temp applications)
- ⚠️ When controlling AC loads (fans, heaters), use proper relays rated for the voltage
- ⚠️ Always use flyback diode with relay coils
- ✅ Start with low-voltage DC loads (5V/6V) for learning
- ✅ Get adult supervision when working with mains voltage

## Practical Applications

- 🌡️ Automatic cooling system
- 💨 Humidity-controlled ventilation
- 🏠 Home automation
- 🌱 Greenhouse climate control
- 📦 Storage room monitoring
- 🖥️ Equipment cooling

## Related Libraries

- [RelayController](../RelayController/) - Relay control for loads
- [Button](../Button/) - Manual override control
- [LEDController](../LEDController/) - Status indicators

## License

MIT License
