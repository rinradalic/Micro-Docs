# Button

Debounced button library with optional internal pull-up support for Arduino and ESP32.

## Quick start

1. Copy the `Button` folder into your Arduino `libraries` folder.
2. Open the example in `examples/ButtonExample`.
3. Call `button.begin()` in `setup()` and `button.update()` in `loop()`.

## API

- Button(uint8_t pin, bool enablePullup = true, unsigned long debounce = 50)
- void begin()
- void update()
- bool wasPressed()
- bool isPressed() const

## Usage

```cpp
#include <Button.h>

Button button(2); // pin 2 with internal pullup

void setup() {
  button.begin();
}

void loop() {
  button.update();
  
  if (button.wasPressed()) {
    // button was just pressed (fires once)
  }
  
  if (button.isPressed()) {
    // button is currently held down
  }
}
```

## Contract

Inputs: pin number, enable pullup (default true), debounce time in ms (default 50).
Outputs: event flags and state queries.
Error modes: none; user must ensure chosen pin is valid for target board.

## License
MIT
