/*
 * Interrupt_SoftwareDebounce
 * 
 * : Interrupts + Advanced Button
 * Software Debounce ขั้นสูง - แยก ISR กับ Debounce Logic
 * 
 * แนวคิด:
 * - ISR แค่บันทึก "เวลาที่เกิดเหตุการณ์" และ "สถานะดิบ"
 * - loop() จะเป็นคนทำ Debounce Logic ที่สมบูรณ์
 * - ตรวจจับทั้งขอบขึ้น (pressed) และขอบลง (released)
 * - ป้องกันการ bounce ได้ดีกว่าการทำ debounce ใน ISR
 * 
 * วงจร (ESP32 ICON-32):
 * - ปุ่ม: GPIO 4 → GND (ใช้ INPUT_PULLUP)
 * - LED:  GPIO 2 → 220Ω → GND
 * 
 * พฤติกรรม:
 * - กดปุ่ม → นับจำนวนครั้งที่กด (แสดงบน Serial)
 * - แสดง "Press" และ "Release" แยกกัน
 * - ไม่นับซ้ำจาก bounce
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <Arduino.h>

// ==================== ค่าคงที่ ====================
const int BUTTON_PIN = 4;
const int LED_PIN = 2;
const unsigned long DEBOUNCE_TIME = 50; // ms

// ==================== ตัวแปร ISR ====================
volatile bool rawInterruptFlag = false;
volatile unsigned long rawInterruptTime = 0;

// ==================== ตัวแปร Debounce ====================
unsigned long lastChangeTime = 0;
bool stableState = HIGH;      // เพราะใช้ PULLUP
bool lastStableState = HIGH;

// ตัวแปรนับ
int pressCount = 0;

// ==================== ISR Function ====================
/**
 * ISR เวอร์ชันที่ถูกต้อง:
 * - ไม่ทำ debounce logic ซับซ้อนใน ISR
 * - แค่จำเวลาและตั้งธง
 * - ให้ loop() ไปตรวจสอบต่อ
 */
void handleButtonInterrupt() {
  rawInterruptFlag = true;
  rawInterruptTime = millis();
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("=== : Interrupt Software Debounce ===");
  Serial.println("ISR แค่บันทึกเวลา, loop() ทำ debounce");
  Serial.println();
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // ติดตั้ง Interrupt แบบ CHANGE (ทั้งขึ้นและลง)
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    CHANGE  // ตรวจจับทั้ง RISING และ FALLING
  );
  
  Serial.println("✓ Interrupt ติดตั้งแล้ว (CHANGE mode)");
  Serial.println("✓ กดปุ่มเพื่อทดสอบ debounce");
  Serial.println();
}

// ==================== Loop ====================
void loop() {
  // เช็คว่ามี interrupt เกิดขึ้นหรือไม่
  if (rawInterruptFlag) {
    rawInterruptFlag = false;  // เคลียร์ธง
    
    // อ่านสถานะปัจจุบันของปุ่ม
    int reading = digitalRead(BUTTON_PIN);
    unsigned long eventTime = rawInterruptTime;
    
    // =============================================
    // Debounce Logic (ทำนอก ISR)
    // =============================================
    if (eventTime - lastChangeTime > DEBOUNCE_TIME) {
      // ผ่าน debounce แล้ว
      
      if (reading != stableState) {
        // สถานะเปลี่ยนจริง (ไม่ใช่ bounce)
        stableState = reading;
        lastChangeTime = eventTime;
        
        // ตรวจจับขอบสัญญาณ
        bool isPressed = (stableState == LOW);  // PULLUP → กด = LOW
        bool wasPressed = (lastStableState == LOW);
        
        if (isPressed && !wasPressed) {
          // เปลี่ยนจาก ปล่อย → กด (Pressed)
          pressCount++;
          digitalWrite(LED_PIN, HIGH);
          
          Serial.print("🔵 Pressed  #");
          Serial.print(pressCount);
          Serial.print(" (");
          Serial.print(eventTime);
          Serial.println(" ms)");
          
        } else if (!isPressed && wasPressed) {
          // เปลี่ยนจาก กด → ปล่อย (Released)
          digitalWrite(LED_PIN, LOW);
          
          Serial.print("⚪ Released #");
          Serial.print(pressCount);
          Serial.print(" (");
          Serial.print(eventTime);
          Serial.println(" ms)");
          Serial.println();
        }
        
        lastStableState = stableState;
      } else {
        // Bounce - สถานะเดียวกัน ไม่ต้องทำอะไร
        Serial.print("⚠️ Bounce ignored (");
        Serial.print(eventTime - lastChangeTime);
        Serial.println(" ms)");
      }
    } else {
      // ยังไม่ผ่าน debounce time - ละเว้น
      Serial.print("🚫 Too fast (");
      Serial.print(eventTime - lastChangeTime);
      Serial.println(" ms) - ignored");
    }
  }
  
  // งานอื่น ๆ ทำได้ต่อเนื่อง
  static unsigned long lastReport = 0;
  unsigned long now = millis();
  
  if (now - lastReport >= 10000) {
    lastReport = now;
    Serial.print("📊 Total presses: ");
    Serial.println(pressCount);
  }
}

/*
 * ==================== การทำงาน ====================
 * 
 * 📌 ลำดับการทำงาน:
 * 
 * 1. เมื่อปุ่มเปลี่ยนสถานะ (กด/ปล่อย)
 *    → GPIO interrupt เกิดขึ้น
 *    → ISR ทำงาน
 * 
 * 2. ISR ทำงาน (เร็ว):
 *    rawInterruptFlag = true;
 *    rawInterruptTime = millis();
 *    → จบ ISR (กลับไปทำงานเดิม)
 * 
 * 3. loop() ตรวจเจอ flag == true:
 *    → เคลียร์ flag
 *    → อ่านสถานะปุ่มจริง
 *    → เช็คว่าเวลาห่างจากครั้งก่อน > DEBOUNCE_TIME หรือไม่
 *    → ถ้าผ่าน → ยอมรับว่าเป็นการกดจริง
 *    → ถ้าไม่ผ่าน → ละเว้น (bounce)
 * 
 * ==================== ข้อดี ====================
 * 
 * ✅ ISR สั้น เร็ว (แค่บันทึกข้อมูล)
 * ✅ Debounce logic สมบูรณ์ทำใน loop()
 * ✅ ตรวจจับทั้ง pressed และ released
 * ✅ ไม่พลาดเหตุการณ์ (เพราะใช้ interrupt)
 * ✅ แยกระหว่าง bounce กับการกดจริง
 * 
 * ==================== เปรียบเทียบ ====================
 * 
 * แบบ BasicToggle:
 * - Debounce ใน ISR (ใช้ millis ตรงๆ)
 * - ตรวจจับแค่ขอบเดียว (FALLING)
 * 
 * แบบ SoftwareDebounce (นี่):
 * - ISR แค่บันทึกเวลา
 * - Debounce logic ใน loop()
 * - ตรวจจับทั้ง pressed และ released
 * - เหมาะกับงานที่ต้องการความแม่นยำสูง
 * 
 * ==================== Debug Tips ====================
 * 
 * ลอง Serial Monitor ดู:
 * - กดปุ่มช้า ๆ → จะเห็น "Pressed" และ "Released" อย่างละครั้ง
 * - กดปุ่มเร็ว ๆ ซ้ำ → จะเห็น "Too fast - ignored"
 * - ปุ่มคุณภาพแย่ → จะเห็น "Bounce ignored" บ่อย
 * 
 * ถ้าเห็น bounce เยอะ:
 * → เพิ่ม DEBOUNCE_TIME
 * → ใช้ Hardware Debounce (RC Filter)
 */
