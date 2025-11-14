/*
 * Button_LED_5Modes
 * 
 * Integration: Button + LEDController
 * ควบคุม LED 5 โหมดด้วยปุ่มกด
 * 
 * Features:
 * - ปุ่ม MODE: สลับโหมด LED (5 โหมด)
 * - ปุ่ม SPEED: ปรับความเร็วกระพริบ
 * - แสดงโหมดปัจจุบันบน Serial
 * 
 * Hardware:
 * - Button MODE:  Pin 4 → Button → GND (INPUT_PULLUP)
 * - Button SPEED: Pin 5 → Button → GND (INPUT_PULLUP)
 * - LED 1: Pin 12 → 220Ω → LED → GND
 * - LED 2: Pin 13 → 220Ω → LED → GND (Built-in on UNO)
 * - LED 3: Pin 14 → 220Ω → LED → GND
 * - LED 4: Pin 15 → 220Ω → LED → GND
 * - LED 5: Pin 16 → 220Ω → LED → GND
 * 
 * 5 Modes:
 * 1. ALL OFF     - ปิดทุกดวง
 * 2. ALL ON      - เปิดทุกดวง
 * 3. RUNNING     - วิ่งทีละดวง (1→2→3→4→5→1...)
 * 4. BLINK ALL   - กระพริบทุกดวงพร้อมกัน
 * 5. ALTERNATING - สลับกัน (1,3,5 ON | 2,4 ON)
 * 
 * SPEED Button:
 * - กดเพื่อสลับความเร็ว: SLOW (500ms) → MEDIUM (250ms) → FAST (100ms)
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <Button.h>
#include <LEDController.h>

// ==================== Configuration ====================
#if defined(ESP32)
  const int BTN_MODE = 4;
  const int BTN_SPEED = 5;
  const int LED_PINS[] = {12, 13, 14, 15, 16};
#else
  const int BTN_MODE = 4;
  const int BTN_SPEED = 5;
  const int LED_PINS[] = {8, 9, 10, 11, 12};
#endif

const int NUM_LEDS = 5;

// ==================== Objects ====================
Button btnMode(BTN_MODE);
Button btnSpeed(BTN_SPEED);

LEDController leds[NUM_LEDS] = {
  LEDController(LED_PINS[0]),
  LEDController(LED_PINS[1]),
  LEDController(LED_PINS[2]),
  LEDController(LED_PINS[3]),
  LEDController(LED_PINS[4])
};

// ==================== Variables ====================
enum Mode {
  MODE_ALL_OFF = 0,
  MODE_ALL_ON,
  MODE_RUNNING,
  MODE_BLINK_ALL,
  MODE_ALTERNATING,
  MODE_COUNT  // จำนวนโหมดทั้งหมด
};

Mode currentMode = MODE_ALL_OFF;

enum Speed {
  SPEED_SLOW = 0,    // 500ms
  SPEED_MEDIUM,      // 250ms
  SPEED_FAST,        // 100ms
  SPEED_COUNT
};

Speed currentSpeed = SPEED_MEDIUM;

const unsigned long SPEED_VALUES[] = {500, 250, 100};
const char* SPEED_NAMES[] = {"SLOW", "MEDIUM", "FAST"};
const char* MODE_NAMES[] = {
  "ALL OFF",
  "ALL ON",
  "RUNNING",
  "BLINK ALL",
  "ALTERNATING"
};

// สำหรับโหมด RUNNING
int runningIndex = 0;
unsigned long lastRunningUpdate = 0;

// สำหรับโหมด ALTERNATING
bool alternatingState = false;
unsigned long lastAlternatingUpdate = 0;

// ==================== Helper Functions ====================

void allOff() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].off();
  }
}

void allOn() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].on();
  }
}

void updateRunning() {
  unsigned long now = millis();
  unsigned long interval = SPEED_VALUES[currentSpeed];
  
  if (now - lastRunningUpdate >= interval) {
    lastRunningUpdate = now;
    
    // ปิดทุกดวงก่อน
    allOff();
    
    // เปิดดวงปัจจุบัน
    leds[runningIndex].on();
    
    // เลื่อนไปดวงถัดไป
    runningIndex = (runningIndex + 1) % NUM_LEDS;
  }
}

void updateBlinkAll() {
  // ใช้ startBlink() ของ LEDController
  unsigned long interval = SPEED_VALUES[currentSpeed];
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].startBlink(interval);
  }
}

void updateAlternating() {
  unsigned long now = millis();
  unsigned long interval = SPEED_VALUES[currentSpeed];
  
  if (now - lastAlternatingUpdate >= interval) {
    lastAlternatingUpdate = now;
    alternatingState = !alternatingState;
    
    // สลับกัน: 0,2,4 vs 1,3
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i % 2 == 0) {
        // เลขคู่ (0,2,4)
        if (alternatingState) {
          leds[i].on();
        } else {
          leds[i].off();
        }
      } else {
        // เลขคี่ (1,3)
        if (alternatingState) {
          leds[i].off();
        } else {
          leds[i].on();
        }
      }
    }
  }
}

void applyMode() {
  // ปิด blinking ทุกดวงก่อน
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].stopBlink();
  }
  
  switch (currentMode) {
    case MODE_ALL_OFF:
      allOff();
      break;
      
    case MODE_ALL_ON:
      allOn();
      break;
      
    case MODE_RUNNING:
      runningIndex = 0;
      lastRunningUpdate = millis();
      allOff();
      leds[0].on();
      break;
      
    case MODE_BLINK_ALL:
      updateBlinkAll();
      break;
      
    case MODE_ALTERNATING:
      alternatingState = false;
      lastAlternatingUpdate = millis();
      // เริ่มต้น: 0,2,4 ON, 1,3 OFF
      for (int i = 0; i < NUM_LEDS; i++) {
        if (i % 2 == 0) {
          leds[i].on();
        } else {
          leds[i].off();
        }
      }
      break;
  }
  
  printStatus();
}

void printStatus() {
  Serial.println("╔════════════════════════════════════════╗");
  Serial.print("║  Mode:  ");
  Serial.print(MODE_NAMES[currentMode]);
  for (int i = 0; i < 28 - strlen(MODE_NAMES[currentMode]); i++) {
    Serial.print(" ");
  }
  Serial.println("║");
  Serial.print("║  Speed: ");
  Serial.print(SPEED_NAMES[currentSpeed]);
  Serial.print(" (");
  Serial.print(SPEED_VALUES[currentSpeed]);
  Serial.print("ms)");
  for (int i = 0; i < 15 - strlen(SPEED_NAMES[currentSpeed]); i++) {
    Serial.print(" ");
  }
  Serial.println("║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  Button + LEDController: 5 I/O Modes");
  Serial.println("===============================================");
  Serial.println();
  
  // Board info
  #if defined(ESP32)
    Serial.println("Board: ESP32");
  #else
    Serial.println("Board: Arduino");
  #endif
  
  Serial.println();
  Serial.println("Hardware:");
  Serial.print("  MODE button:  Pin ");
  Serial.println(BTN_MODE);
  Serial.print("  SPEED button: Pin ");
  Serial.println(BTN_SPEED);
  Serial.print("  LEDs:         Pins ");
  for (int i = 0; i < NUM_LEDS; i++) {
    Serial.print(LED_PINS[i]);
    if (i < NUM_LEDS - 1) Serial.print(", ");
  }
  Serial.println();
  Serial.println();
  
  Serial.println("Controls:");
  Serial.println("  MODE button:  Switch LED mode");
  Serial.println("  SPEED button: Change speed (SLOW/MEDIUM/FAST)");
  Serial.println();
  
  Serial.println("Modes:");
  Serial.println("  1. ALL OFF     - All LEDs off");
  Serial.println("  2. ALL ON      - All LEDs on");
  Serial.println("  3. RUNNING     - Running light (1→2→3→4→5)");
  Serial.println("  4. BLINK ALL   - All LEDs blink together");
  Serial.println("  5. ALTERNATING - Alternating pattern");
  Serial.println();
  
  // Initialize
  btnMode.begin();
  btnSpeed.begin();
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].begin();
  }
  
  Serial.println("✓ System ready");
  Serial.println();
  
  applyMode();
}

// ==================== Loop ====================
void loop() {
  // Update buttons
  btnMode.update();
  btnSpeed.update();
  
  // =============================================
  // MODE Button - สลับโหมด
  // =============================================
  if (btnMode.wasPressed()) {
    currentMode = (Mode)((currentMode + 1) % MODE_COUNT);
    applyMode();
  }
  
  // =============================================
  // SPEED Button - เปลี่ยนความเร็ว
  // =============================================
  if (btnSpeed.wasPressed()) {
    currentSpeed = (Speed)((currentSpeed + 1) % SPEED_COUNT);
    
    Serial.print("⚡ Speed changed to: ");
    Serial.print(SPEED_NAMES[currentSpeed]);
    Serial.print(" (");
    Serial.print(SPEED_VALUES[currentSpeed]);
    Serial.println("ms)");
    Serial.println();
    
    // อัปเดต blink rate ถ้าอยู่ในโหมด BLINK_ALL
    if (currentMode == MODE_BLINK_ALL) {
      updateBlinkAll();
    }
  }
  
  // =============================================
  // Update LEDs ตามโหมด
  // =============================================
  switch (currentMode) {
    case MODE_RUNNING:
      updateRunning();
      break;
      
    case MODE_ALTERNATING:
      updateAlternating();
      break;
      
    case MODE_BLINK_ALL:
      // LEDController จัดการ blinking เอง
      break;
      
    default:
      // MODE_ALL_OFF, MODE_ALL_ON ไม่ต้องทำอะไร
      break;
  }
  
  // อัปเดต LEDController (สำหรับ blinking)
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].update();
  }
}

/*
 * ==================== Mode Details ====================
 * 
 * 1. ALL OFF:
 *    LED: ○ ○ ○ ○ ○
 *    
 * 2. ALL ON:
 *    LED: ● ● ● ● ●
 *    
 * 3. RUNNING:
 *    Step 1: ● ○ ○ ○ ○
 *    Step 2: ○ ● ○ ○ ○
 *    Step 3: ○ ○ ● ○ ○
 *    Step 4: ○ ○ ○ ● ○
 *    Step 5: ○ ○ ○ ○ ●
 *    (repeat)
 *    
 * 4. BLINK ALL:
 *    State 1: ● ● ● ● ●
 *    State 2: ○ ○ ○ ○ ○
 *    (toggle based on speed)
 *    
 * 5. ALTERNATING:
 *    State 1: ● ○ ● ○ ●
 *    State 2: ○ ● ○ ● ○
 *    (toggle based on speed)
 * 
 * ==================== Speed Settings ====================
 * 
 * SLOW:   500ms - ช้า เห็นชัดเจน
 * MEDIUM: 250ms - ปานกลาง (default)
 * FAST:   100ms - เร็ว สะดุดตา
 * 
 * ==================== Wiring ====================
 * 
 * Buttons (INPUT_PULLUP):
 *   Pin → [Button] → GND
 *   
 * LEDs:
 *   Pin → [220Ω Resistor] → [LED Anode(+)] → [LED Cathode(-)] → GND
 *   
 * ⚠️ ตรวจสอบ LED polarity:
 *   - ขายาว = Anode (+)
 *   - ขาสั้น = Cathode (-)
 * 
 * ==================== Extensions ====================
 * 
 * เพิ่มโหมดใหม่:
 * 1. เพิ่ม enum Mode ใหม่
 * 2. เพิ่มชื่อใน MODE_NAMES[]
 * 3. เพิ่ม case ใน applyMode()
 * 4. เขียน function อัปเดต (ถ้าจำเป็น)
 * 
 * ตัวอย่างโหมดเพิ่มเติม:
 * - WAVE: คลื่น (fade in/out)
 * - RANDOM: สุ่มติด/ดับ
 * - BINARY: แสดงเลขไบนารี
 * - BREATHING: หายใจ (fade smooth)
 * - KNIGHT RIDER: วิ่งไป-กลับ
 * 
 * ==================== Troubleshooting ====================
 * 
 * LED ไม่ติด:
 * - ตรวจสอบ polarity (ขายาว = +)
 * - ตรวจสอบ resistor (220Ω)
 * - ลอง digitalWrite(pin, HIGH) ตรง ๆ
 * 
 * LED สว่างน้อย:
 * - ใช้ resistor เล็กลง (150Ω)
 * - ตรวจสอบแรงดัน (5V สำหรับ LED ทั่วไป)
 * 
 * ปุ่มไม่ตอบสนอง:
 * - ตรวจสอบการต่อ: Pin → Button → GND
 * - เปิด Serial Monitor ดู
 * - ลองเพิ่ม debounce time
 * 
 * โหมดไม่ทำงาน:
 * - ตรวจสอบ switch case
 * - ดู Serial output
 * - ลองกด MODE button หลาย ๆ ครั้ง
 */
