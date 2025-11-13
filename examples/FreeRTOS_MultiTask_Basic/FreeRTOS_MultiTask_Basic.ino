/*
 * FreeRTOS_MultiTask_Basic
 * 
 * EP7.2: FreeRTOS Multi-tasking
 * ตัวอย่างพื้นฐาน - แยกงานเป็น 3 Tasks ทำงานพร้อมกัน
 * 
 * แนวคิด:
 * - ESP32 มี FreeRTOS อยู่แล้ว สามารถสร้าง Task หลายตัวได้
 * - แต่ละ Task มีลูปของตัวเอง ทำงานแบบ concurrent
 * - ไม่ต้องกังวลเรื่อง delay() บล็อก loop() หลัก
 * 
 * Tasks:
 * 1. TaskDisplay  - สแกน 4-digit 7-segment (ความถี่สูง 2ms)
 * 2. TaskButtons  - อ่านปุ่มเปลี่ยนโหมด (10ms polling)
 * 3. TaskSensor   - อ่าน DHT11 ทุก 1 วินาที
 * 
 * วงจร (ESP32 ICON-32):
 * 
 * === 7-Segment 4-Digit (Common Cathode) ===
 * Segments (a-g): GPIO 14,27,26,25,33,32,23
 * Digits (1-4):   GPIO 22,21,19,18 → NPN transistors → GND
 * 
 * === DHT11 Sensor ===
 * DATA: GPIO 4
 * VCC:  3.3V
 * GND:  GND
 * 
 * === Button ===
 * GPIO 5 → Button → GND (INPUT_PULLUP)
 * 
 * พฤติกรรม:
 * - จอแสดงอุณหภูมิ/ความชื้นจาก DHT11
 * - กดปุ่ม → สลับระหว่าง Temp (°C×10) และ Humidity (%)
 * - ทุก Task ทำงานพร้อมกัน ไม่บล็อคกัน
 * 
 * Dependencies:
 * - DHT sensor library by Adafruit
 * - Adafruit Unified Sensor
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <DHT.h>

// ==================== DHT11 Configuration ====================
#define DHTPIN   4
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

// ==================== 7-Segment Pin Configuration ====================
// Common Cathode, 4-digit display
int seg[7] = {14, 27, 26, 25, 33, 32, 23};  // a, b, c, d, e, f, g
int digit[4] = {22, 21, 19, 18};            // digit1, digit2, digit3, digit4

// ==================== Button Pin ====================
const int BTN_MODE_PIN = 5;

// ==================== Segment Patterns (0-9) ====================
byte numbers[10][7] = {
  {1,1,1,1,1,1,0},  // 0
  {0,1,1,0,0,0,0},  // 1
  {1,1,0,1,1,0,1},  // 2
  {1,1,1,1,0,0,1},  // 3
  {0,1,1,0,0,1,1},  // 4
  {1,0,1,1,0,1,1},  // 5
  {1,0,1,1,1,1,1},  // 6
  {1,1,1,0,0,0,0},  // 7
  {1,1,1,1,1,1,1},  // 8
  {1,1,1,1,0,1,1}   // 9
};

// ==================== Shared Variables (volatile) ====================
volatile float g_temperature = 0.0;    // อุณหภูมิ (°C)
volatile float g_humidity = 0.0;       // ความชื้น (%)
volatile int g_displayMode = 0;        // 0 = Temp, 1 = Humidity
volatile int g_displayValue = 0;       // ค่าที่แสดงบนจอ (0-9999)

// สำหรับการสแกนจอ
int currentDigit = 0;

// ==================== Helper Functions ====================

/**
 * แสดงตัวเลขหนึ่งหลักบน 7-segment
 */
void setSegments(int num) {
  if (num < 0 || num > 9) num = 0;
  for (int i = 0; i < 7; i++) {
    digitalWrite(seg[i], numbers[num][i]);
  }
}

/**
 * แปลงค่าเป็น 4 หลัก
 */
void extractDigits(int value, int digits[4]) {
  digits[0] = (value / 1000) % 10;
  digits[1] = (value / 100) % 10;
  digits[2] = (value / 10) % 10;
  digits[3] = value % 10;
}

// ==================== Task 1: Display ====================
/**
 * TaskDisplay - สแกน 7-segment 4 หลักด้วย multiplexing
 * Priority: 2 (สูง) - ต้องสแกนเร็วเพื่อไม่ให้กระพริบ
 * Delay: 2ms ต่อหลัก (ความถี่สแกน ~125Hz)
 */
void TaskDisplay(void *pvParameters) {
  (void) pvParameters;
  
  Serial.println("[Task] Display started");
  
  while (1) {
    // อ่านค่าจากตัวแปรแชร์
    int value = g_displayValue;
    
    // แยกเป็น 4 หลัก
    int d[4];
    extractDigits(value, d);
    
    // ปิดทุก digit ก่อน (multiplexing)
    for (int i = 0; i < 4; i++) {
      digitalWrite(digit[i], HIGH);  // HIGH = OFF (NPN base)
    }
    
    // เปิด digit ปัจจุบัน
    setSegments(d[currentDigit]);
    digitalWrite(digit[currentDigit], LOW);  // LOW = ON
    
    // เลื่อนไปหลักถัดไป
    currentDigit = (currentDigit + 1) % 4;
    
    // หน่วงเวลาสั้น (ไม่บล็อก CPU)
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

// ==================== Task 2: Buttons ====================
/**
 * TaskButtons - อ่านปุ่มและเปลี่ยนโหมดแสดงผล
 * Priority: 1 (กลาง)
 * Delay: 10ms (polling rate 100Hz)
 */
void TaskButtons(void *pvParameters) {
  (void) pvParameters;
  
  pinMode(BTN_MODE_PIN, INPUT_PULLUP);
  bool lastState = HIGH;
  
  Serial.println("[Task] Buttons started");
  
  while (1) {
    bool reading = digitalRead(BTN_MODE_PIN);
    
    // ตรวจจับขอบลง (pressed)
    if (lastState == HIGH && reading == LOW) {
      // เปลี่ยนโหมด
      g_displayMode = (g_displayMode + 1) % 2;
      
      Serial.print("[Button] Mode changed to: ");
      Serial.println(g_displayMode == 0 ? "Temperature" : "Humidity");
      
      // Debounce แบบง่าย
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    
    lastState = reading;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ==================== Task 3: Sensor ====================
/**
 * TaskSensor - อ่านค่า DHT11 และอัปเดตค่าที่แสดง
 * Priority: 1 (กลาง)
 * Delay: 1000ms (อ่านทุก 1 วินาที)
 */
void TaskSensor(void *pvParameters) {
  (void) pvParameters;
  
  dht.begin();
  
  Serial.println("[Task] Sensor started");
  Serial.println("[Sensor] Waiting for DHT11 to stabilize...");
  
  // รอให้ DHT11 เสถียร
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  
  while (1) {
    // อ่านค่าจาก DHT11
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    // ตรวจสอบว่าอ่านได้หรือไม่
    if (!isnan(h) && !isnan(t)) {
      // อัปเดตค่าแชร์
      g_temperature = t;
      g_humidity = h;
      
      // คำนวณค่าที่จะแสดงบนจอ
      int showVal = 0;
      
      if (g_displayMode == 0) {
        // โหมดอุณหภูมิ: แสดง xx.x → xxx (x10)
        // เช่น 27.3°C → แสดง "273"
        showVal = (int)(g_temperature * 10);
      } else {
        // โหมดความชื้น: แสดง xx% → xx
        // เช่น 65% → แสดง "65"
        showVal = (int)g_humidity;
      }
      
      // จำกัดขอบเขต 0-9999
      if (showVal < 0) showVal = 0;
      if (showVal > 9999) showVal = 9999;
      
      g_displayValue = showVal;
      
      // Debug output
      Serial.print("[Sensor] Temp: ");
      Serial.print(t, 1);
      Serial.print("°C  Humidity: ");
      Serial.print(h, 1);
      Serial.print("%  Display: ");
      Serial.println(showVal);
      
    } else {
      Serial.println("[Sensor] Failed to read from DHT sensor!");
    }
    
    // อ่านทุก 1 วินาที
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  EP7.2: FreeRTOS Multi-tasking Basic");
  Serial.println("===============================================");
  Serial.println();
  Serial.println("Tasks:");
  Serial.println("  1. TaskDisplay - Scan 7-segment @ 2ms");
  Serial.println("  2. TaskButtons - Read button @ 10ms");
  Serial.println("  3. TaskSensor  - Read DHT11 @ 1s");
  Serial.println();
  
  // ==================== ตั้งค่าขา GPIO ====================
  
  // 7-Segment pins
  for (int i = 0; i < 7; i++) {
    pinMode(seg[i], OUTPUT);
    digitalWrite(seg[i], LOW);
  }
  
  for (int i = 0; i < 4; i++) {
    pinMode(digit[i], OUTPUT);
    digitalWrite(digit[i], HIGH);  // ปิดทุก digit
  }
  
  // ==================== สร้าง Tasks ====================
  
  Serial.println("Creating tasks...");
  
  // Task 1: Display (Priority สูง - ต้องสแกนเร็ว)
  xTaskCreate(
    TaskDisplay,      // ฟังก์ชัน Task
    "Display",        // ชื่อ Task
    2048,             // Stack size (words)
    NULL,             // Parameters
    2,                // Priority (สูงกว่า = สำคัญกว่า)
    NULL              // Task handle
  );
  
  // Task 2: Buttons (Priority กลาง)
  xTaskCreate(
    TaskButtons,
    "Buttons",
    2048,
    NULL,
    1,
    NULL
  );
  
  // Task 3: Sensor (Priority กลาง, Stack ใหญ่กว่า - DHT library ใช้เยอะ)
  xTaskCreate(
    TaskSensor,
    "Sensor",
    4096,
    NULL,
    1,
    NULL
  );
  
  Serial.println("✓ All tasks created");
  Serial.println("✓ FreeRTOS scheduler running");
  Serial.println();
}

// ==================== Loop ====================
void loop() {
  // loop() ว่างเปล่า - ทุกอย่างทำงานใน Tasks
  // (FreeRTOS จะจัดการให้เอง)
  
  // ถ้าต้องการทำอะไรเพิ่มก็ทำได้ เช่น Serial debug
  vTaskDelay(5000 / portTICK_PERIOD_MS);
}

/*
 * ==================== หมายเหตุสำคัญ ====================
 * 
 * 📌 FreeRTOS on ESP32
 * 
 * ESP32 มี FreeRTOS มาพร้อม:
 * - Core 0: WiFi/Bluetooth stack
 * - Core 1: Arduino loop() + Tasks ที่เราสร้าง
 * 
 * ==================== Task Priority ====================
 * 
 * ค่า Priority (1-25):
 * - 0 = Idle task (ต่ำสุด)
 * - 1 = ปกติ (เหมาะสำหรับงานทั่วไป)
 * - 2-5 = สูง (งานที่ต้องตอบสนองเร็ว)
 * - > 5 = สูงมาก (ใช้เฉพาะงาน real-time)
 * 
 * ในตัวอย่างนี้:
 * - TaskDisplay = 2 (ต้องสแกนเร็ว ไม่ให้จอกระพริบ)
 * - TaskButtons = 1 (ไม่เร่งด่วน)
 * - TaskSensor  = 1 (ไม่เร่งด่วน)
 * 
 * ==================== vTaskDelay() ====================
 * 
 * ⚠️ ห้ามใช้ delay() ใน Task!
 * 
 * ✅ ใช้ vTaskDelay() แทน:
 * 
 * vTaskDelay(ms / portTICK_PERIOD_MS);
 * 
 * ข้อดี:
 * - ปล่อย CPU ให้ Task อื่นทำงาน
 * - ไม่บล็อกทั้งระบบ
 * - Scheduler จัดการให้
 * 
 * ==================== Shared Variables ====================
 * 
 * ตัวแปรที่ใช้ร่วมกันระหว่าง Tasks:
 * - ต้องเป็น volatile
 * - ระวังเรื่อง race condition
 * 
 * ในตัวอย่างนี้:
 * - g_temperature, g_humidity → เขียนโดย TaskSensor
 * - g_displayMode → เขียนโดย TaskButtons
 * - g_displayValue → เขียนโดย TaskSensor, อ่านโดย TaskDisplay
 * 
 * สำหรับ data ซับซ้อน → ใช้ Semaphore, Mutex, Queue
 * (จะสอนใน EP7.2 Advanced)
 * 
 * ==================== Stack Size ====================
 * 
 * Stack คือหน่วยความจำสำหรับ Task:
 * - 2048 = เพียงพอสำหรับงานง่าย ๆ
 * - 4096 = งานที่ใช้ library ซับซ้อน (DHT, WiFi)
 * - 8192+ = งานที่ใช้ buffer ใหญ่
 * 
 * ถ้า Stack overflow → ESP32 จะ panic/reset
 * → เพิ่ม Stack size ให้มากขึ้น
 * 
 * ==================== Debugging ====================
 * 
 * ดู Task status:
 * 
 * Serial.print("Free heap: ");
 * Serial.println(ESP.getFreeHeap());
 * 
 * Serial.print("Task stack watermark: ");
 * Serial.println(uxTaskGetStackHighWaterMark(NULL));
 * 
 * ==================== ข้อดีของ FreeRTOS ====================
 * 
 * ✅ แยกงานชัดเจน (Separation of Concerns)
 * ✅ ไม่ต้องกังวลเรื่อง delay() บล็อก
 * ✅ แต่ละ Task มี timing ของตัวเอง
 * ✅ ขยายงานง่าย (เพิ่ม Task ใหม่)
 * ✅ เหมาะกับ IoT, Smart Home, Industrial
 * 
 * ==================== เปรียบเทียบ ====================
 * 
 * แบบ Single Loop (EP1-EP6):
 * - ทุกอย่างใน loop()
 * - ถ้ามี delay() นาน → ทุกอย่างหยุด
 * - ยากต่อการจัดการเวลา
 * 
 * แบบ FreeRTOS (EP7.2):
 * - แต่ละงานมี Task
 * - vTaskDelay() ไม่บล็อก Task อื่น
 * - ง่ายต่อการออกแบบระบบซับซ้อน
 * 
 * ==================== การต่อยอด ====================
 * 
 * ลองเพิ่ม Tasks ใหม่:
 * 
 * 1. TaskBlink - กระพริบ LED status
 * 2. TaskWiFi - ส่งข้อมูลขึ้น Cloud
 * 3. TaskSD - บันทึกข้อมูลลง SD card
 * 4. TaskAlarm - ตรวจสอบเงื่อนไข แล้วส่งแจ้งเตือน
 * 
 * ==================== Safety Tips ====================
 * 
 * ⚠️ อย่าใช้ Serial.print() มากเกินไปใน Task ความถี่สูง
 * → จะทำให้ Task ช้า
 * 
 * ⚠️ ระวัง Stack Overflow
 * → เพิ่ม Stack size ถ้าเจอ panic
 * 
 * ⚠️ ระวัง Race Condition
 * → ใช้ Mutex/Semaphore สำหรับ shared data ซับซ้อน
 * 
 * ⚠️ Priority Inversion
 * → อย่าให้ Task priority สูงรอ Task priority ต่ำ
 */
