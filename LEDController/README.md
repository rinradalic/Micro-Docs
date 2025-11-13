# LEDController

A small Arduino/ESP32 library to control a single LED pin. Supports on/off/toggle and non-blocking blinking.

## Quick start

1. Copy the `LEDController` folder into your Arduino `libraries` folder, or install via PlatformIO linking the folder.
2. Open the example in `examples/LEDControllerExample`.
3. Call `led.begin()` in `setup()` and `led.update()` in `loop()`.

## API

- LEDController(uint8_t pin)
- void begin()
- void on()
- void off()
- void toggle()
- void startBlink(unsigned long ms)
- void stopBlink()
- void update()
- bool isOn() const

## Contract

Inputs: pin number, interval (ms) for blinking.
Outputs: toggles the GPIO pin HIGH/LOW.
Error modes: none; user must ensure chosen pin is valid for target board.

## License
MIT
