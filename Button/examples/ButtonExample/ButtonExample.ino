/*
 * ButtonExample
 * 
 * EP1: Basic Button with Software Debounce
 * ตัวอย่างพื้นฐานการใช้งาน Button Library
 * 
 * รองรับ:
 * - ESP32 (ICON-32, DevKit, etc.)
 * - Arduino UNO R3/R4
 * - Arduino Mega
 * - Arduino Nano
 * 
 * วงจร:
 * === ESP32 ===
 * - Button: GPIO 4 → Button → GND (INPUT_PULLUP)
 * - LED:    GPIO 2 (Built-in LED)
 * 
 * === Arduino UNO ===
 * - Button: Pin 4 → Button → GND (INPUT_PULLUP)
 * - LED:    Pin 13 (Built-in LED)
 * 
 * พฤติกรรม:
 * - กดปุ่ม → Toggle LED
 * - แสดงสถานะปุ่มทุก 1 วินาที
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <Button.h>

// ==================== Configuration ====================
// เลือกตาม board ที่ใช้:

#if defined(ESP32)
  // ESP32 Configuration
  const int BUTTON_PIN = 4;   // GPIO 4
  const int LED_PIN = 2;      // GPIO 2 (Built-in LED)
#elif defined(ARDUINO_ARCH_RENESAS)
  // Arduino UNO R4 Configuration
  const int BUTTON_PIN = 4;   // Pin 4
  const int LED_PIN = 13;     // Pin 13 (Built-in LED)
#else
  // Arduino UNO R3, Mega, Nano Configuration
  const int BUTTON_PIN = 4;   // Pin 4
  const int LED_PIN = 13;     // Pin 13 (Built-in LED)
#endif

// ==================== Objects ====================
Button button(BUTTON_PIN, true, 50);  // pin, pullup, debounce(ms)

// ==================== Variables ====================
bool ledState = false;

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  EP1: Button Example");
  Serial.println("===============================================");
  Serial.println();
  
  // แสดง board ที่ใช้
  #if defined(ESP32)
    Serial.println("Board: ESP32");
  #elif defined(ARDUINO_ARCH_RENESAS)
    Serial.println("Board: Arduino UNO R4");
  #else
    Serial.println("Board: Arduino UNO/Mega/Nano");
  #endif
  
  Serial.println();
  Serial.println("Hardware:");
  Serial.print("  Button: Pin ");
  Serial.println(BUTTON_PIN);
  Serial.print("  LED:    Pin ");
  Serial.println(LED_PIN);
  Serial.println();
  
  // Initialize
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  button.begin();
  
  Serial.println("✓ Button library initialized");
  Serial.println("✓ Press button to toggle LED");
  Serial.println();
}

// ==================== Loop ====================
void loop() {
  // อัปเดตสถานะปุ่ม (เรียกทุก loop)
  button.update();
  
  // ตรวจสอบว่าปุ่มถูกกด
  if (button.wasPressed()) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    
    Serial.print("Button pressed! LED: ");
    Serial.println(ledState ? "ON" : "OFF");
  }
  
  // แสดงสถานะปุ่มทุก 1 วินาที
  static unsigned long lastReport = 0;
  unsigned long now = millis();
  
  if (now - lastReport >= 1000) {
    lastReport = now;
    
    Serial.print("Status - Button: ");
    Serial.print(button.isPressed() ? "PRESSED  " : "RELEASED ");
    Serial.print("  LED: ");
    Serial.println(ledState ? "ON " : "OFF");
  }
}

/*
 * ==================== Pin Configuration Guide ====================
 * 
 * ESP32 (ICON-32):
 * - BUTTON_PIN: 4, 5, 12-15, 16-19, 21-23, 25-27, 32-33
 * - หลีกเลี่ยง: GPIO 0,2 (boot), GPIO 6-11 (flash), GPIO 34-39 (input only)
 * - LED_PIN: 2 (built-in)
 * 
 * Arduino UNO R3/R4:
 * - BUTTON_PIN: 2-12 (ทั่วไป)
 * - LED_PIN: 13 (built-in)
 * - หมายเหตุ: Pin 0,1 ใช้สำหรับ Serial
 * 
 * Arduino Mega:
 * - BUTTON_PIN: 2-53
 * - LED_PIN: 13 (built-in)
 * 
 * Arduino Nano:
 * - BUTTON_PIN: 2-12
 * - LED_PIN: 13 (built-in)
 * 
 * ==================== Troubleshooting ====================
 * 
 * ปุ่มไม่ตอบสนอง:
 * - ตรวจสอบวงจร: ปุ่มต่อระหว่าง BUTTON_PIN กับ GND
 * - ตรวจสอบว่าใช้ INPUT_PULLUP (ค่า default ใน constructor)
 * - เปิด Serial Monitor (115200 baud) ดูว่ามี "Button pressed!" หรือไม่
 * - ลองกด-ปล่อยปุ่มช้า ๆ
 * 
 * ปุ่มตอบสนองหลายครั้ง (bounce):
 * - เพิ่มเวลา debounce: Button button(BUTTON_PIN, true, 100);
 * - ใช้ปุ่มคุณภาพดีกว่า
 * - เพิ่ม Hardware Debounce (capacitor 0.1µF จาก pin → GND)
 * 
 * LED ไม่ติด:
 * - ตรวจสอบว่า LED_PIN ตั้งเป็น OUTPUT แล้ว
 * - ลอง digitalWrite(LED_PIN, HIGH) ใน setup() ดู
 * - บาง board LED อาจต่อแบบ Active LOW (สลับ HIGH/LOW)
 * 
 * Serial Monitor ไม่แสดงผล:
 * - ตรวจสอบ Baud rate = 115200
 * - Arduino UNO ต้องรอหลัง upload 2-3 วินาที
 * - กด Reset button บน board
 * 
 * ==================== การปรับแต่ง ====================
 * 
 * เปลี่ยน Pin:
 * - แก้ BUTTON_PIN และ LED_PIN ตามต้องการ
 * - หรือลบ #if defined() ออก แล้วใส่ค่าตรง ๆ
 * 
 * เปลี่ยน Debounce Time:
 * - Button button(BUTTON_PIN, true, 100);  // 100ms
 * - เพิ่มเป็น 100-200ms ถ้าปุ่มคุณภาพต่ำ
 * 
 * ปิด Pull-up (ใช้ external pull-up):
 * - Button button(BUTTON_PIN, false, 50);
 * - ต้องต่อ resistor 10kΩ จาก pin → VCC
 */

