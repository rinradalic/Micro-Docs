# DigitalLogic Library

> **Digital Logic + (ESP32)**  
> ผสมผสาน Digital Logic ICs (74xx/CD4xxx) กับไมโครคอนโทรลเลอร์ ESP32

ไลบรารีสำหรับเชื่อมต่อและควบคุม IC ดิจิทัลแบบดั้งเดิม (Logic Gates, Counters, Decoders) ด้วย (ESP32) เหมาะสำหรับการเรียนรู้การผสมผสานระหว่างวงจรดิจิทัลกับไมโครคอนโทรลเลอร์

---

## 📚 เนื้อหาที่เรียนรู้

### Digital Logic Gates
- การทำงานของ IC Digital พื้ฐาน (Logic Gates)
- Truth Tables และการทดสอบ gates
- ใช้ ESP32 ป้อนสัญญาณให้ IC

### Digital Counter + 7-Segment
- การต่อวงจร IC Digital กับ ESP32
- การใช้ ESP32 เป็น Clock Generator
- การสร้างระบบ Binary Counter ด้วย 7493
- การแปลง BCD เป็น 7-segment ด้วย CD4511
- โปรเจค Smart Counter ที่รวมทุกคอมโพเนนต์เข้าด้วยกัน

### Multiplexing 2-Digit 7-Segment
- หลักการ Multiplexing (สแกนทีละหลักแบบรวดเร็ว)
- ขับ 7-segment 2 หลักด้วย GPIO เพียง 9 ขา
- Non-blocking display scanning
- โปรเจค 2-digit counter with controls

### Multiplexing 4-Digit 7-Segment
- ขับ 7-segment 4 หลักด้วย GPIO เพียง 11 ขา
- แสดงเลข 0000-9999
- นาฬิกาดิจิทัล (HH:MM format)
- แสดงอุณหภูมิจาก DHT11/DHT22

---

## 🎯 IC ที่รองรับ

### Logic Gates (74xx Series - TTL)
- **7400** - Quad 2-input NAND gates
- **7404** - Hex inverters (NOT gates)
- **7408** - Quad 2-input AND gates
- **7432** - Quad 2-input OR gates
- **7486** - Quad 2-input XOR gates

### Counters (74xx Series)
- **7473** - Dual JK Flip-Flop with Clear
- **7493** - 4-bit Binary Counter

### Decoders (CD4xxx Series - CMOS)
- **CD4511** - BCD-to-7-Segment Decoder/Driver (Common Cathode)

---

## ⚡ Quick Start

### Installation

1. ดาวน์โหลดไลบรารีนี้
2. วางในโฟลเดอร์ `libraries` ของ Arduino
3. เปิด Arduino IDE และเลือก **Sketch → Include Library → DigitalLogic**

### Basic Usage

#### Example 1: Test Logic Gates

```cpp
#include <DigitalLogic.h>

LogicGate andGate(26, 27, GATE_AND);

void setup() {
  andGate.begin();
  andGate.setInputs(true, true);  // Set both inputs HIGH
  bool output = andGate.calculateOutput();  // Should be true
}
```

#### Example 2: Generate Clock Pulses

```cpp
#include <DigitalLogic.h>

ClockGenerator clock(25);

void setup() {
  clock.begin(2);  // 2 Hz clock
  clock.start();
}

void loop() {
  clock.update();  // Must call in loop for non-blocking operation
}
```

#### Example 3: Display on 7-Segment

```cpp
#include <DigitalLogic.h>

BCDEncoder display(19, 18, 5, 17);  // A, B, C, D pins

void setup() {
  display.begin();
}

void loop() {
  for (int i = 0; i <= 9; i++) {
    display.displayDigit(i);
    delay(500);
  }
}
```

#### Example 4: Multiplexed 4-Digit Display

```cpp
#include <DigitalLogic.h>

// Create 4-digit multiplexed display
SevenSegmentMultiplex display(
  14, 27, 26, 25, 33, 32, 23,  // Segment pins (a-g)
  22, 21, 19, 18,              // Digit control pins
  true                          // Common Cathode
);

int counter = 0;

void setup() {
  display.begin();
  display.setLeadingZeros(false);
}

void loop() {
  display.update();  // MUST call frequently for smooth display!
  
  static unsigned long lastCount = 0;
  if (millis() - lastCount > 1000) {
    counter++;
    if (counter > 9999) counter = 0;
    display.showNumber(counter);
    lastCount = millis();
  }
}
```

---

## 📟 IC Pinout References

### 7493 - 4-Bit Binary Counter

```
       ┌──────────────┐
    B  │1          14│ A (CLK input)
  R0(1)│2          13│ NC
  R0(2)│3          12│ QA (Q0 - LSB)
    NC │4          11│ QD (Q3 - MSB)
   VCC │5          10│ GND
    NC │6           9│ QB (Q1)
    NC │7           8│ QC (Q2)
       └──────────────┘
```

**Functionality:**
- Clock input A: pin 14
- Outputs: Q0(12), Q1(9), Q2(8), Q3(11)
- Reset: R0(1) and R0(2) both HIGH = reset to 0000
- Counts 0000 → 1111 (0-15 in decimal)

### CD4511 - BCD to 7-Segment Decoder

```
       ┌──────────────┐
     B │1          16│ VCC
     C │2          15│ f
    LT │3          14│ g
  BI/RB│4          13│ a
    LE │5          12│ b
     D │6          11│ c
     A │7          10│ d
   GND │8           9│ e
       └──────────────┘
```

**Inputs:**
- A, B, C, D: BCD input (0000-1001 for digits 0-9)
- LT (Lamp Test): Active LOW - lights all segments
- BI (Blanking Input): Active LOW - blanks display
- LE (Latch Enable): LOW = transparent, HIGH = latched

**Outputs:**
- a-g: 7-segment outputs (Common Cathode compatible)

### 7408 - Quad 2-Input AND Gates

```
       ┌──────────────┐
   1A  │1          14│ VCC
   1B  │2          13│ 4B
   1Y  │3          12│ 4A
   2A  │4          11│ 4Y
   2B  │5          10│ 3B
   2Y  │6           9│ 3A
   GND │7           8│ 3Y
       └──────────────┘
```

**Note:** Contains 4 independent AND gates. Each gate has 2 inputs and 1 output.

---

## 🔧 API Reference

### LogicGate Class

| Method | Description |
|--------|-------------|
| `LogicGate(pinA, type)` | Constructor for single-input gates (NOT) |
| `LogicGate(pinA, pinB, type)` | Constructor for two-input gates |
| `begin()` | Initialize GPIO pins |
| `setInputs(a, b)` | Set both inputs (b ignored for NOT) |
| `setA(level)` | Set input A |
| `setB(level)` | Set input B |
| `bool getA()` | Get current state of A |
| `bool getB()` | Get current state of B |
| `bool calculateOutput()` | Calculate expected output |

**Gate Types:** `GATE_AND`, `GATE_OR`, `GATE_NOT`, `GATE_NAND`, `GATE_XOR`

### ClockGenerator Class

| Method | Description |
|--------|-------------|
| `ClockGenerator(pin)` | Constructor |
| `begin(frequency)` | Initialize with frequency in Hz |
| `setFrequency(freq)` | Set clock frequency (1-1000 Hz) |
| `unsigned long getFrequency()` | Get current frequency |
| `start()` | Start clock generation |
| `stop()` | Stop clock generation |
| `bool isRunning()` | Check if clock is running |
| `pulse()` | Generate single pulse |
| `update()` | Update clock state (call in loop) |

### BCDEncoder Class

| Method | Description |
|--------|-------------|
| `BCDEncoder(pinA, pinB, pinC, pinD)` | Constructor with BCD pins |
| `begin()` | Initialize pins |
| `displayDigit(digit)` | Display digit 0-9 on 7-segment |
| `uint8_t getCurrentDigit()` | Get currently displayed digit |
| `clear()` | Clear display (blank) |
| `lampTest()` | Light all segments for testing |
| `endLampTest()` | End lamp test |

### CounterController Class

| Method | Description |
|--------|-------------|
| `CounterController(clockPin, resetPin)` | Constructor |
| `begin(frequency)` | Initialize with frequency |
| `reset()` | Reset counter to 0 |
| `start()` | Start counting |
| `stop()` | Stop counting |
| `singleStep()` | Generate single count pulse |
| `setFrequency(freq)` | Set count frequency |
| `ClockGenerator* getClock()` | Get clock generator |
| `update()` | Update clock (call in loop) |

### SevenSegmentMultiplex Class

| Method | Description |
|--------|-------------|
| `SevenSegmentMultiplex(seg pins..., dig pins..., commonCathode)` | Constructor for 2-digit or 4-digit |
| `begin()` | Initialize display pins |
| `update()` | **MUST call in loop()** for display scanning |
| `showNumber(value)` | Display number (0-99 or 0-9999) |
| `uint16_t getNumber()` | Get currently displayed number |
| `setLeadingZeros(enable)` | Show/hide leading zeros |
| `setDecimalPoint(index, enable)` | Set decimal point for digit |
| `clearDecimalPoints()` | Clear all decimal points |
| `setScanInterval(micros)` | Set scan speed (default 1000μs) |
| `clear()` | Clear display |
| `showTime(hours, minutes)` | Display time HH:MM (4-digit only) |
| `showTemperature(temp, decimals)` | Display temperature with decimals |

**⚠️ Important:** Must call `update()` frequently in `loop()` for smooth display!

---

## 💡 Examples

### 1. LogicGateTester
ทดสอบการทำงานของ logic gates ต่าง ๆ (AND, OR, NOT, NAND, XOR) โดยแสดง truth table ผ่าน Serial Monitor

### 2. ClockGenerator_Demo
สาธิตการสร้างสัญญาณ clock ความถี่ต่าง ๆ พร้อมควบคุมผ่าน Serial commands

### 3. BinaryCounter_7493
ใช้ 7493 นับเลขแบบ Binary (0-15) แสดงผลด้วย LED และ Serial Monitor

### 4. SevenSegment_CD4511
ใช้ CD4511 แสดงตัวเลข 0-9 บน 7-segment display พร้อม demo modes ต่าง ๆ

### 5. SmartCounter
ระบบนับเลขอัจฉริยะรวม ESP32 + 7493 + CD4511 + 7-segment display พร้อมปุ่มควบคุม COUNT และ RESET

### 6. Multiplex_2Digit
- แสดงเลข 00-99 บน 7-segment 2 หลัก
- ใช้ GPIO เพียง 9 ขา (7 segments + 2 digits)
- ปุ่มควบคุม UP/DOWN/RESET
- โหมดนับอัตโนมัติ

### 7. Multiplex_4Digit
- แสดงเลข 0000-9999 บน 7-segment 4 หลัก
- ใช้ GPIO เพียง 11 ขา (7 segments + 4 digits)
- 3 โหมด: Counter, Stopwatch, Countdown
- Non-blocking display scanning

### 8. DigitalClock_4Digit
- นาฬิกาดิจิทัล 24 ชั่วโมง (HH:MM)
- ตั้งเวลาด้วยปุ่ม
- โหมด 12/24 ชั่วโมง (ตัวเลือก)
- กะพริบเครื่องหมาย ":" ทุกวินาที

### 9. TemperatureDisplay_4Digit
- แสดงอุณหภูมิและความชื้นจาก DHT11/DHT22
- รองรับ 3 โหมดแสดงผล
- แสดงทศนิยม 1 ตำแหน่ง (XX.X)
- สลับแสดงผลอัตโนมัติ

---

## 🔌 Multiplexing Circuit Diagrams

### 2-Digit 7-Segment Multiplexing

```
ESP32 Segments (shared by both digits):
   GPIO 14 ──┬──► R 220Ω ──► Segment a (Digit 1 & 2)
   GPIO 27 ──┼──► R 220Ω ──► Segment b
   GPIO 26 ──┼──► R 220Ω ──► Segment c
   GPIO 25 ──┼──► R 220Ω ──► Segment d
   GPIO 33 ──┼──► R 220Ω ──► Segment e
   GPIO 32 ──┼──► R 220Ω ──► Segment f
   GPIO 23 ──┴──► R 220Ω ──► Segment g

Digit Control (Common Cathode):
   GPIO 22 ──► R 10kΩ ──► Base NPN 1
                          Collector ──► Digit 1 Common Cathode
                          Emitter ──► GND

   GPIO 21 ──► R 10kΩ ──► Base NPN 2
                          Collector ──► Digit 2 Common Cathode
                          Emitter ──► GND

Timing:
   - Digit 1 ON for 1ms
   - Digit 2 ON for 1ms
   - Repeat at ~500 Hz
   - Eye sees continuous display!
```

### 4-Digit 7-Segment Multiplexing

```
ESP32 Segments (shared by all 4 digits):
   GPIO 14-23 ──► Same as 2-digit (a-g segments)

Digit Control (Common Cathode):
   GPIO 22 ──► NPN 1 ──► Digit 1 (Thousands)
   GPIO 21 ──► NPN 2 ──► Digit 2 (Hundreds)
   GPIO 19 ──► NPN 3 ──► Digit 3 (Tens)
   GPIO 18 ──► NPN 4 ──► Digit 4 (Ones)

Timing:
   - Each digit ON for 1ms
   - Full cycle = 4ms
   - Scan rate = ~250 Hz
   - Smooth continuous display!

NPN Transistor (2N2222 / BC547):
   Base ─── R 10kΩ ─── ESP32 GPIO
   Collector ─── Digit Common Cathode
   Emitter ─── GND
```

---

## 🔌 Complete Circuit Diagram - Smart Counter

```
                    ┌─────────────────┐
                    │         │
                    │    (ESP32)      │
                    └─────────────────┘
                           │ │
        ┌──────────────────┘ └──────────────────┐
        │                                       │
    GPIO 25 (CLK)                          GPIO 26 (RST)
        │                                       │
        ▼                                       ▼
    ┌────────────┐                          Reset pins
    │   7493     │                          (tied together)
    │  Counter   │                              │
    │            │◄─────────────────────────────┘
    └────────────┘
      │ │ │ │
      Q0 Q1 Q2 Q3 (BCD output)
      │ │ │ │
      │ │ │ └─────────────┐
      │ │ └───────────┐   │
      │ └───────┐     │   │
      │         │     │   │
      ▼         ▼     ▼   ▼
    ┌─────────────────────────┐
    │      CD4511             │
    │  BCD-to-7-Segment       │
    │      Decoder            │
    └─────────────────────────┘
      │ │ │ │ │ │ │
      a b c d e f g (segments)
      │ │ │ │ │ │ │
      └─┬─┴─┬─┴─┬─┴─┬───► Each through 220Ω
        ▼   ▼   ▼   ▼
    ┌─────────────────────┐
    │   7-Segment Display │
    │   (Common Cathode)  │
    └─────────────────────┘
            │
            ▼
           GND
```

### Button Connections

```
GPIO 4 ────┬────► COUNT button ──► GND
           └────► 10kΩ Pull-up to VCC (optional, ESP32 has internal)

GPIO 5 ────┬────► RESET button ──► GND
           └────► 10kΩ Pull-up to VCC (optional)
```

---

## ⚠️ Important Notes

### Voltage Levels

- **ESP32 Logic Level:** 3.3V
- **74xx Series (TTL):** Designed for 5V but most work with 3.3V inputs
  - VIH (min): ~2.0V (3.3V is sufficient)
  - VOH (typical): 3.4V @ VCC=5V
- **CD4xxx Series (CMOS):** Work with both 3.3V and 5V

### Recommendations

1. **For reliable operation:** Use 5V for IC VCC, but ESP32 outputs (3.3V) can drive inputs
2. **For LED brightness:** 5V gives better 7-segment brightness through CD4511
3. **Level shifting:** If reading outputs back to ESP32, ensure they don't exceed 3.3V
4. **Power supply:** Use external 5V supply for ICs, share common GND with ESP32

### Clock Speed Limits

- Maximum clock frequency: **1000 Hz (1 kHz)** recommended
- For stable operation with mechanical switches: **1-10 Hz**
- 7493 can handle much higher frequencies, but ESP32 GPIO toggling limits apply

---

## 🎓 Educational Value

### Why Combine Digital Logic with Microcontrollers?

1. **Understanding Fundamentals:** ดิจิทัล logic เป็นพื้นฐานของ microcontrollers
2. **Cost-Effective Solutions:** IC ดิจิทัลบางอย่างถูกกว่าใช้ MCU ทำงานเดียวกัน
3. **Real-World Applications:** ระบบจริงมักผสมผสาน hardware logic กับ software control
4. **Hands-On Learning:** เห็นสัญญาณดิจิทัลจริง ๆ ไม่ใช่แค่ simulation

### Skills Gained

- ✅ อ่าน datasheet ของ IC ดิจิทัล
- ✅ ต่อวงจรดิจิทัลให้ทำงานร่วมกับ MCU
- ✅ เข้าใจ timing, clock, และ pulse generation
- ✅ ใช้ oscilloscope/logic analyzer (ถ้ามี)
- ✅ Debugging ระบบ hardware-software hybrid

---

## 🔍 Troubleshooting

### Problem: 7-Segment ไม่แสดงผล

**Solution:**
1. ตรวจสอบว่าใช้ Common Cathode (CD4511 ออกแบบสำหรับ CC)
2. ตรวจสอบการต่อ resistors ที่ segments (220Ω แนะนำ)
3. วัดแรงดันที่ขา VCC ของ CD4511 (ควรเป็น 5V หรือ 3.3V)
4. ตรวจสอบ control pins: LT=HIGH, BI=HIGH, LE=LOW

### Problem: Counter นับไม่ถูกต้อง

**Solution:**
1. ตรวจสอบ clock signal ด้วย LED หรือ oscilloscope
2. ตรวจสอบ reset pins ของ 7493 (ต้องต่อ R0(1) และ R0(2) together)
3. ลดความเร็ว clock ลง (เริ่มที่ 1 Hz)
4. ตรวจสอบการต่อสาย Q0-Q3 ถูกต้อง (ดูที่ pinout)

### Problem: Logic Gate ไม่ทำงาน

**Solution:**
1. ตรวจสอบ pin VCC (pin 14 สำหรับ 14-pin ICs) และ GND (pin 7)
2. ตรวจสอบว่าใช้ input pins ถูกต้อง (ดู datasheet)
3. unused inputs ของ gates อื่นควรต่อ GND หรือ VCC (ไม่ปล่อยลอย)
4. วัดแรงดันที่ output pin ด้วย multimeter

### Problem: ESP32 reset หรือค้างเมื่อเชื่อมต่อ IC

**Solution:**
1. ใช้ external 5V power supply สำหรับ ICs (ไม่ดึงจาก ESP32)
2. ต่อ GND ร่วมกันระหว่าง ESP32 และ IC power supply
3. ใส่ decoupling capacitor (0.1µF) ใกล้ VCC pin ของแต่ละ IC
4. ตรวจสอบไม่มี short circuit

### Problem: Multiplexed Display กระพริบ หรือ แสดงผลไม่ชัด

**Solution:**
1. **ต้องเรียก `display.update()` ใน `loop()` บ่อย ๆ!** - นี่คือสาเหตุหลักที่สุด
2. ห้ามใช้ `delay()` แบบยาวใน loop (จะทำให้การสแกนหยุดชะงัก)
3. ตรวจสอบทรานซิสเตอร์ NPN ทำงานถูกต้อง
   - Base → R 10kΩ → ESP32 GPIO
   - Collector → Digit common cathode
   - Emitter → GND
4. ตรวจสอบว่า Common Cathode/Anode ตรงกับที่ตั้งค่าใน code
5. ลด scan interval ถ้ากระพริบ: `display.setScanInterval(700);` (microseconds)

### Problem: Multiplexed Display แสดงตัวเลขผิด หรือ ซ้อนกัน

**Solution:**
1. ตรวจสอบลำดับการต่อ digit pins (Digit 1-4 ต้องตรงกับ code)
2. ตรวจสอบลำดับ segment pins (a-g ต้องตรงกับ code)
3. ตรวจสอบว่าไม่มีขาสั้น (short) ระหว่าง segments
4. ลอง `display.setLeadingZeros(true);` เพื่อ debug ง่ายขึ้น
5. แสดงเลขง่าย ๆ เช่น 0, 1, 8 เพื่อทดสอบ segments แต่ละตัว

### Problem: Multiplexed Display สว่างไม่เท่ากัน แต่ละหลัก

**Solution:**
1. ใส่ resistor ที่ segments แทนที่จะใส่ที่ digit control
2. ตรวจสอบทรานซิสเตอร์ทุกตัวเป็นรุ่นเดียวกัน (2N2222 หรือ BC547)
3. ใช้ current limiting resistor ที่เหมาะสม (220Ω แนะนำ)
4. ถ้ายังไม่เท่า อาจต้องปรับ scan interval ให้เร็วขึ้น

### Problem: Temperature Display แสดงค่า 8888 ตลอด

**Solution:**
1. DHT sensor ไม่ทำงาน - ตรวจสอบการต่อสาย (VCC, DATA, GND)
2. ต้องมี pull-up resistor 10kΩ ที่ DATA pin → VCC
3. รอสักครู่หลัง power on (DHT ต้องการเวลาเริ่มต้น ~1-2 วินาที)
4. ตรวจสอบว่าติดตั้ง DHT library แล้ว (Adafruit DHT)

---

## 📖 References

### Datasheets
- [7493 - 4-Bit Binary Counter](https://www.ti.com/lit/ds/symlink/sn7493.pdf)
- [CD4511 - BCD to 7-Segment Decoder](https://www.ti.com/lit/ds/symlink/cd4511b.pdf)
- [7408 - Quad AND Gate](https://www.ti.com/lit/ds/symlink/sn7408.pdf)
- [7404 - Hex Inverter](https://www.ti.com/lit/ds/symlink/sn7404.pdf)

### Resources
- [Digital Logic Tutorial](https://www.electronics-tutorials.ws/logic/logic_1.html)
- [7-Segment Display Guide](https://www.electronicshub.org/seven-segment-display/)
- [Multiplexing 7-Segment Displays](https://www.electronics-tutorials.ws/blog/7-segment-display-tutorial.html)
- [ESP32 GPIO Reference](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)

---


####  Logic Gates
- ทฤษฎี: Logic levels, Truth tables
- ปฏิบัติ: ต่อ IC gates + ทดสอบด้วย ESP32
- แบบฝึกหัด: เขียน truth table และออกแบบวงจรเงื่อนไข

#### Counter + 7-Segment
- ทฤษฎี: Binary counting, BCD encoding
- ปฏิบัติ: ต่อ 7493 → CD4511 → 7-segment
- โปรเจค: Smart Counter with buttons

#### Multiplexing 2-Digit
- ทฤษฎี: Multiplexing concept, POV (Persistence of Vision)
- ปฏิบัติ: ต่อ 2-digit display + ทรานซิสเตอร์
- เน้น: Non-blocking programming with `update()`

#### Multiplexing 4-Digit
- ปฏิบัติ: ขยายเป็น 4-digit
- โปรเจค: Digital Clock หรือ Temperature Display
- Integration: รวมกับ DHT sensor library

### จุดเน้นก
- เน้นให้เด็กเข้าใจว่า microcontroller ไม่ใช่ทดแทน digital logic แต่**ทำงานร่วมกัน**
- ให้เด็กได้ใช้ oscilloscope/logic analyzer ดู clock signal (ถ้ามี)
- ส่งเสริมให้เด็กอ่าน datasheet และเข้าใจ timing diagram

#### 
- **แนวคิด Multiplexing:** อธิบายว่าทำไมต้องสแกนเร็ว (POV - Persistence of Vision)
- **Non-blocking Programming:** สอนให้เด็กเข้าใจว่า `delay()` ทำให้ display กระพริบ
- **Hardware Control:** ให้เด็กเห็นการทำงานของทรานซิสเตอร์เป็น "สวิตช์ดิจิทัล"
- **Optimization:** ลด GPIO usage จาก 14 pins → 9 pins (2-digit) หรือ 28 pins → 11 pins (4-digit)


### ข้อควรระวัง

- **Safety First:** สอนให้เด็กตรวจ VCC/GND ก่อนเปิดไฟทุกครั้ง
- **Transistor Polarity:** NPN ต้องต่อถูก (E→GND, C→Load, B←Signal)
- **Current Limiting:** ต้องมี resistor ที่ segments เสมอ
- **Common Cathode vs Anode:** ต้องสอนให้เด็กแยกได้


---

## 📜 License

This library is released under the MIT License.

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

---


#### Part 1: ทฤษฎี
- Logic gates พื้นฐาน และ truth tables
- Counter และ sequential logic
- BCD และการแสดงผลบน 7-segment

#### Part 2: ปฏิบัติการ
- ต่อวงจร logic gates และทดสอบ
- ต่อ 7493 binary counter
- ต่อ CD4511 กับ 7-segment display

#### Part 3: โปรเจค
- ประกอบ Smart Counter system
- ทดสอบและแก้ปัญหา
- นำเสนอผลงาน

### จุดเน้น
- เน้นให้เด็กเข้าใจว่า microcontroller ไม่ใช่ทดแทน digital logic แต่ **ทำงานร่วมกัน**
- ให้เด็กได้ใช้ oscilloscope/logic analyzer ดู clock signal (ถ้ามี)
- ส่งเสริมให้เด็กอ่าน datasheet และเข้าใจ timing diagram

---

**สนุกกับการเรียนรู้ Digital Logic + Microcontroller! 🚀**
