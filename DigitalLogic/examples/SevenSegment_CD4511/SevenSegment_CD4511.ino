/**
 * 7-Segment Display with CD4511
 * 
 * ใช้ IC CD4511 (BCD to 7-segment decoder) แสดงตัวเลข 0-9
 * บน 7-segment display โดยควบคุมจาก ESP32
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - IC CD4511 (BCD to 7-segment decoder/driver)
 * - 7-segment display แบบ Common Cathode
 * - ตัวต้านทาน 220Ω จำนวน 7 ตัว (สำหรับ segment a-g)
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร CD4511:
 * 
 *   ESP32 GPIO 19 ────► CD4511 pin 7  (A - LSB)
 *   ESP32 GPIO 18 ────► CD4511 pin 1  (B)
 *   ESP32 GPIO 5  ────► CD4511 pin 2  (C)
 *   ESP32 GPIO 17 ────► CD4511 pin 6  (D - MSB)
 *   
 *   CD4511 pin 13 (a) ──► R 220Ω ──► 7-seg pin a
 *   CD4511 pin 12 (b) ──► R 220Ω ──► 7-seg pin b
 *   CD4511 pin 11 (c) ──► R 220Ω ──► 7-seg pin c
 *   CD4511 pin 10 (d) ──► R 220Ω ──► 7-seg pin d
 *   CD4511 pin 9  (e) ──► R 220Ω ──► 7-seg pin e
 *   CD4511 pin 15 (f) ──► R 220Ω ──► 7-seg pin f
 *   CD4511 pin 14 (g) ──► R 220Ω ──► 7-seg pin g
 *   
 *   CD4511 pin 3  (LT) ──► VCC (lamp test inactive)
 *   CD4511 pin 4  (BI) ──► VCC (blanking inactive)
 *   CD4511 pin 5  (LE) ──► GND (latch transparent)
 *   CD4511 pin 8  ──────► GND
 *   CD4511 pin 16 ──────► VCC (5V แนะนำ, หรือ 3.3V ก็ได้)
 *   
 *   7-segment common cathode ──► GND
 * 
 * หมายเหตุ:
 * - CD4511 เป็น CMOS IC ทำงานได้ทั้ง 3.3V และ 5V
 * - ใช้ 5V จะได้ความสว่างของ 7-segment ดีกว่า
 * - Common Anode ต้องใช้ CD4543 แทน
 */

#include <DigitalLogic.h>

// BCD pins
const int PIN_A = 19;  // LSB
const int PIN_B = 18;
const int PIN_C = 5;
const int PIN_D = 17;  // MSB

// สร้าง BCD encoder object
BCDEncoder display(PIN_A, PIN_B, PIN_C, PIN_D);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("==========================================");
  Serial.println(" 7-Segment Display with CD4511 - ESP32");
  Serial.println("==========================================\n");
  
  // เริ่มต้น BCD encoder
  display.begin();
  
  Serial.println("✓ CD4511 initialized");
  Serial.println("  BCD pins: GPIO 19,18,5,17 (A,B,C,D)");
  Serial.println("  Display: 7-segment common cathode\n");
  
  Serial.println("Demo modes:");
  Serial.println("  1 - Count 0-9 continuously");
  Serial.println("  2 - Count 0-9 then reverse");
  Serial.println("  3 - Random digits");
  Serial.println("  4 - Manual control (0-9)\n");
  
  // เริ่มต้นด้วยการแสดง 0
  display.displayDigit(0);
}

void loop() {
  // Demo 1: นับ 0-9 วนซ้ำ
  Serial.println("Mode 1: Counting 0-9...\n");
  countUp();
  delay(2000);
  
  // Demo 2: นับขึ้น-ลง
  Serial.println("\nMode 2: Count up and down...\n");
  countUpDown();
  delay(2000);
  
  // Demo 3: สุ่มเลข
  Serial.println("\nMode 3: Random digits...\n");
  randomDigits();
  delay(2000);
  
  // Demo 4: ทดสอบแต่ละเลข
  Serial.println("\nMode 4: Testing each digit...\n");
  testAllDigits();
  delay(2000);
  
  Serial.println("\n========================================\n");
}

void countUp() {
  for (int i = 0; i <= 9; i++) {
    display.displayDigit(i);
    Serial.print("Displaying: ");
    Serial.print(i);
    Serial.print(" (BCD: ");
    printBCD(i);
    Serial.println(")");
    delay(500);
  }
}

void countUpDown() {
  // นับขึ้น 0-9
  for (int i = 0; i <= 9; i++) {
    display.displayDigit(i);
    Serial.print("↑ ");
    Serial.println(i);
    delay(400);
  }
  
  delay(500);
  
  // นับลง 9-0
  for (int i = 9; i >= 0; i--) {
    display.displayDigit(i);
    Serial.print("↓ ");
    Serial.println(i);
    delay(400);
  }
}

void randomDigits() {
  for (int i = 0; i < 20; i++) {
    int digit = random(0, 10);
    display.displayDigit(digit);
    Serial.print("Random: ");
    Serial.println(digit);
    delay(300);
  }
}

void testAllDigits() {
  Serial.println("Digit | 7-Segment Pattern");
  Serial.println("------|-------------------");
  
  for (int i = 0; i <= 9; i++) {
    display.displayDigit(i);
    Serial.print("  ");
    Serial.print(i);
    Serial.print("   | ");
    printSegmentPattern(i);
    Serial.println();
    delay(800);
  }
  
  // ทดสอบค่าที่มากกว่า 9 (จะแสดงอะไร?)
  Serial.println("\nTesting invalid BCD (>9):");
  for (int i = 10; i <= 15; i++) {
    display.displayDigit(i);
    Serial.print("BCD ");
    Serial.print(i);
    Serial.print(": ");
    printBCD(i);
    Serial.println(" -> Usually blank or dash");
    delay(800);
  }
  
  // Clear display
  Serial.println("\nClearing display...");
  display.clear();
  delay(1000);
}

void printBCD(int num) {
  for (int i = 3; i >= 0; i--) {
    Serial.print((num & (1 << i)) ? '1' : '0');
  }
}

void printSegmentPattern(int digit) {
  // ASCII art representation of 7-segment
  // This is approximate visualization
  switch (digit) {
    case 0: Serial.print("█▀▀█ (all except g)"); break;
    case 1: Serial.print("   █ (b,c only)"); break;
    case 2: Serial.print("▀▀▀█ (a,b,d,e,g)"); break;
    case 3: Serial.print("▀▀▀▀ (a,b,c,d,g)"); break;
    case 4: Serial.print("  ▀█ (b,c,f,g)"); break;
    case 5: Serial.print("█▀▀  (a,c,d,f,g)"); break;
    case 6: Serial.print("█▀▀█ (a,c,d,e,f,g)"); break;
    case 7: Serial.print("▀▀ █ (a,b,c)"); break;
    case 8: Serial.print("█▀▀█ (all segments)"); break;
    case 9: Serial.print("█▀▀█ (all except e)"); break;
    default: Serial.print("Unknown"); break;
  }
}
