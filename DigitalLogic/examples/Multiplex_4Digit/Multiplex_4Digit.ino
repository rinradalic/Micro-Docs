/**
 * 4-Digit 7-Segment Multiplexing
 * 
 * Multiplexing สำหรับขับ 7-segment 4 หลัก
 * โดยใช้ขา GPIO เพียง 11 ขา (7 segments + 4 digit control)
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - 7-segment display 4 หลัก แบบ Common Cathode
 * - ทรานซิสเตอร์ NPN 4 ตัว (2N2222 หรือ BC547)
 * - ตัวต้านทาน 220Ω จำนวน 7 ตัว (สำหรับ segments)
 * - ตัวต้านทาน 10kΩ จำนวน 4 ตัว (สำหรับ base ของทรานซิสเตอร์)
 * - ปุ่มกด 3 ตัว
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร:
 * 
 * Segments (ใช้ร่วมกันทั้ง 4 หลัก):
 *   GPIO 14 ──► a ──► R 220Ω ──► segment a ของทั้ง 4 หลัก
 *   GPIO 27 ──► b ──► R 220Ω ──► segment b
 *   GPIO 26 ──► c ──► R 220Ω ──► segment c
 *   GPIO 25 ──► d ──► R 220Ω ──► segment d
 *   GPIO 33 ──► e ──► R 220Ω ──► segment e
 *   GPIO 32 ──► f ──► R 220Ω ──► segment f
 *   GPIO 23 ──► g ──► R 220Ω ──► segment g
 * 
 * Digit Control (ควบคุมแยกแต่ละหลัก):
 *   GPIO 22 ──► R 10kΩ ──► Base NPN 1 ──► Digit1 (Thousands)
 *   GPIO 21 ──► R 10kΩ ──► Base NPN 2 ──► Digit2 (Hundreds)
 *   GPIO 19 ──► R 10kΩ ──► Base NPN 3 ──► Digit3 (Tens)
 *   GPIO 18 ──► R 10kΩ ──► Base NPN 4 ──► Digit4 (Ones)
 * 
 * Buttons:
 *   GPIO 4 ──► SW UP ──► GND
 *   GPIO 5 ──► SW MODE ──► GND
 *   GPIO 17 ──► SW RESET ──► GND
 * 
 * หลักการ Multiplexing 4-Digit:
 * - เปิดทีละหลัก (D1→D2→D3→D4) แต่ละหลัก 1ms
 * - รอบเต็ม = 4ms → ความถี่ 250 Hz
 * - ตาคนมองไม่ทัน เห็นเป็นภาพนิ่ง
 * 
 * โหมดการทำงาน:
 * 1. Counter Mode - นับ 0000-9999
 * 2. Stopwatch Mode - จับเวลาเป็นวินาที
 * 3. Countdown Mode - นับถอยหลัง
 */

#include <DigitalLogic.h>

// Pin definitions - Segments
const int SEG_A = 14;
const int SEG_B = 27;
const int SEG_C = 26;
const int SEG_D = 25;
const int SEG_E = 33;
const int SEG_F = 32;
const int SEG_G = 23;

// Pin definitions - Digit Control
const int DIGIT_1 = 22;  // Thousands
const int DIGIT_2 = 21;  // Hundreds
const int DIGIT_3 = 19;  // Tens
const int DIGIT_4 = 18;  // Ones

// Pin definitions - Buttons
const int BTN_UP = 4;
const int BTN_MODE = 5;
const int BTN_RESET = 17;

// Create 4-digit display object
SevenSegmentMultiplex display(
  SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G,
  DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4,
  true  // Common Cathode
);

// Mode enumeration
enum DisplayMode {
  MODE_COUNTER,
  MODE_STOPWATCH,
  MODE_COUNTDOWN
};

DisplayMode currentMode = MODE_COUNTER;
const char* modeNames[] = {"Counter", "Stopwatch", "Countdown"};

// Variables
int value = 0;
bool running = false;
unsigned long lastUpdate = 0;
int updateInterval = 100; // ms
int countdownStart = 30; // countdown starting value

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("============================================");
  Serial.println("  4-Digit 7-Segment Multiplexing");
  Serial.println("============================================\n");
  
  // Initialize display
  display.begin();
  display.setLeadingZeros(false);  // Hide leading zeros initially
  
  // Initialize buttons
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  Serial.println("✓ 4-digit display initialized");
  Serial.println("  Range: 0000-9999");
  Serial.println("  Scan rate: ~250 Hz\n");
  
  Serial.println("Controls:");
  Serial.println("  Button UP (GPIO 4)    - Start/Stop or Increment");
  Serial.println("  Button MODE (GPIO 5)  - Switch mode");
  Serial.println("  Button RESET (GPIO 17)- Reset value\n");
  
  Serial.println("Modes:");
  Serial.println("  1. Counter   - Count up/down");
  Serial.println("  2. Stopwatch - Time in seconds (00.00)");
  Serial.println("  3. Countdown - Count down from 30\n");
  
  Serial.println("Serial Commands:");
  Serial.println("  s - Start/Stop");
  Serial.println("  r - Reset");
  Serial.println("  m - Change mode");
  Serial.println("  + - Increase speed");
  Serial.println("  - - Decrease speed");
  Serial.println("  z - Toggle leading zeros");
  Serial.println("  i - Show info\n");
  
  printMode();
  display.showNumber(value);
}

void loop() {
  // CRITICAL: Must call update() frequently for smooth display
  display.update();
  
  // Check buttons
  checkButtons();
  
  // Update based on mode
  if (running) {
    unsigned long now = millis();
    if (now - lastUpdate >= updateInterval) {
      lastUpdate = now;
      updateValue();
    }
  }
  
  // Check serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
}

void checkButtons() {
  // UP button - Start/Stop or Increment
  static bool lastUpState = HIGH;
  bool upState = digitalRead(BTN_UP);
  
  if (upState == LOW && lastUpState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_UP) == LOW) {
      if (currentMode == MODE_COUNTER && !running) {
        // Manual increment in counter mode
        value++;
        if (value > 9999) value = 0;
        display.showNumber(value);
        printValue();
      } else {
        // Toggle start/stop for other modes
        toggleRunning();
      }
    }
  }
  lastUpState = upState;
  
  // MODE button
  static bool lastModeState = HIGH;
  bool modeState = digitalRead(BTN_MODE);
  
  if (modeState == LOW && lastModeState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_MODE) == LOW) {
      changeMode();
    }
  }
  lastModeState = modeState;
  
  // RESET button
  static bool lastResetState = HIGH;
  bool resetState = digitalRead(BTN_RESET);
  
  if (resetState == LOW && lastResetState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_RESET) == LOW) {
      resetValue();
    }
  }
  lastResetState = resetState;
}

void updateValue() {
  switch (currentMode) {
    case MODE_COUNTER:
      value++;
      if (value > 9999) value = 0;
      break;
      
    case MODE_STOPWATCH:
      value++;
      if (value > 9999) {
        value = 0;
        Serial.println("⏱ Stopwatch overflow! Reset to 0");
      }
      break;
      
    case MODE_COUNTDOWN:
      value--;
      if (value <= 0) {
        value = 0;
        running = false;
        Serial.println("⏰ Countdown finished!");
        // Blink display
        for (int i = 0; i < 3; i++) {
          display.clear();
          delay(200);
          display.showNumber(0);
          delay(200);
        }
      }
      break;
  }
  
  display.showNumber(value);
  printValue();
}

void toggleRunning() {
  running = !running;
  Serial.print(running ? "▶ " : "■ ");
  Serial.print(modeNames[currentMode]);
  Serial.println(running ? " STARTED" : " STOPPED");
}

void changeMode() {
  running = false;
  currentMode = (DisplayMode)((currentMode + 1) % 3);
  resetValue();
  printMode();
}

void resetValue() {
  running = false;
  
  switch (currentMode) {
    case MODE_COUNTER:
      value = 0;
      break;
    case MODE_STOPWATCH:
      value = 0;
      break;
    case MODE_COUNTDOWN:
      value = countdownStart;
      break;
  }
  
  display.showNumber(value);
  Serial.println("⟲ RESET");
  printValue();
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 's':
    case 'S':
      toggleRunning();
      break;
      
    case 'r':
    case 'R':
      resetValue();
      break;
      
    case 'm':
    case 'M':
      changeMode();
      break;
      
    case '+':
      updateInterval -= 50;
      if (updateInterval < 10) updateInterval = 10;
      Serial.print("⚡ Speed: ");
      Serial.print(updateInterval);
      Serial.println(" ms");
      break;
      
    case '-':
      updateInterval += 50;
      if (updateInterval > 2000) updateInterval = 2000;
      Serial.print("🐢 Speed: ");
      Serial.print(updateInterval);
      Serial.println(" ms");
      break;
      
    case 'z':
    case 'Z':
      {
        static bool leadingZeros = false;
        leadingZeros = !leadingZeros;
        display.setLeadingZeros(leadingZeros);
        Serial.print("Leading zeros: ");
        Serial.println(leadingZeros ? "ON" : "OFF");
      }
      break;
      
    case 'i':
    case 'I':
      showInfo();
      break;
  }
}

void printMode() {
  Serial.println("\n┌────────────────────────┐");
  Serial.print("│ Mode: ");
  Serial.print(modeNames[currentMode]);
  for (int i = strlen(modeNames[currentMode]); i < 16; i++) {
    Serial.print(" ");
  }
  Serial.println("│");
  Serial.println("└────────────────────────┘");
}

void printValue() {
  if (currentMode == MODE_STOPWATCH) {
    // Display as XX.XX format (seconds with decimals)
    int secs = value / 100;
    int decimals = value % 100;
    Serial.print("⏱ ");
    if (secs < 10) Serial.print("0");
    Serial.print(secs);
    Serial.print(".");
    if (decimals < 10) Serial.print("0");
    Serial.println(decimals);
  } else {
    Serial.print("Value: ");
    Serial.println(value);
  }
}

void showInfo() {
  Serial.println("\n========== Display Info ==========");
  Serial.print("Mode: ");
  Serial.println(modeNames[currentMode]);
  Serial.print("Current Value: ");
  Serial.println(value);
  Serial.print("Running: ");
  Serial.println(running ? "YES" : "NO");
  Serial.print("Update Interval: ");
  Serial.print(updateInterval);
  Serial.println(" ms");
  Serial.println("\nMultiplexing Info:");
  Serial.println("  Digits: 4");
  Serial.println("  Scan interval: 1000 μs per digit");
  Serial.println("  Full cycle: 4 ms");
  Serial.println("  Scan rate: ~250 Hz");
  Serial.println("  GPIO used: 11 pins");
  Serial.println("    - 7 segment pins");
  Serial.println("    - 4 digit control pins");
  Serial.println("==================================\n");
}
