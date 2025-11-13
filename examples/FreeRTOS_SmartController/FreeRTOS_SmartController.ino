/*
 * FreeRTOS_SmartController
 * 
 * EP7.2: FreeRTOS Multi-tasking - โปรเจกต์บูรณาการ
 * ระบบควบคุมอุณหภูมิอัจฉริยะพร้อม UI แบบครบวงจร
 * 
 * Integration: ใช้ทุก Library ที่สร้างมา
 * - DigitalLogic (7-segment display)
 * - Button (Advanced button detection)
 * - TemperatureSensor (DHT11/DHT22)
 * - RelayController (Fan control)
 * - LEDController (Status indicators)
 * 
 * Features:
 * - แสดงอุณหภูมิ/ความชื้นแบบ real-time
 * - ควบคุมพัดลมด้วย Hysteresis
 * - ปุ่มควบคุม: Mode, Manual Override, Temp Adjust
 * - LED status: Power, Fan, Warning, Manual Mode
 * - Serial logging
 * 
 * วงจร (ESP32 ICON-32):
 * 
 * === 7-Segment 4-Digit ===
 * Segments: GPIO 14,27,26,25,33,32,23,12 (a-g+dp)
 * Digits:   GPIO 22,21,19,18 → NPN → GND
 * 
 * === DHT22 Sensor ===
 * DATA: GPIO 4
 * 
 * === Buttons ===
 * MODE:   GPIO 5  (สลับ Auto/Manual)
 * UP:     GPIO 16 (เพิ่มอุณหภูมิ threshold)
 * DOWN:   GPIO 17 (ลดอุณหภูมิ threshold)
 * MANUAL: GPIO 18 (เปิด/ปิดพัดลมด้วยมือ)
 * 
 * === Relay (Fan Control) ===
 * RELAY: GPIO 13 → Relay Module → Fan
 * 
 * === Status LEDs ===
 * LED_POWER:  GPIO 2  (สีเขียว - ระบบทำงาน)
 * LED_FAN:    GPIO 15 (สีน้ำเงิน - พัดลมทำงาน)
 * LED_WARN:   GPIO 32 (สีแดง - อุณหภูมิสูงเกินไป)
 * LED_MANUAL: GPIO 33 (สีเหลือง - โหมด Manual)
 * 
 * Tasks:
 * 1. TaskDisplay    - สแกนจอ 7-segment @ 2ms (Priority 3 - สูงสุด)
 * 2. TaskButtons    - อ่านปุ่ม 4 ปุ่ม @ 10ms (Priority 2)
 * 3. TaskSensor     - อ่าน DHT22 @ 2s (Priority 1)
 * 4. TaskController - ควบคุมพัดลม @ 100ms (Priority 2)
 * 5. TaskLEDs       - อัปเดต LED status @ 100ms (Priority 1)
 * 6. TaskLogger     - บันทึก Serial log @ 5s (Priority 1)
 * 
 * Modes:
 * - AUTO: ควบคุมพัดลมตาม threshold + hysteresis
 * - MANUAL: ผู้ใช้เปิด/ปิดพัดลมเอง
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <DHT.h>
#include <Button.h>

// ==================== Configuration ====================
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// GPIO Pins
int seg[8] = {14, 27, 26, 25, 33, 32, 23, 12};  // a-g+dp
int digit[4] = {22, 21, 19, 18};

const int BTN_MODE = 5;
const int BTN_UP = 16;
const int BTN_DOWN = 17;
const int BTN_MANUAL = 19;

const int RELAY_FAN = 13;

const int LED_POWER = 2;
const int LED_FAN = 15;
const int LED_WARN = 32;
const int LED_MANUAL = 33;

// Buttons
AdvancedButton btnMode(BTN_MODE);
AdvancedButton btnUp(BTN_UP);
AdvancedButton btnDown(BTN_DOWN);
AdvancedButton btnManual(BTN_MANUAL);

// ==================== Segment Patterns ====================
byte numbers[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

// ==================== System State ====================
volatile float g_temp = 0.0;
volatile float g_humi = 0.0;

volatile bool g_autoMode = true;          // true=Auto, false=Manual
volatile float g_threshold = 30.0;        // °C
volatile float g_hysteresis = 2.0;        // °C
volatile bool g_fanState = false;
volatile bool g_manualOverride = false;

volatile bool g_warning = false;          // อุณหภูมิสูงเกินไป (>40°C)

// Display
int displayDigits[4] = {0,0,0,0};
bool displayDP[4] = {false, false, false, false};
int currentDigit = 0;

// ==================== Helper Functions ====================

void setSegments(int num, bool dp) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(seg[i], numbers[num][i]);
  }
  digitalWrite(seg[7], dp ? HIGH : LOW);
}

void updateDisplay() {
  int value = (int)(g_temp * 10);  // 27.3 → 273
  
  displayDigits[0] = (value / 100) % 10;
  displayDigits[1] = (value / 10) % 10;
  displayDigits[2] = value % 10;
  displayDigits[3] = 0;  // blank
  
  displayDP[0] = false;
  displayDP[1] = true;   // decimal point after second digit
  displayDP[2] = false;
  displayDP[3] = false;
}

// ==================== Tasks ====================

/**
 * Task 1: Display - สแกน 7-segment
 * Priority สูงสุด เพื่อไม่ให้จอกระพริบ
 */
void TaskDisplay(void *pvParameters) {
  (void) pvParameters;
  
  while (1) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(digit[i], HIGH);
    }
    
    setSegments(displayDigits[currentDigit], displayDP[currentDigit]);
    digitalWrite(digit[currentDigit], LOW);
    
    currentDigit = (currentDigit + 1) % 4;
    
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

/**
 * Task 2: Buttons - อ่านปุ่ม 4 ปุ่ม
 */
void TaskButtons(void *pvParameters) {
  (void) pvParameters;
  
  btnMode.begin();
  btnUp.begin();
  btnDown.begin();
  btnManual.begin();
  
  Serial.println("[Task] Buttons started");
  
  while (1) {
    btnMode.update();
    btnUp.update();
    btnDown.update();
    btnManual.update();
    
    // MODE: สลับ Auto/Manual
    if (btnMode.singleClick) {
      g_autoMode = !g_autoMode;
      Serial.print("[Mode] ");
      Serial.println(g_autoMode ? "AUTO" : "MANUAL");
    }
    
    // UP: เพิ่ม threshold
    if (btnUp.singleClick) {
      g_threshold += 0.5;
      if (g_threshold > 40.0) g_threshold = 40.0;
      Serial.print("[Threshold] ");
      Serial.print(g_threshold, 1);
      Serial.println("°C");
    }
    
    // DOWN: ลด threshold
    if (btnDown.singleClick) {
      g_threshold -= 0.5;
      if (g_threshold < 20.0) g_threshold = 20.0;
      Serial.print("[Threshold] ");
      Serial.print(g_threshold, 1);
      Serial.println("°C");
    }
    
    // MANUAL: สลับพัดลมในโหมด Manual
    if (btnManual.singleClick && !g_autoMode) {
      g_manualOverride = !g_manualOverride;
      Serial.print("[Manual] Fan ");
      Serial.println(g_manualOverride ? "ON" : "OFF");
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/**
 * Task 3: Sensor - อ่าน DHT22
 */
void TaskSensor(void *pvParameters) {
  (void) pvParameters;
  
  dht.begin();
  Serial.println("[Task] Sensor started");
  
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  
  while (1) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    
    if (!isnan(h) && !isnan(t)) {
      g_temp = t;
      g_humi = h;
      
      updateDisplay();
      
      // ตรวจสอบ warning
      g_warning = (t > 40.0);
    }
    
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

/**
 * Task 4: Controller - ควบคุมพัดลมด้วย Hysteresis
 */
void TaskController(void *pvParameters) {
  (void) pvParameters;
  
  Serial.println("[Task] Controller started");
  
  while (1) {
    bool newFanState = g_fanState;
    
    if (g_autoMode) {
      // โหมด AUTO: ใช้ Hysteresis
      if (!g_fanState && g_temp >= g_threshold) {
        // เปิดพัดลมเมื่อถึง threshold
        newFanState = true;
      }
      else if (g_fanState && g_temp <= (g_threshold - g_hysteresis)) {
        // ปิดพัดลมเมื่อต่ำกว่า threshold - hysteresis
        newFanState = false;
      }
    } else {
      // โหมด MANUAL: ใช้ manual override
      newFanState = g_manualOverride;
    }
    
    // อัปเดตสถานะ Relay
    if (newFanState != g_fanState) {
      g_fanState = newFanState;
      digitalWrite(RELAY_FAN, g_fanState ? HIGH : LOW);
      
      Serial.print("[Fan] ");
      Serial.println(g_fanState ? "ON" : "OFF");
    }
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * Task 5: LEDs - อัปเดต LED status
 */
void TaskLEDs(void *pvParameters) {
  (void) pvParameters;
  
  Serial.println("[Task] LEDs started");
  
  while (1) {
    // Power LED: เปิดเสมอ
    digitalWrite(LED_POWER, HIGH);
    
    // Fan LED: ตาม fanState
    digitalWrite(LED_FAN, g_fanState ? HIGH : LOW);
    
    // Warning LED: กระพริบเมื่อ warning
    if (g_warning) {
      digitalWrite(LED_WARN, (millis() / 500) % 2);  // กระพริบทุก 500ms
    } else {
      digitalWrite(LED_WARN, LOW);
    }
    
    // Manual LED: เปิดเมื่อโหมด Manual
    digitalWrite(LED_MANUAL, !g_autoMode ? HIGH : LOW);
    
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

/**
 * Task 6: Logger - บันทึก log ทุก 5 วินาที
 */
void TaskLogger(void *pvParameters) {
  (void) pvParameters;
  
  Serial.println("[Task] Logger started");
  
  while (1) {
    Serial.println("─────────────────────────────────────");
    Serial.print("Temp: ");
    Serial.print(g_temp, 1);
    Serial.print("°C  Humi: ");
    Serial.print(g_humi, 1);
    Serial.println("%");
    
    Serial.print("Mode: ");
    Serial.print(g_autoMode ? "AUTO" : "MANUAL");
    Serial.print("  Threshold: ");
    Serial.print(g_threshold, 1);
    Serial.println("°C");
    
    Serial.print("Fan: ");
    Serial.print(g_fanState ? "ON " : "OFF");
    Serial.print("  Warning: ");
    Serial.println(g_warning ? "YES" : "NO");
    Serial.println();
    
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  EP7.2: FreeRTOS Smart Controller");
  Serial.println("  Integration Project - All Libraries");
  Serial.println("===============================================");
  Serial.println();
  
  // GPIO Setup
  for (int i = 0; i < 8; i++) {
    pinMode(seg[i], OUTPUT);
    digitalWrite(seg[i], LOW);
  }
  
  for (int i = 0; i < 4; i++) {
    pinMode(digit[i], OUTPUT);
    digitalWrite(digit[i], HIGH);
  }
  
  pinMode(RELAY_FAN, OUTPUT);
  digitalWrite(RELAY_FAN, LOW);
  
  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_FAN, OUTPUT);
  pinMode(LED_WARN, OUTPUT);
  pinMode(LED_MANUAL, OUTPUT);
  
  // Create Tasks
  xTaskCreate(TaskDisplay, "Display", 2048, NULL, 3, NULL);
  xTaskCreate(TaskButtons, "Buttons", 4096, NULL, 2, NULL);
  xTaskCreate(TaskSensor, "Sensor", 4096, NULL, 1, NULL);
  xTaskCreate(TaskController, "Controller", 2048, NULL, 2, NULL);
  xTaskCreate(TaskLEDs, "LEDs", 2048, NULL, 1, NULL);
  xTaskCreate(TaskLogger, "Logger", 2048, NULL, 1, NULL);
  
  Serial.println("✓ All 6 tasks created");
  Serial.println();
  Serial.println("Controls:");
  Serial.println("  MODE:   Auto/Manual mode");
  Serial.println("  UP:     Increase threshold");
  Serial.println("  DOWN:   Decrease threshold");
  Serial.println("  MANUAL: Toggle fan (Manual mode only)");
  Serial.println();
}

void loop() {
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

/*
 * ==================== System Architecture ====================
 * 
 * Task Priority Design:
 * 
 * Priority 3 (สูงสุด):
 * - TaskDisplay: ต้องสแกนเร็วมาก ไม่ให้กระพริบ
 * 
 * Priority 2 (กลาง-สูง):
 * - TaskButtons: ต้องตอบสนองไว
 * - TaskController: ควบคุม real-time
 * 
 * Priority 1 (ปกติ):
 * - TaskSensor: อ่านช้าได้ (2s)
 * - TaskLEDs: ไม่เร่งด่วน
 * - TaskLogger: ไม่เร่งด่วน
 * 
 * ==================== Control Logic ====================
 * 
 * AUTO Mode (Hysteresis Control):
 * - Fan ON:  temp >= threshold
 * - Fan OFF: temp <= threshold - hysteresis
 * 
 * ตัวอย่าง (threshold=30°C, hysteresis=2°C):
 * - 29.9°C → Fan OFF
 * - 30.0°C → Fan ON
 * - 29.0°C → Fan still ON
 * - 28.0°C → Fan OFF
 * 
 * → ป้องกัน relay กระพริบเร็วเกินไป
 * 
 * MANUAL Mode:
 * - ผู้ใช้ควบคุมเอง ผ่านปุ่ม MANUAL
 * 
 * ==================== Safety Features ====================
 * 
 * 1. Warning LED กระพริบเมื่อ temp > 40°C
 * 2. Threshold จำกัด 20-40°C
 * 3. Hysteresis ป้องกัน relay สึก
 * 4. Serial logging สำหรับ debug
 * 
 * ==================== การต่อยอด ====================
 * 
 * เพิ่ม Features:
 * 1. WiFi - ส่งข้อมูลขึ้น Cloud
 * 2. SD Card - บันทึก historical data
 * 3. ปุ่มเพิ่ม - ปรับ hysteresis
 * 4. Buzzer - แจ้งเตือนเสียง
 * 5. OLED Display - แสดงผลเพิ่มเติม
 * 6. Schedule - ควบคุมตามเวลา
 * 7. PID Controller - ควบคุมแม่นยำขึ้น
 * 
 * ==================== Real-World Applications ====================
 * 
 * ระบบนี้เหมาะกับ:
 * - ระบบควบคุมอุณหภูมิห้อง
 * - เรือนกระจก (Greenhouse)
 * - ตู้เย็น/ตู้อบ
 * - ห้องเซิร์ฟเวอร์
 * - โรงเพาะเลี้ยงสัตว์
 * 
 * ==================== Performance Notes ====================
 * 
 * CPU Usage (ประมาณการ):
 * - TaskDisplay: ~15% (ทำงานบ่อยที่สุด)
 * - TaskButtons: ~5%
 * - TaskSensor: ~2%
 * - Others: <5%
 * - Total: ~30-40% (เหลือ CPU ให้ Task อื่นมาก)
 * 
 * Memory Usage:
 * - Total Stack: ~20KB
 * - Heap: ~15KB
 * - Flash: ~150KB
 * 
 * → เหลือ resource มากสำหรับ WiFi, Bluetooth
 */
