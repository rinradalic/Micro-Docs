# RelayController

A safe and simple relay control library for ESP32/Arduino projects. 

## Features

- ✅ Simple on/off/toggle control
- ✅ Support for active-high and active-low relay modules
- ✅ State management (track current relay state)
- ✅ Safe default (relay starts OFF)
- ✅ Compatible with transistor-driven and relay module circuits

## Hardware Requirements

### Option 1: Relay Module (Recommended for beginners)
- 5V or 3.3V relay module
- Direct GPIO connection

### Option 2: Transistor-Driven Relay Circuit
- NPN Transistor (2N2222, BC547, or similar)
- Relay coil (5V or matching your power supply)
- 1N4007 flyback diode
- 1kΩ resistor (base resistor)
- Power supply (5V/6V for relay)

## Circuit Diagram

### Transistor-Driven Relay (DIY Circuit)

```
                      +5V/6V (External Power)
                        │
                     Relay Coil
                        │
               ┌────────┴────────┐
               │                 │
              |>| 1N4007        NPN Transistor
               │              (2N2222/BC547)
               │                 │
              GND       Collector (C)
                                 │
                        Emitter (E) → GND

ESP32 GPIO ──[1kΩ]──► Base (B)
```

### Load Connection (NO Configuration)

```
+6V ────── COM
           Relay
LOAD ───── NO
  │
 GND
```

## Installation

### Arduino IDE

1. Copy `RelayController` folder to `~/Arduino/libraries/`
2. Restart Arduino IDE
3. Open examples: **File → Examples → RelayController**

### PlatformIO

```ini
lib_deps = 
    file:///path/to/Micro-Docs/RelayController
```

## Quick Start

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

## API Reference

| Method | Description |
|--------|-------------|
| `RelayController(pin, invertLogic=false)` | Constructor - pin number and logic inversion |
| `void begin()` | Initialize relay (call in `setup()`) |
| `void on()` | Turn relay ON (energize) |
| `void off()` | Turn relay OFF (de-energize) |
| `void toggle()` | Toggle relay state |
| `bool isOn()` | Returns true if relay is ON |
| `void setState(bool state)` | Set relay to specific state |

## Examples

### 1. Basic Control
Toggle relay every second.
```cpp
#include <RelayController.h>

RelayController relay(26);

void setup() {
  relay.begin();
}

void loop() {
  relay.toggle();
  delay(1000);
}
```

### 2. Button Control
Control relay with a push button.
```cpp
#include <Button.h>
#include <RelayController.h>

Button btn(4);
RelayController relay(26);

void setup() {
  btn.begin();
  relay.begin();
}

void loop() {
  btn.update();
  if (btn.wasPressed()) {
    relay.toggle();
  }
}
```

### 3. Temperature Control
Automatic relay control based on temperature.
```cpp
#include <RelayController.h>
#include <DHT.h>

DHT dht(4, DHT11);
RelayController relay(26);

void setup() {
  dht.begin();
  relay.begin();
}

void loop() {
  float temp = dht.readTemperature();
  relay.setState(temp > 30); // ON if temp > 30°C
  delay(2000);
}
```

## Safety Guidelines ⚠️

### Important Safety Rules:

1. **Low Voltage Testing**
   - Use only 5V/6V loads for initial testing
   - Test with LED, small fan, or buzzer first

2. **High Voltage (220V) Safety**
   - ⚠️ **DANGER**: 220V can be lethal
   - Use **opto-isolated relay modules** only
   - Never connect 220V to breadboards
   - Always use proper fuses and circuit breakers
   - Never touch live wires
   - Get adult supervision for AC projects

3. **Circuit Requirements**
   - Always connect common GND between ESP32 and relay circuit
   - Use flyback diode (1N4007) to protect transistor
   - Ensure adequate power supply for relay coil

4. **Testing**
   - Test circuit logic with multimeter first
   - Verify relay clicks before connecting load
   - Monitor current draw to avoid overheating

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Relay doesn't click | Check power supply, verify transistor connections |
| Relay clicks but load doesn't turn on | Check NO/NC/COM wiring, verify load voltage |
| ESP32 resets when relay switches | Add capacitor across relay coil, use separate power supply |
| Inverted behavior (ON = OFF) | Use `RelayController(pin, true)` for active-low modules |

## Pin Recommendations (ESP32)

- ✅ GPIO26, GPIO27, GPIO25, GPIO33 (safe for relay control)
- ⚠️ Avoid: GPIO0, GPIO2, GPIO12, GPIO15 (boot pins)
- ⚠️ Avoid: GPIO6-11 (flash pins)

## License

MIT License

## Related Libraries

- [Button](../Button/) - Debounced button input
- [LEDController](../LEDController/) - LED control with blinking

## Learning Resources

- Relay Control & Transistor Driver Circuit
- Understanding NPN transistors as switches
- Flyback diode protection explained
- Safe handling of AC loads
