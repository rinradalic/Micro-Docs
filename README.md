# Micro-Docs Arduino Libraries

A collection of lightweight, personal Arduino/ESP32 libraries for common microcontroller tasks.

## Libraries

### 1. LEDController
A simple LED controller supporting on/off/toggle and non-blocking blinking.

**Features:**
- On/off/toggle control
- Non-blocking blink mode with configurable interval
- Works with any digital GPIO pin
- Compatible with Arduino and ESP32

**Quick Example:**
```cpp
#include <LEDController.h>

LEDController led(LED_BUILTIN);

void setup() {
  led.begin();
  led.startBlink(500); // Blink every 500ms
}

void loop() {
  led.update(); // Must call in loop for blinking
}
```

[Full documentation →](LEDController/README.md)

---

### 2. Button
Debounced button library with optional internal pull-up support.

**Features:**
- Hardware debouncing (configurable, default 50ms)
- Internal pull-up resistor support
- `wasPressed()` - Single-fire event detection
- `isPressed()` - Current state query
- Compatible with Arduino and ESP32

**Quick Example:**
```cpp
#include <Button.h>

Button button(2); // Pin 2 with internal pullup

void setup() {
  Serial.begin(115200);
  button.begin();
}

void loop() {
  button.update();
  
  if (button.wasPressed()) {
    Serial.println("Button pressed!");
  }
}
```

[Full documentation →](Button/README.md)

---

### 3. RelayController
Safe relay control library for controlling external loads via transistor driver circuits or relay modules.

**Features:**
- Simple on/off/toggle control
- Support for active-high and active-low relay modules
- Built-in state management
- Safe default (relay starts OFF)
- Designed for EP3 curriculum (relay & transistor circuits)

**Quick Example:**
```cpp
#include <RelayController.h>

RelayController relay(26); // GPIO26

void setup() {
  relay.begin();
}

void loop() {
  relay.on();
  delay(1000);
  relay.off();
  delay(1000);
}
```

[Full documentation →](RelayController/README.md)

⚠️ **Safety Warning**: When working with relays and external loads, always follow proper safety guidelines. Use low-voltage loads (5V/6V) for testing. For AC loads (220V), use opto-isolated modules and get adult supervision.

---

### 4. PotentiometerController
Analog input library for reading potentiometers (VR) with built-in mapping and PWM control examples.

**Features:**
- Simple ADC reading with automatic platform detection
- Built-in percentage conversion (0-100%)
- Custom range mapping
- Examples for LED brightness control via PWM
- Designed for EP4 curriculum (analog control with VR)

**Quick Example:**
```cpp
#include <PotentiometerController.h>

PotentiometerController vr(34); // GPIO34 (ADC)

void setup() {
  Serial.begin(115200);
  vr.begin();
}

void loop() {
  int percent = vr.readPercent();
  Serial.print("VR Position: ");
  Serial.print(percent);
  Serial.println("%");
  delay(200);
}
```

[Full documentation →](PotentiometerController/README.md)

---

### 5. TemperatureSensor
DHT11/DHT22 temperature and humidity sensor library with smart control features and hysteresis support.

**Features:**
- Simple OOP interface for DHT sensors
- Temperature (°C/°F) and humidity (%RH) reading
- Hysteresis threshold management
- Temperature level detection (COOL/WARM/HOT)
- Designed for EP5 curriculum (Smart Fan Control)

**Quick Example:**
```cpp
#include <TemperatureSensor.h>
#include <RelayController.h>

TemperatureSensor sensor(4, DHT11);
RelayController fan(26);

void setup() {
  sensor.begin();
  sensor.setThresholds(30.0, 28.0); // ON at 30°C, OFF at 28°C
  fan.begin();
}

void loop() {
  if (sensor.read()) {
    if (sensor.isAboveHighThreshold()) fan.on();
    if (sensor.isBelowLowThreshold()) fan.off();
  }
  delay(1000);
}
```

[Full documentation →](TemperatureSensor/README.md)

**Required:** Install "DHT sensor library" and "Adafruit Unified Sensor" via Arduino Library Manager.

---

## Installation

### Arduino IDE

1. Download or clone this repository
2. Copy the library folders to your Arduino libraries directory:
   ```bash
   cp -r LEDController ~/Arduino/libraries/
   cp -r Button ~/Arduino/libraries/
   cp -r RelayController ~/Arduino/libraries/
   cp -r PotentiometerController ~/Arduino/libraries/
   cp -r TemperatureSensor ~/Arduino/libraries/
   ```
3. Restart Arduino IDE
4. Open examples via **File → Examples → [Library Name]**

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps = 
    file:///path/to/Micro-Docs/LEDController
    file:///path/to/Micro-Docs/Button
    file:///path/to/Micro-Docs/RelayController
    file:///path/to/Micro-Docs/PotentiometerController
    file:///path/to/Micro-Docs/TemperatureSensor
    DHT sensor library
    Adafruit Unified Sensor
```

Or use symbolic links:
```bash
cd your-project/lib
ln -s /path/to/Micro-Docs/LEDController
ln -s /path/to/Micro-Docs/Button
ln -s /path/to/Micro-Docs/RelayController
ln -s /path/to/Micro-Docs/PotentiometerController
ln -s /path/to/Micro-Docs/TemperatureSensor
```

---

## Project Structure

```
Micro-Docs/
├── LEDController/
│   ├── src/
│   │   ├── LEDController.h
│   │   └── LEDController.cpp
│   ├── examples/
│   │   ├── LEDControllerExample/
│   │   └── StateMachineExample/
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
│
├── Button/
│   ├── src/
│   │   ├── Button.h
│   │   └── Button.cpp
│   ├── examples/
│   │   └── ButtonExample/
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
│
├── RelayController/
│   ├── src/
│   │   ├── RelayController.h
│   │   └── RelayController.cpp
│   ├── examples/
│   │   ├── RelayBasicExample/
│   │   ├── RelayButtonControl/
│   │   └── RelayTemperatureControl/
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
│
├── PotentiometerController/
│   ├── src/
│   │   ├── PotentiometerController.h
│   │   └── PotentiometerController.cpp
│   ├── examples/
│   │   ├── VR_BasicReading/
│   │   ├── VR_LED_Brightness/
│   │   ├── VR_DualLED_Control/
│   │   └── VR_RunningLight_Speed/
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
│
├── TemperatureSensor/
│   ├── src/
│   │   ├── TemperatureSensor.h
│   │   └── TemperatureSensor.cpp
│   ├── examples/
│   │   ├── DHT_BasicReading/
│   │   ├── SmartFan_Hysteresis/
│   │   ├── SmartFan_ManualOverride/
│   │   └── TempLevel_Indicator/
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
│
├── examples/
│   └── StateMachineStandalone/
│
└── README.md (this file)
```

---

## Usage Examples

### Combined Example: LED Toggle with Button

```cpp
#include <LEDController.h>
#include <Button.h>

LEDController led(LED_BUILTIN);
Button button(2); // Pin 2

void setup() {
  Serial.begin(115200);
  led.begin();
  button.begin();
  Serial.println("LED + Button example");
}

void loop() {
  led.update();
  button.update();
  
  if (button.wasPressed()) {
    led.toggle();
    Serial.println("LED toggled");
  }
}
```

### Advanced: Blink Control with Button

```cpp
#include <LEDController.h>
#include <Button.h>

LEDController led(LED_BUILTIN);
Button button(2);

bool blinking = false;

void setup() {
  led.begin();
  button.begin();
}

void loop() {
  led.update();
  button.update();
  
  if (button.wasPressed()) {
    if (blinking) {
      led.stopBlink();
      led.off();
      blinking = false;
    } else {
      led.startBlink(200);
      blinking = true;
    }
  }
}
```

---

## Testing

Each library includes example sketches in the `examples/` directory.

**To test:**

1. Open Arduino IDE or PlatformIO
2. Load the example sketch
3. Select your board (Arduino Uno, ESP32, etc.)
4. Upload and monitor Serial output at 115200 baud

**Hardware requirements:**
- **LEDController**: Any board with LED_BUILTIN or external LED on a GPIO pin
- **Button**: A push button connected to a GPIO pin (with or without external pull-up)
- **RelayController**: Relay module OR transistor (NPN) + relay coil + flyback diode + resistor
- **PotentiometerController**: Potentiometer (VR) connected to ADC pin (use 3.3V on ESP32!)
- **TemperatureSensor**: DHT11 or DHT22 sensor (requires Adafruit DHT library)

---

## API Reference

### LEDController

| Method | Description |
|--------|-------------|
| `LEDController(uint8_t pin)` | Constructor |
| `void begin()` | Initialize pin (call in `setup()`) |
| `void on()` | Turn LED on |
| `void off()` | Turn LED off |
| `void toggle()` | Toggle LED state |
| `void startBlink(unsigned long ms)` | Start blinking with interval |
| `void stopBlink()` | Stop blinking, preserve state |
| `void update()` | Update blink state (call in `loop()`) |
| `bool isOn() const` | Query current state |

### Button

| Method | Description |
|--------|-------------|
| `Button(uint8_t pin, bool pullup=true, unsigned long debounce=50)` | Constructor |
| `void begin()` | Initialize pin (call in `setup()`) |
| `void update()` | Update button state (call in `loop()`) |
| `bool wasPressed()` | Returns true once per press |
| `bool isPressed() const` | Returns current pressed state |

### RelayController

| Method | Description |
|--------|-------------|
| `RelayController(uint8_t pin, bool invert=false)` | Constructor - pin and logic inversion |
| `void begin()` | Initialize relay (call in `setup()`) |
| `void on()` | Turn relay ON (energize) |
| `void off()` | Turn relay OFF (de-energize) |
| `void toggle()` | Toggle relay state |
| `bool isOn() const` | Returns true if relay is ON |
| `void setState(bool state)` | Set relay to specific state |

### PotentiometerController

| Method | Description |
|--------|-------------|
| `PotentiometerController(uint8_t pin)` | Constructor with ADC pin |
| `void begin()` | Initialize (optional on most platforms) |
| `int read()` | Read raw ADC value (0-4095 ESP32, 0-1023 Arduino) |
| `int readPercent()` | Read as percentage (0-100%) |
| `int readMapped(int min, int max)` | Read and map to custom range |
| `int getRaw() const` | Get last raw value without reading |
| `int getMaxRaw() const` | Get ADC resolution (4095 or 1023) |

### TemperatureSensor

| Method | Description |
|--------|-------------|
| `TemperatureSensor(uint8_t pin, uint8_t type=DHT11)` | Constructor with pin and sensor type |
| `void begin()` | Initialize sensor (call in `setup()`) |
| `bool read()` | Read temperature and humidity (returns true if successful) |
| `float getTemperature()` | Get temperature in Celsius |
| `float getTemperatureF()` | Get temperature in Fahrenheit |
| `float getHumidity()` | Get humidity in %RH |
| `bool isValid()` | Check if last reading was valid |
| `void setThresholds(float high, float low)` | Set hysteresis thresholds |
| `bool isAboveHighThreshold()` | Check if temp >= high threshold |
| `bool isBelowLowThreshold()` | Check if temp <= low threshold |
| `TempLevel getTemperatureLevel(...)` | Get COOL/WARM/HOT level |

---

## Development

### Adding New Libraries

Each library should follow this structure:

```
LibraryName/
├── src/
│   ├── LibraryName.h
│   └── LibraryName.cpp
├── examples/
│   └── ExampleName/
│       └── ExampleName.ino
├── library.properties
├── README.md
└── keywords.txt
```

### Contributing

This is a personal library collection. Feel free to fork and adapt for your own use.

---

## Compatibility

- ✅ Arduino Uno
- ✅ Arduino Mega
- ✅ Arduino Nano
- ✅ ESP32 (all variants)
- ✅ ESP8266
- ✅ Any Arduino-compatible board

---

## License

MIT License - see individual library folders for details.

---

## Author

Your Name

---

## Changelog

### v1.0.0 (2025-11-13)
- Initial release
- Added LEDController library
- Added Button library
