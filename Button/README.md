# Button Library

Button control library for Arduino and ESP32 with debouncing, interrupt support, and advanced click detection.

---

## 🎯 Learning Objectives

After completing this library, students will be able to:

### Basic Button Control
- Use `digitalRead()` to read button state
- Implement software debounce with `millis()`
- Detect button press events (edge detection)
- Use internal pull-up resistors

### Advanced Button Techniques
- **Understand Polling vs Interrupt**
  - Explain when to use each approach
  - Identify limitations of polling with `delay()`
  
- **Use Hardware Interrupts**
  - Implement `attachInterrupt()` on ESP32
  - Write proper ISR (Interrupt Service Routine)
  - Use `volatile` and `IRAM_ATTR` correctly
  
- **Implement Advanced Debounce**
  - Software debounce in ISR vs loop()
  - Hardware debounce with RC filter
  - Calculate RC time constants
  
- **Detect Click Patterns**
  - Single Click
  - Double Click
  - Long Press
  - Use `AdvancedButton` class for complex interactions

---

## 🚀 Quick Start

### Example 1: Basic Button

```cpp
#include <Button.h>

Button button(4);  // GPIO 4 with internal pullup

void setup() {
  button.begin();
}

void loop() {
  button.update();
  
  if (button.wasPressed()) {
    // Button just pressed (fires once)
  }
}
```

### Example 2: Interrupt-based Button 

```cpp
const int BUTTON_PIN = 4;
volatile bool flag = false;

void IRAM_ATTR buttonISR() {
  flag = true;  // Keep ISR short!
}

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
  if (flag) {
    flag = false;
    // Handle button press
  }
}
```

### Example 3: Advanced Button 

```cpp
#include <Button.h>

AdvancedButton button(4);

void setup() {
  button.begin();
}

void loop() {
  button.update();
  
  if (button.singleClick) {
    // Single click detected
  }
  
  if (button.doubleClick) {
    // Double click detected
  }
  
  if (button.longPress) {
    // Long press detected
  }
}
```

---

## 📚 API Reference

### Button Class 

Basic button with software debounce.

| Method | Description |
|--------|-------------|
| `Button(pin, pullup, debounce)` | Constructor. `pin` = GPIO pin, `pullup` = enable internal pull-up (default true), `debounce` = debounce time in ms (default 50) |
| `begin()` | Initialize pin mode. Call in `setup()` |
| `update()` | Update button state. Call in `loop()` |
| `wasPressed()` | Returns true once when button is pressed |
| `isPressed()` | Returns true while button is held down |

### AdvancedButton Class 

Advanced button with single/double click and long press detection.

| Method | Description |
|--------|-------------|
| `AdvancedButton(pin, pullup, debounce, doubleClickWindow, longPressThreshold)` | Constructor with timing configuration |
| `begin()` | Initialize pin mode |
| `update()` | Update state and detect events. **Call every loop()** |
| `isPressed()` | Returns true while button is held |
| `getPressedDuration()` | Returns how long button has been held (ms) |
| `setDebounceTime(ms)` | Set debounce time (default 40ms) |
| `setDoubleClickWindow(ms)` | Set max time between clicks for double-click (default 300ms) |
| `setLongPressThreshold(ms)` | Set min time for long press (default 800ms) |

**Event Flags** (reset after each `update()`):
- `singleClick` - Single short press detected
- `doubleClick` - Two quick presses detected
- `longPress` - Button held for long time
- `pressed` - Button just pressed (edge)
- `released` - Button just released (edge)

---

## 📂 Examples

### : Basic Button
1. **ButtonExample** - Basic polling with software debounce

### : Interrupts + Advanced Button
2. **Interrupt_BasicToggle** - Introduction to ISR and interrupts
3. **Interrupt_SoftwareDebounce** - Advanced debounce outside ISR
4. **AdvancedButton_ClickTypes** - Single/Double/Long press detection
5. **HardwareDebounce_Comparison** - Compare software vs hardware debounce

---

## 🧠 : Concepts & Techniques

### Polling vs Interrupt

#### Polling ( Approach)
```cpp
void loop() {
  int state = digitalRead(buttonPin);  // Read every loop
  // Process...
}
```

**Advantages:**
- Simple to understand
- Full control of execution order
- No special syntax required

**Disadvantages:**
- Can miss button presses if `loop()` is slow
- Wastes CPU cycles checking when nothing happens
- Poor responsiveness with `delay()` in code

#### Interrupt ( Approach)
```cpp
void IRAM_ATTR buttonISR() {
  // CPU jumps here immediately when event occurs
  flag = true;
}

void setup() {
  attachInterrupt(digitalPinToInterrupt(pin), buttonISR, FALLING);
}
```

**Advantages:**
- Instant response (no waiting for loop)
- CPU does other work until event occurs
- Perfect for time-critical events

**Disadvantages:**
- ISR must be short and fast
- Cannot use `Serial.print()`, `delay()`, or complex logic
- Need `volatile` for shared variables

---

### ISR Best Practices

**✅ DO:**
- Keep ISR short (< 10 lines)
- Only set flags or store timestamps
- Use `volatile` for variables shared with `loop()`
- Use `IRAM_ATTR` on ESP32 (faster execution)

**❌ DON'T:**
- Use `delay()`
- Use `Serial.print()`
- Use `millis()` extensively (use it, but don't rely on accuracy)
- Perform complex calculations
- Call blocking functions

**Example:**
```cpp
volatile bool buttonPressed = false;
volatile unsigned long pressTime = 0;

void IRAM_ATTR buttonISR() {
  buttonPressed = true;
  pressTime = millis();  // OK: quick timestamp
  // Don't do complex logic here!
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false;
    // Do all the processing here
    handleButtonPress(pressTime);
  }
}
```

---

### Software Debounce Techniques

#### Method 1: Debounce in ISR (Simple)
```cpp
volatile unsigned long lastInterrupt = 0;
const unsigned long DEBOUNCE_MS = 150;

void IRAM_ATTR buttonISR() {
  unsigned long now = millis();
  if (now - lastInterrupt > DEBOUNCE_MS) {
    // Valid press
    lastInterrupt = now;
    flag = true;
  }
}
```

**Pros:** Simple, works for basic use  
**Cons:** Still does logic in ISR

#### Method 2: Debounce in loop() (Better)
```cpp
volatile bool rawEvent = false;
volatile unsigned long rawTime = 0;

void IRAM_ATTR buttonISR() {
  rawEvent = true;     // Just record event
  rawTime = millis();  // And timestamp
}

void loop() {
  if (rawEvent) {
    rawEvent = false;
    // Do full debounce logic here
    if (rawTime - lastValid > DEBOUNCE_MS) {
      // Valid press
      handlePress();
      lastValid = rawTime;
    }
  }
}
```

**Pros:** ISR is minimal, full control in loop()  
**Cons:** Slightly more code

---

### Hardware Debounce: RC Filter

Reduce bounce at the source with a capacitor!

#### Circuit Diagram
```
     3.3V
      │
    [10kΩ]  ← Pull-up Resistor
      │
      ├──────┐
      │      │
    GPIO   [0.1µF]  ← Capacitor to GND
      │      │
  [Button]  GND
      │
     GND
```

#### How It Works
- **R** (resistor) + **C** (capacitor) = Low-pass filter
- Signal changes slowly → smooth edges
- Eliminates rapid bouncing

#### Calculating Time Constant
```
τ (tau) = R × C

Example:
R = 10kΩ = 10,000 Ω
C = 0.1µF = 0.0000001 F
τ = 10,000 × 0.0000001 = 0.001s = 1ms

Signal settles in ~5τ = 5ms
```

#### Choosing Capacitor Value

| Capacitor | Time Constant | Settling Time | Use Case |
|-----------|---------------|---------------|----------|
| 0.01 µF   | 0.1 ms        | 0.5 ms        | High-quality buttons |
| 0.1 µF    | 1 ms          | 5 ms          | **Recommended** for most buttons |
| 0.47 µF   | 4.7 ms        | 23.5 ms       | Poor quality buttons |
| 1.0 µF    | 10 ms         | 50 ms         | Very old/worn buttons |

**⚠️ Caution:**
- Too large C → slow response
- Too small C → doesn't filter bounce completely

**💡 Best Practice:** Use both hardware (RC filter) + software debounce for best results!

---

### Advanced Click Detection

The `AdvancedButton` class uses a state machine to detect different click patterns:

#### Single Click
1. Press button briefly
2. Release
3. Wait > 300ms (no second click)
4. → `singleClick` flag set

**Note:** Single click has delay because it waits to see if a second click comes.

#### Double Click
1. Press button briefly → Release
2. Press again within 300ms → Release
3. → `doubleClick` flag set immediately

#### Long Press
1. Press button
2. Hold for > 800ms
3. → `longPress` flag set
4. Release (long press already fired)

#### Timing Diagram
```
Press         Release    Press    Release
  │             │         │         │
  ▼             ▼         ▼         ▼
──┐           ┌───────┐           ┌─────  Signal
  └───────────┘       └───────────┘

  ←── < 800ms ──→ ← <300ms → ← < 800ms →
     Short Press   Quick gap   Short Press
                   
                   = DOUBLE CLICK

Press               Hold...                Release
  │                                          │
  ▼                                          ▼
──┐                                        ┌─────
  └────────────────────────────────────────┘

  ←────────── > 800ms ──────────→
           Long Hold
           
           = LONG PRESS (fires while holding)
```

---

## 🔧 Troubleshooting

### Button not detected
- Check wiring (button to GND, GPIO to button)
- Verify `pinMode(pin, INPUT_PULLUP)` is called
- Try reversing button polarity in constructor: `Button(pin, false)`

### Multiple presses detected (bouncing)
- Increase debounce time: `button.setDebounceTime(100)`
- Add hardware RC filter (0.1µF capacitor)
- Check button quality (cheap buttons bounce more)

### Interrupt not working
- Verify pin supports interrupts (most ESP32 GPIOs do)
- Check ISR function has `IRAM_ATTR` attribute
- Verify `attachInterrupt()` is called in `setup()`
- Make sure shared variables are `volatile`

### Double click hard to trigger
- Decrease `doubleClickWindow`: `button.setDoubleClickWindow(400)`
- Decrease debounce time: `button.setDebounceTime(30)`
- Practice clicking faster!

### Long press not detected
- Decrease threshold: `button.setLongPressThreshold(600)`
- Make sure `update()` is called every loop (no long delays)

### Single click delayed
- This is normal! Single click waits for potential double click
- Use `button.pressed` for immediate response if needed

---

## 📖 Teaching Guide (สำหรับครูผู้สอน)

###  Lesson Plan (4-6 hours)

#### Part 1: Introduction to Interrupts (1-1.5 hours)
**Objectives:**
- Understand polling limitations
- Learn interrupt concept
- Write first ISR

**Activities:**
1. Demo: Show polling with `delay()` missing button presses
2. Explain CPU interrupt mechanism
3. Live code: `Interrupt_BasicToggle` example
4. Experiment: Add `delay(1000)` to loop, show button still works

**Key Concepts:**
- Interrupt = "emergency call" for CPU
- ISR must be fast
- `volatile` keyword purpose
- `IRAM_ATTR` for ESP32

---

#### Part 2: Software Debounce (1-1.5 hours)
**Objectives:**
- Understand button bounce phenomenon
- Implement proper software debounce
- Separate ISR from logic

**Activities:**
1. Demo: Show bounce on oscilloscope (if available)
2. Serial Monitor: See bounce events in `Interrupt_SoftwareDebounce`
3. Experiment: Change debounce time, observe results
4. Code review: ISR vs loop() responsibilities

**Teaching Tip:**
- Use Serial Monitor to show raw events vs debounced events
- Let students tune debounce time for their buttons

---

#### Part 3: Hardware Debounce (1 hour)
**Objectives:**
- Understand RC filter concept
- Calculate time constants
- Compare hardware vs software

**Activities:**
1. Build circuit with 0.1µF capacitor
2. Run `HardwareDebounce_Comparison` example
3. Compare bounce rates
4. Calculate τ = R × C for different values

**Materials Needed:**
- Capacitors: 0.01µF, 0.1µF, 0.47µF, 1µF
- Oscilloscope (optional but recommended)

**Teaching Tip:**
- Show oscilloscope traces if available (very impactful!)
- Have students swap capacitors and observe differences

---

#### Part 4: Advanced Click Detection (1-2 hours)
**Objectives:**
- Detect single/double/long press
- Use state machine concept
- Apply to real-world UI

**Activities:**
1. Demo: Show `AdvancedButton_ClickTypes`
2. Brainstorm: Where would you use each click type?
3. Project: Students create 3-function LED controller
   - Single click → Toggle LED
   - Double click → Change brightness mode
   - Long press → Turn off all

**Real-World Applications:**
- Smartphone volume buttons
- TV remote controls
- Smart home switches
- Industrial control panels

---

### Assessment Ideas

**Basic ( level):**
- Wire button with pull-up resistor
- Implement software debounce
- Toggle LED on press

**Intermediate ( level):**
- Use interrupt instead of polling
- Explain ISR best practices
- Add RC filter and measure improvement

**Advanced ( mastery):**
- Create button with 3 click types
- Each type controls different function
- Combine with other libraries (LEDController, RelayController)

**Project Example:**
Smart Room Controller
- Single click → Toggle light
- Double click → Toggle fan
- Long press → Emergency mode (all off)

---

### Common Mistakes & Solutions

**Mistake 1:** Doing Serial.print() in ISR
```cpp
void IRAM_ATTR badISR() {
  Serial.println("Pressed");  // ❌ WRONG!
}
```
**Solution:** Set flag, print in loop()
```cpp
void IRAM_ATTR goodISR() {
  flag = true;  // ✅ CORRECT
}
void loop() {
  if (flag) {
    flag = false;
    Serial.println("Pressed");
  }
}
```

**Mistake 2:** Forgetting volatile
```cpp
bool flag = false;  // ❌ Compiler may optimize away
```
**Solution:**
```cpp
volatile bool flag = false;  // ✅ CORRECT
```

**Mistake 3:** RC filter backwards
```cpp
// ❌ WRONG: Capacitor in series
GPIO ──[C]──[Button]── GND
```
**Solution:**
```cpp
// ✅ CORRECT: Capacitor to GND
     GPIO ──[Button]── GND
      │
     [C]
      │
     GND
```

---

## 📄 License

MIT License - Free for educational and commercial use.

---

## 🔗 Related Libraries

- **LEDController** - Control LEDs with patterns and timing
- **RelayController** - Control relays with button input
- **DigitalLogic** - Combine buttons with logic circuits

---

**Micro-Docs Library Collection**  
Educational Arduino/ESP32 libraries for hands-on learning
