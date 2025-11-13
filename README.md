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

## Installation

### Arduino IDE

1. Download or clone this repository
2. Copy the library folders to your Arduino libraries directory:
   ```bash
   cp -r LEDController ~/Arduino/libraries/
   cp -r Button ~/Arduino/libraries/
   ```
3. Restart Arduino IDE
4. Open examples via **File → Examples → [Library Name]**

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps = 
    file:///path/to/Micro-Docs/LEDController
    file:///path/to/Micro-Docs/Button
```

Or use symbolic links:
```bash
cd your-project/lib
ln -s /path/to/Micro-Docs/LEDController
ln -s /path/to/Micro-Docs/Button
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
│   │   └── LEDControllerExample/
│   │       └── LEDControllerExample.ino
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
│   │       └── ButtonExample.ino
│   ├── library.properties
│   ├── README.md
│   └── keywords.txt
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
