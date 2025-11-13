/*
 * HardwareDebounce_Comparison
 * 
 * : Interrupts + Advanced Button
 * เปรียบเทียบ Software Debounce vs Hardware Debounce (RC Filter)
 * 
 * แนวคิด:
 * - แสดงให้เห็นว่า Hardware Debounce ช่วยลดปัญหา bounce ที่ต้นทาง
 * - ปุ่มที่ใช้ RC Filter จะมีสัญญาณเรียบขึ้น → ลด bounce
 * - นักเรียนสามารถเปรียบเทียบผลลัพธ์ได้จาก Serial Monitor
 * 
 * วงจร (ESP32 ICON-32):
 * 
 * === ปุ่ม 1: Software Debounce Only ===
 * 3.3V ──[10kΩ]──┐
 *                 │
 *               GPIO4 ──[Button]── GND
 * 
 * === ปุ่ม 2: Hardware Debounce (RC Filter) ===
 * 3.3V ──[10kΩ]──┐
 *                 │
 *               GPIO5 ──[Button]── GND
 *                 │
 *              [0.1µF]
 *                 │
 *                GND
 * 
 * LED สำหรับแสดงผล:
 * - LED1 (GPIO 2):  ปุ่ม 1 (Software Only)
 * - LED2 (GPIO 15): ปุ่ม 2 (Hardware + Software)
 * 
 * การทดสอบ:
 * - กดปุ่มทั้ง 2 ปุ่ม
 * - ดูจาก Serial ว่าปุ่มไหนมี bounce events น้อยกว่า
 * - ปุ่มที่มี RC Filter จะ stable กว่า
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <Button.h>

// ==================== ค่าคงที่ ====================
const int BUTTON1_PIN = 4;   // Software Debounce Only
const int BUTTON2_PIN = 5;   // Hardware Debounce (RC Filter)

const int LED1_PIN = 2;      // LED for Button 1
const int LED2_PIN = 15;     // LED for Button 2

// ==================== Objects ====================
Button button1(BUTTON1_PIN, true, 50);  // 50ms debounce
Button button2(BUTTON2_PIN, true, 50);  // 50ms debounce

// ==================== ตัวแปร ====================
bool led1State = false;
bool led2State = false;

int button1PressCount = 0;
int button2PressCount = 0;

unsigned long button1BounceEvents = 0;
unsigned long button2BounceEvents = 0;

// สำหรับตรวจจับ bounce manually
volatile unsigned long button1RawEvents = 0;
volatile unsigned long button2RawEvents = 0;

unsigned long lastButton1Event = 0;
unsigned long lastButton2Event = 0;

// ==================== ISR Functions ====================
void IRAM_ATTR button1ISR() {
  button1RawEvents++;
  unsigned long now = millis();
  if (now - lastButton1Event < 10) {  // ถ้าเกิดซ้ำภายใน 10ms = bounce
    button1BounceEvents++;
  }
  lastButton1Event = now;
}

void IRAM_ATTR button2ISR() {
  button2RawEvents++;
  unsigned long now = millis();
  if (now - lastButton2Event < 10) {  // ถ้าเกิดซ้ำภายใน 10ms = bounce
    button2BounceEvents++;
  }
  lastButton2Event = now;
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  : Hardware Debounce Comparison");
  Serial.println("===============================================");
  Serial.println();
  Serial.println("วงจร:");
  Serial.println("  🔵 Button 1 (GPIO 4):  Software Debounce Only");
  Serial.println("  🟢 Button 2 (GPIO 5):  Hardware RC Filter + Software");
  Serial.println();
  Serial.println("💡 RC Filter:");
  Serial.println("   R = 10kΩ (Pull-up)");
  Serial.println("   C = 0.1µF (100nF) to GND");
  Serial.println();
  
  // ตั้งค่า LED
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  
  // เริ่มต้น Button
  button1.begin();
  button2.begin();
  
  // ติดตั้ง Interrupt สำหรับตรวจจับ bounce
  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), button1ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), button2ISR, CHANGE);
  
  Serial.println("✓ ระบบพร้อมใช้งาน");
  Serial.println("✓ ลองกดปุ่มทั้ง 2 ปุ่มเพื่อเปรียบเทียบ");
  Serial.println();
}

// ==================== Loop ====================
void loop() {
  // อัปเดตสถานะปุ่ม
  button1.update();
  button2.update();
  
  // =============================================
  // ปุ่ม 1: Software Debounce Only
  // =============================================
  if (button1.wasPressed()) {
    button1PressCount++;
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  🔵 BUTTON 1 (Software Only)           ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.print("   Press Count:   ");
    Serial.println(button1PressCount);
    Serial.print("   Raw Events:    ");
    Serial.println(button1RawEvents);
    Serial.print("   Bounce Events: ");
    Serial.println(button1BounceEvents);
    Serial.print("   LED1: ");
    Serial.println(led1State ? "ON" : "OFF");
    Serial.println();
  }
  
  // =============================================
  // ปุ่ม 2: Hardware + Software Debounce
  // =============================================
  if (button2.wasPressed()) {
    button2PressCount++;
    led2State = !led2State;
    digitalWrite(LED2_PIN, led2State);
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  🟢 BUTTON 2 (Hardware RC Filter)      ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.print("   Press Count:   ");
    Serial.println(button2PressCount);
    Serial.print("   Raw Events:    ");
    Serial.println(button2RawEvents);
    Serial.print("   Bounce Events: ");
    Serial.println(button2BounceEvents);
    Serial.print("   LED2: ");
    Serial.println(led2State ? "ON" : "OFF");
    Serial.println();
  }
  
  // =============================================
  // รายงานเปรียบเทียบทุก 10 วินาที
  // =============================================
  static unsigned long lastReport = 0;
  unsigned long now = millis();
  
  if (now - lastReport >= 10000) {
    lastReport = now;
    
    Serial.println("═══════════════════════════════════════════════");
    Serial.println("              📊 COMPARISON REPORT");
    Serial.println("═══════════════════════════════════════════════");
    Serial.println();
    
    Serial.println("Button 1 (Software Only):");
    Serial.print("  Valid Presses:  ");
    Serial.println(button1PressCount);
    Serial.print("  Raw Events:     ");
    Serial.println(button1RawEvents);
    Serial.print("  Bounce Events:  ");
    Serial.print(button1BounceEvents);
    if (button1RawEvents > 0) {
      float bounceRate1 = (float)button1BounceEvents / button1RawEvents * 100;
      Serial.print(" (");
      Serial.print(bounceRate1, 1);
      Serial.print("%)");
    }
    Serial.println();
    Serial.println();
    
    Serial.println("Button 2 (Hardware RC Filter):");
    Serial.print("  Valid Presses:  ");
    Serial.println(button2PressCount);
    Serial.print("  Raw Events:     ");
    Serial.println(button2RawEvents);
    Serial.print("  Bounce Events:  ");
    Serial.print(button2BounceEvents);
    if (button2RawEvents > 0) {
      float bounceRate2 = (float)button2BounceEvents / button2RawEvents * 100;
      Serial.print(" (");
      Serial.print(bounceRate2, 1);
      Serial.print("%)");
    }
    Serial.println();
    Serial.println();
    
    // คำแนะนำ
    if (button2BounceEvents < button1BounceEvents) {
      Serial.println("✅ Hardware Debounce (RC Filter) มี bounce น้อยกว่า!");
      Serial.println("   → RC Filter ช่วยกรองสัญญาณได้ดีกว่า Software เพียงอย่างเดียว");
    } else if (button1BounceEvents < button2BounceEvents) {
      Serial.println("⚠️  Software Only มี bounce น้อยกว่า (ผิดปกติ)");
      Serial.println("   → ตรวจสอบ RC Filter ว่าต่อถูกต้องหรือไม่");
    } else {
      Serial.println("ℹ️  ยังไม่มีข้อมูลเพียงพอ - ลองกดปุ่มเพิ่มเติม");
    }
    
    Serial.println("═══════════════════════════════════════════════");
    Serial.println();
  }
}

/*
 * ==================== Hardware Debounce: RC Filter ====================
 * 
 * 📌 หลักการทำงาน:
 * 
 * R (Resistor) + C (Capacitor) สร้าง "Low-pass Filter"
 * → ทำให้สัญญาณเปลี่ยนช้าลง แต่เรียบขึ้น
 * 
 * เมื่อกดปุ่ม:
 * - ไม่มี C: สัญญาณกระโดดขึ้น-ลงเร็วมาก (bounce)
 * - มี C:    สัญญาณค่อย ๆ ลง เรียบขึ้น
 * 
 * 📐 การคำนวณ:
 * 
 * Time Constant (τ) = R × C
 * 
 * ตัวอย่าง:
 * R = 10kΩ = 10,000 Ω
 * C = 0.1µF = 0.0000001 F
 * τ = 10,000 × 0.0000001 = 0.001 s = 1 ms
 * 
 * สัญญาณจะใช้เวลา ~5τ = 5ms ในการ settle
 * 
 * 💡 แนวทางเลือก Capacitor:
 * 
 * | C (µF) | τ (ms) | 5τ (ms) | Use Case                |
 * |--------|--------|---------|-------------------------|
 * | 0.01   | 0.1    | 0.5     | ปุ่มคุณภาพสูง           |
 * | 0.1    | 1      | 5       | ปุ่มทั่วไป (แนะนำ)      |
 * | 0.47   | 4.7    | 23.5    | ปุ่มคุณภาพต่ำ           |
 * | 1.0    | 10     | 50      | ปุ่มเก่า/สัมผัสแย่      |
 * 
 * ⚠️ ข้อควรระวัง:
 * - C ใหญ่เกินไป → สัญญาณช้า, ตอบสนองช้า
 * - C เล็กเกินไป → กรอง bounce ไม่หมด
 * 
 * ==================== วงจรที่ถูกต้อง ====================
 * 
 *      3.3V
 *       │
 *     [10kΩ] ← Pull-up Resistor
 *       │
 *       ├──────┐
 *       │      │
 *      GPIO   [C]  ← Capacitor (0.1µF)
 *       │      │
 *   [Button]  GND
 *       │
 *      GND
 * 
 * 🔧 ตัวต้านทานต้องเป็น Pull-up (เชื่อม 3.3V)
 * 🔧 Capacitor ต่อจาก GPIO → GND
 * 🔧 ขา INPUT_PULLUP ของ ESP32 สามารถใช้แทน R ภายนอกได้
 * 
 * ==================== ผลลัพธ์ที่คาดหวัง ====================
 * 
 * ✅ ปุ่มที่มี RC Filter:
 * - Raw Events น้อยกว่า (สัญญาณไม่กระเพื่อม)
 * - Bounce Events น้อยกว่า
 * - Bounce Rate ต่ำกว่า
 * 
 * ⚠️ ถ้าผลตรงกันข้าม:
 * → ตรวจสอบว่า C ต่อถูกต้องหรือไม่
 * → ตรวจสอบว่า C ไม่ชำรุด
 * → ลองเปลี่ยน C ค่าอื่น (0.47µF หรือ 1µF)
 * 
 * ==================== การทดสอบ ====================
 * 
 * 1. กดปุ่มทั้ง 2 ปุ่มจำนวนเท่ากัน (เช่น 10 ครั้งต่อปุ่ม)
 * 2. สังเกต Bounce Events
 * 3. ปุ่มที่มี RC Filter ควรมี bounce น้อยกว่า
 * 4. ถ้ามี Oscilloscope → สังเกตสัญญาณดูจะเห็นความแตกต่างชัดเจน
 * 
 * ==================== สรุป ====================
 * 
 * Hardware Debounce (RC Filter):
 * ✅ ข้อดี:
 * - ลด bounce ที่ต้นทาง
 * - ลดภาระ CPU (ไม่ต้องประมวลผล bounce เยอะ)
 * - เสถียรกว่า
 * 
 * ❌ ข้อเสีย:
 * - ต้องใช้อุปกรณ์เพิ่ม (R + C)
 * - ต้องคำนวณค่าที่เหมาะสม
 * - C ใหญ่เกิน → ตอบสนองช้า
 * 
 * 💡 Best Practice:
 * → ใช้ทั้ง Hardware + Software Debounce ร่วมกัน
 * → RC Filter ลด bounce หยาบ
 * → Software Debounce จัดการที่เหลือ
 */
