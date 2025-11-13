/*
 * AdvancedButton_ClickTypes
 * 
 * : Interrupts + Advanced Button
 * ใช้ AdvancedButton Class ตรวจจับ Single/Double Click และ Long Press
 * 
 * แนวคิด:
 * - ใช้ millis() วัดเวลาระหว่างการกด
 * - ตรวจจับ pattern การกด:
 *   • Single Click = กดสั้น 1 ครั้ง
 *   • Double Click = กดสั้น 2 ครั้งติดกัน (ภายใน 300ms)
 *   • Long Press   = กดค้างนาน (> 800ms)
 * 
 * วงจร (ESP32 ICON-32):
 * - ปุ่ม: GPIO 4 → GND (ใช้ INPUT_PULLUP)
 * - LED1: GPIO 2  → 220Ω → GND (Built-in LED)
 * - LED2: GPIO 15 → 220Ω → GND
 * - LED3: GPIO 13 → 220Ω → GND
 * 
 * พฤติกรรม:
 * - Single Click → Toggle LED1
 * - Double Click → Toggle LED2
 * - Long Press   → Toggle LED3 + Reset ทุก LED
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <Button.h>

// ==================== ค่าคงที่ ====================
const int BUTTON_PIN = 4;
const int LED1_PIN = 2;   // Single Click
const int LED2_PIN = 15;  // Double Click
const int LED3_PIN = 13;  // Long Press

// ==================== Objects ====================
AdvancedButton button(BUTTON_PIN, true);  // pin 4, pullup enabled

// ==================== ตัวแปร ====================
bool led1State = false;
bool led2State = false;
bool led3State = false;

int singleClickCount = 0;
int doubleClickCount = 0;
int longPressCount = 0;

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  : Advanced Button - Click Types");
  Serial.println("===============================================");
  Serial.println();
  Serial.println("ทดสอบ 3 รูปแบบการกดปุ่ม:");
  Serial.println("  🔵 Single Click → Toggle LED1");
  Serial.println("  🟣 Double Click → Toggle LED2");
  Serial.println("  🔴 Long Press   → Toggle LED3 + Reset All");
  Serial.println();
  
  // ตั้งค่า LED
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  
  // เริ่มต้น Button
  button.begin();
  
  // ปรับค่า timing (optional)
  // button.setDebounceTime(40);           // default 40ms
  // button.setDoubleClickWindow(300);     // default 300ms
  // button.setLongPressThreshold(800);    // default 800ms
  
  Serial.println("✓ AdvancedButton พร้อมใช้งาน");
  Serial.println("✓ ลองกดปุ่มแบบต่าง ๆ ดูผล");
  Serial.println();
}

// ==================== Loop ====================
void loop() {
  // อัปเดตสถานะปุ่ม (เรียกทุก loop)
  button.update();
  
  // =============================================
  // ตรวจจับ Single Click
  // =============================================
  if (button.singleClick) {
    singleClickCount++;
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  🔵 SINGLE CLICK                       ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.print("   Count: ");
    Serial.println(singleClickCount);
    Serial.print("   LED1: ");
    Serial.println(led1State ? "ON" : "OFF");
    Serial.println();
  }
  
  // =============================================
  // ตรวจจับ Double Click
  // =============================================
  if (button.doubleClick) {
    doubleClickCount++;
    led2State = !led2State;
    digitalWrite(LED2_PIN, led2State);
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  🟣 DOUBLE CLICK                       ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.print("   Count: ");
    Serial.println(doubleClickCount);
    Serial.print("   LED2: ");
    Serial.println(led2State ? "ON" : "OFF");
    Serial.println();
  }
  
  // =============================================
  // ตรวจจับ Long Press
  // =============================================
  if (button.longPress) {
    longPressCount++;
    led3State = !led3State;
    digitalWrite(LED3_PIN, led3State);
    
    // Long press = รีเซ็ต LED ทั้งหมด
    led1State = false;
    led2State = false;
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  🔴 LONG PRESS                         ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.print("   Count: ");
    Serial.println(longPressCount);
    Serial.print("   LED3: ");
    Serial.println(led3State ? "ON" : "OFF");
    Serial.println("   → All LEDs Reset!");
    Serial.println();
  }
  
  // =============================================
  // แสดงสถานะเมื่อกด/ปล่อย (optional)
  // =============================================
  if (button.pressed) {
    Serial.print("⬇️  Pressed (");
    Serial.print(millis());
    Serial.println(" ms)");
  }
  
  if (button.released) {
    unsigned long duration = button.getPressedDuration();
    Serial.print("⬆️  Released (duration: ");
    Serial.print(duration);
    Serial.println(" ms)");
  }
  
  // =============================================
  // รายงานสถานะทุก 15 วินาที
  // =============================================
  static unsigned long lastReport = 0;
  unsigned long now = millis();
  
  if (now - lastReport >= 15000) {
    lastReport = now;
    
    Serial.println("───────────────────────────────────────");
    Serial.println("📊 Summary:");
    Serial.print("   Single Clicks: ");
    Serial.println(singleClickCount);
    Serial.print("   Double Clicks: ");
    Serial.println(doubleClickCount);
    Serial.print("   Long Presses:  ");
    Serial.println(longPressCount);
    Serial.println("───────────────────────────────────────");
    Serial.println();
  }
}

/*
 * ==================== วิธีใช้งาน ====================
 * 
 * 📌 Single Click:
 * - กดปุ่มสั้น ๆ 1 ครั้ง
 * - ปล่อย
 * - รอ > 300ms (ไม่กดอีก)
 * → ระบบจะตีความว่า "Single Click"
 * 
 * 📌 Double Click:
 * - กดปุ่มสั้น ๆ 1 ครั้ง → ปล่อย
 * - กดอีกครั้งภายใน 300ms
 * → ระบบจะตีความว่า "Double Click"
 * 
 * 📌 Long Press:
 * - กดปุ่มค้างไว้ > 800ms
 * → ระบบจะตีความว่า "Long Press"
 * 
 * ==================== Configuration ====================
 * 
 * ปรับค่าใน setup() ได้:
 * 
 * button.setDebounceTime(50);           // ป้องกัน bounce (ms)
 * button.setDoubleClickWindow(400);     // ระยะห่างสูงสุดสำหรับ double click (ms)
 * button.setLongPressThreshold(1000);   // เวลาขั้นต่ำสำหรับ long press (ms)
 * 
 * ==================== Advanced Usage ====================
 * 
 * ตรวจจับสถานะปัจจุบัน:
 * 
 * if (button.isPressed()) {
 *   // ปุ่มกำลังถูกกดอยู่
 *   unsigned long duration = button.getPressedDuration();
 *   Serial.println(duration);  // เวลาที่กดค้างไว้ (ms)
 * }
 * 
 * ==================== หมายเหตุ ====================
 * 
 * ⚠️ Single Click มี delay:
 * - เมื่อกดปุ่มสั้น ๆ ระบบจะรอ 300ms ก่อน
 * - เพื่อให้แน่ใจว่าไม่มีการกดครั้งที่ 2
 * - ถ้าต้องการตอบสนองทันที → ใช้ button.pressed แทน
 * 
 * ✅ Double Click และ Long Press:
 * - ตอบสนองทันทีเมื่อตรวจจับได้
 * 
 * ==================== Applications ====================
 * 
 * 💡 ตัวอย่างการใช้งานจริง:
 * 
 * Single Click:
 * - เปิด/ปิดไฟ
 * - เลื่อนเมนู
 * 
 * Double Click:
 * - เปลี่ยนโหมด
 * - Reset ค่า
 * 
 * Long Press:
 * - เข้า Config Mode
 * - ปิดระบบ
 * - Factory Reset
 * 
 * ==================== Debug Tips ====================
 * 
 * ถ้า Single Click ตรวจจับไม่ได้:
 * → เพิ่ม doubleClickWindow
 * 
 * ถ้า Double Click ยาก:
 * → ลด doubleClickWindow
 * → ลด debounceTime
 * 
 * ถ้า Long Press ตรวจจับช้า:
 * → ลด longPressThreshold
 */
