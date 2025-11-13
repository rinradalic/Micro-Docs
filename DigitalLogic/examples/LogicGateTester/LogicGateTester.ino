/**
 * Logic Gate Tester
 * 
 * ทดสอบการทำงานของ Logic Gates ต่าง ๆ โดยใช้ ESP32 เป็นตัวป้อนสัญญาณ
 * และแสดงผลการคำนวณผ่าน Serial Monitor
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - IC 7408 (AND), 7432 (OR), 7404 (NOT), 7400 (NAND), หรือ 7486 (XOR)
 * - LED + R 220Ω (สำหรับดูผลลัพธ์)
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร (ตัวอย่าง 7408 - AND Gate):
 * 
 *   ESP32 GPIO 26 ────► 7408 pin 1 (1A)
 *   ESP32 GPIO 27 ────► 7408 pin 2 (1B)
 *   7408 pin 3 (1Y) ──► LED ──► R 220Ω ──► GND
 *   7408 pin 7 ───────► GND
 *   7408 pin 14 ──────► 3.3V (หรือ 5V ถ้ามี)
 * 
 * หมายเหตุ:
 * - IC 74xx ส่วนใหญ่ทำงานที่ 5V แต่รับสัญญาณ 3.3V ได้
 * - ถ้าต้องการความน่าเชื่อถือสูง ให้ใช้ level shifter
 * - ตรวจสอบ datasheet ของแต่ละ IC สำหรับ pinout
 */

#include <DigitalLogic.h>

// กำหนด GPIO pins สำหรับทดสอบ gates
const int PIN_A = 26;  // Input A
const int PIN_B = 27;  // Input B

// สร้าง LogicGate objects
LogicGate andGate(PIN_A, PIN_B, GATE_AND);
LogicGate orGate(PIN_A, PIN_B, GATE_OR);
LogicGate nandGate(PIN_A, PIN_B, GATE_NAND);
LogicGate xorGate(PIN_A, PIN_B, GATE_XOR);
LogicGate notGate(PIN_A, GATE_NOT);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("===========================================");
  Serial.println("  Logic Gate Tester - + 74xx ICs");
  Serial.println("===========================================\n");

  // เริ่มต้น gate (ในตัวอย่างนี้ใช้ pins เดียวกัน แต่จริง ๆ ควรต่อแยก IC)
  andGate.begin();
  
  Serial.println("เชื่อมต่อ IC ที่ต้องการทดสอบกับ GPIO 26 และ 27");
  Serial.println("และต่อ output ของ IC ไปยัง LED");
  Serial.println("\nกำลังเริ่มทดสอบ...\n");
  
  delay(2000);
}

void loop() {
  // ทดสอบ AND Gate
  testAndGate();
  delay(3000);
  
  // ทดสอบ OR Gate
  testOrGate();
  delay(3000);
  
  // ทดสอบ NAND Gate
  testNandGate();
  delay(3000);
  
  // ทดสอบ XOR Gate
  testXorGate();
  delay(3000);
  
  // ทดสอบ NOT Gate
  testNotGate();
  delay(3000);
  
  Serial.println("\n========================================\n");
  delay(2000);
}

void testAndGate() {
  Serial.println("--- Testing AND Gate (7408) ---");
  Serial.println("Truth Table:");
  Serial.println("A | B | Output");
  Serial.println("--|---|-------");
  
  // 0 AND 0 = 0
  andGate.setInputs(false, false);
  printGateOutput(andGate.getA(), andGate.getB(), andGate.calculateOutput());
  delay(500);
  
  // 0 AND 1 = 0
  andGate.setInputs(false, true);
  printGateOutput(andGate.getA(), andGate.getB(), andGate.calculateOutput());
  delay(500);
  
  // 1 AND 0 = 0
  andGate.setInputs(true, false);
  printGateOutput(andGate.getA(), andGate.getB(), andGate.calculateOutput());
  delay(500);
  
  // 1 AND 1 = 1
  andGate.setInputs(true, true);
  printGateOutput(andGate.getA(), andGate.getB(), andGate.calculateOutput());
  delay(500);
  
  Serial.println();
}

void testOrGate() {
  Serial.println("--- Testing OR Gate (7432) ---");
  Serial.println("Truth Table:");
  Serial.println("A | B | Output");
  Serial.println("--|---|-------");
  
  orGate.setInputs(false, false);
  printGateOutput(orGate.getA(), orGate.getB(), orGate.calculateOutput());
  delay(500);
  
  orGate.setInputs(false, true);
  printGateOutput(orGate.getA(), orGate.getB(), orGate.calculateOutput());
  delay(500);
  
  orGate.setInputs(true, false);
  printGateOutput(orGate.getA(), orGate.getB(), orGate.calculateOutput());
  delay(500);
  
  orGate.setInputs(true, true);
  printGateOutput(orGate.getA(), orGate.getB(), orGate.calculateOutput());
  delay(500);
  
  Serial.println();
}

void testNandGate() {
  Serial.println("--- Testing NAND Gate (7400) ---");
  Serial.println("Truth Table:");
  Serial.println("A | B | Output");
  Serial.println("--|---|-------");
  
  nandGate.setInputs(false, false);
  printGateOutput(nandGate.getA(), nandGate.getB(), nandGate.calculateOutput());
  delay(500);
  
  nandGate.setInputs(false, true);
  printGateOutput(nandGate.getA(), nandGate.getB(), nandGate.calculateOutput());
  delay(500);
  
  nandGate.setInputs(true, false);
  printGateOutput(nandGate.getA(), nandGate.getB(), nandGate.calculateOutput());
  delay(500);
  
  nandGate.setInputs(true, true);
  printGateOutput(nandGate.getA(), nandGate.getB(), nandGate.calculateOutput());
  delay(500);
  
  Serial.println();
}

void testXorGate() {
  Serial.println("--- Testing XOR Gate (7486) ---");
  Serial.println("Truth Table:");
  Serial.println("A | B | Output");
  Serial.println("--|---|-------");
  
  xorGate.setInputs(false, false);
  printGateOutput(xorGate.getA(), xorGate.getB(), xorGate.calculateOutput());
  delay(500);
  
  xorGate.setInputs(false, true);
  printGateOutput(xorGate.getA(), xorGate.getB(), xorGate.calculateOutput());
  delay(500);
  
  xorGate.setInputs(true, false);
  printGateOutput(xorGate.getA(), xorGate.getB(), xorGate.calculateOutput());
  delay(500);
  
  xorGate.setInputs(true, true);
  printGateOutput(xorGate.getA(), xorGate.getB(), xorGate.calculateOutput());
  delay(500);
  
  Serial.println();
}

void testNotGate() {
  Serial.println("--- Testing NOT Gate (7404) ---");
  Serial.println("Truth Table:");
  Serial.println("A | Output");
  Serial.println("--|-------");
  
  notGate.setA(false);
  printGateOutput(notGate.getA(), notGate.calculateOutput());
  delay(500);
  
  notGate.setA(true);
  printGateOutput(notGate.getA(), notGate.calculateOutput());
  delay(500);
  
  Serial.println();
}

// Helper functions
void printGateOutput(bool a, bool b, bool output) {
  Serial.print(a ? "1" : "0");
  Serial.print(" | ");
  Serial.print(b ? "1" : "0");
  Serial.print(" |   ");
  Serial.println(output ? "1" : "0");
}

void printGateOutput(bool a, bool output) {
  Serial.print(a ? "1" : "0");
  Serial.print(" |   ");
  Serial.println(output ? "1" : "0");
}
