/**
 * 2-Digit 7-Segment Multiplexing
 * 
 * Multiplexing สำหรับขับ 7-segment 2 หลัก
 * โดยใช้ขา GPIO เพียง 9 ขา (7 segments + 2 digit control)
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - 7-segment display 2 หลัก แบบ Common Cathode
 * - ทรานซิสเตอร์ NPN 2 ตัว (2N2222 หรือ BC547)
 * - ตัวต้านทาน 220Ω จำนวน 7 ตัว (สำหรับ segments)
 * - ตัวต้านทาน 10kΩ จำนวน 2 ตัว (สำหรับ base ของทรานซิสเตอร์)
 * - ปุ่มกด 3 ตัว (UP, DOWN, RESET)
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร:
 * 
 * Segments (ใช้ร่วมกันทั้ง 2 หลัก):
 *   GPIO 14 ──► a ──► R 220Ω ──► segment a ของทั้ง 2 หลัก
 *   GPIO 27 ──► b ──► R 220Ω ──► segment b
 *   GPIO 26 ──► c ──► R 220Ω ──► segment c
 *   GPIO 25 ──► d ──► R 220Ω ──► segment d
 *   GPIO 33 ──► e ──► R 220Ω ──► segment e
 *   GPIO 32 ──► f ──► R 220Ω ──► segment f
 *   GPIO 23 ──► g ──► R 220Ω ──► segment g
 * 
 * Digit Control (ควบคุมแยกแต่ละหลัก):
 *   GPIO 22 ──► R 10kΩ ──► Base NPN 1 ──► Collector to Digit1 Cathode
 *   GPIO 21 ──► R 10kΩ ──► Base NPN 2 ──► Collector to Digit2 Cathode
 *   (Emitter ของ NPN ทั้งคู่ต่อ GND)
 * 
 * Buttons:
 *   GPIO 4 ──► SW UP ──► GND (ใช้ internal pullup)
 *   GPIO 5 ──► SW DOWN ──► GND
 *   GPIO 18 ──► SW RESET ──► GND
 * 
 * หลักการ Multiplexing:
 * - เปิดทีละหลัก (Digit1 ON 1ms, Digit2 ON 1ms)
 * - สแกนเร็วมาก (~500 Hz) จนตาคนมองเป็นภาพนิ่ง
 * - ใช้ GPIO น้อยลง (9 ขา แทนที่จะเป็น 14 ขา)
 * 
 * คุณสมบัติ:
 * - แสดงเลข 00-99
 * - ปุ่ม UP เพิ่มค่า
 * - ปุ่ม DOWN ลดค่า
 * - ปุ่ม RESET กลับไป 0
 * - นับอัตโนมัติ (เปิด/ปิดได้)
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
const int DIGIT_1 = 22;  // Tens digit
const int DIGIT_2 = 21;  // Ones digit

// Pin definitions - Buttons
const int BTN_UP = 4;
const int BTN_DOWN = 5;
const int BTN_RESET = 18;

// Create 2-digit display object
SevenSegmentMultiplex display(
  SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G,
  DIGIT_1, DIGIT_2,
  true  // Common Cathode
);

// Counter variables
int counter = 0;
bool autoCount = false;
unsigned long lastAutoCount = 0;
int autoSpeed = 500; // ms

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("============================================");
  Serial.println("  2-Digit 7-Segment Multiplexing");
  Serial.println("============================================\n");
  
  // Initialize display
  display.begin();
  display.setLeadingZeros(true);  // Show leading zeros (00-09)
  
  // Initialize buttons
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  Serial.println("✓ 2-digit display initialized");
  Serial.println("  Range: 00-99");
  Serial.println("  Scan rate: ~1000 Hz\n");
  
  Serial.println("Controls:");
  Serial.println("  Button UP (GPIO 4)    - Increment");
  Serial.println("  Button DOWN (GPIO 5)  - Decrement");
  Serial.println("  Button RESET (GPIO 18)- Reset to 0\n");
  
  Serial.println("Serial Commands:");
  Serial.println("  a - Toggle auto count");
  Serial.println("  + - Speed up auto count");
  Serial.println("  - - Slow down auto count");
  Serial.println("  z - Toggle leading zeros");
  Serial.println("  i - Show info\n");
  
  // Show initial value
  display.showNumber(counter);
  printCounter();
}

void loop() {
  // CRITICAL: Must call update() frequently for smooth display
  display.update();
  
  // Check buttons
  checkButtons();
  
  // Auto count mode
  if (autoCount) {
    unsigned long now = millis();
    if (now - lastAutoCount >= autoSpeed) {
      counter++;
      if (counter > 99) counter = 0;
      display.showNumber(counter);
      printCounter();
      lastAutoCount = now;
    }
  }
  
  // Check serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
}

void checkButtons() {
  // UP button
  static bool lastUpState = HIGH;
  bool upState = digitalRead(BTN_UP);
  
  if (upState == LOW && lastUpState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_UP) == LOW) {
      counter++;
      if (counter > 99) counter = 0;
      display.showNumber(counter);
      printCounter();
    }
  }
  lastUpState = upState;
  
  // DOWN button
  static bool lastDownState = HIGH;
  bool downState = digitalRead(BTN_DOWN);
  
  if (downState == LOW && lastDownState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_DOWN) == LOW) {
      counter--;
      if (counter < 0) counter = 99;
      display.showNumber(counter);
      printCounter();
    }
  }
  lastDownState = downState;
  
  // RESET button
  static bool lastResetState = HIGH;
  bool resetState = digitalRead(BTN_RESET);
  
  if (resetState == LOW && lastResetState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_RESET) == LOW) {
      counter = 0;
      display.showNumber(counter);
      Serial.println("⟲ Counter RESET");
      printCounter();
    }
  }
  lastResetState = resetState;
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 'a':
    case 'A':
      autoCount = !autoCount;
      Serial.print(autoCount ? "▶ Auto count STARTED" : "■ Auto count STOPPED");
      Serial.print(" (Speed: ");
      Serial.print(autoSpeed);
      Serial.println(" ms)");
      break;
      
    case '+':
      autoSpeed -= 50;
      if (autoSpeed < 50) autoSpeed = 50;
      Serial.print("⚡ Speed: ");
      Serial.print(autoSpeed);
      Serial.println(" ms");
      break;
      
    case '-':
      autoSpeed += 50;
      if (autoSpeed > 2000) autoSpeed = 2000;
      Serial.print("🐢 Speed: ");
      Serial.print(autoSpeed);
      Serial.println(" ms");
      break;
      
    case 'z':
    case 'Z':
      {
        static bool leadingZeros = true;
        leadingZeros = !leadingZeros;
        display.setLeadingZeros(leadingZeros);
        Serial.print("Leading zeros: ");
        Serial.println(leadingZeros ? "ON (00-09)" : "OFF (0-9)");
      }
      break;
      
    case 'i':
    case 'I':
      showInfo();
      break;
      
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
        int digit = cmd - '0';
        counter = counter * 10 + digit;
        if (counter > 99) counter = counter % 100;
        display.showNumber(counter);
        printCounter();
      }
      break;
  }
}

void printCounter() {
  Serial.print("╔═════╗\n║  ");
  if (counter < 10) Serial.print("0");
  Serial.print(counter);
  Serial.println("  ║\n╚═════╝");
}

void showInfo() {
  Serial.println("\n========== Display Info ==========");
  Serial.print("Current Value: ");
  Serial.println(counter);
  Serial.print("Auto Count: ");
  Serial.println(autoCount ? "ENABLED" : "DISABLED");
  if (autoCount) {
    Serial.print("Speed: ");
    Serial.print(autoSpeed);
    Serial.println(" ms");
  }
  Serial.println("\nMultiplexing Info:");
  Serial.println("  Digits: 2");
  Serial.println("  Scan interval: 1000 μs");
  Serial.println("  Scan rate: ~500 Hz");
  Serial.println("  GPIO used: 9 pins");
  Serial.println("==================================\n");
}
