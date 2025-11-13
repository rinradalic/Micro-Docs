/*
 * FreeRTOS_TempDisplay
 * 
 * EP7.2: FreeRTOS Multi-tasking
 * แสดงอุณหภูมิ/ความชื้นบน 7-segment พร้อมควบคุมด้วยปุ่ม
 * 
 * Features:
 * - แสดงอุณหภูมิแบบ xx.x°C (ใช้จุดทศนิยม)
 * - 3 ปุ่ม: MODE (สลับ Temp/Humi), UP, DOWN (ปรับ threshold)
 * - LED แจ้งเตือนเมื่อเกิน threshold
 * - ทั้งหมดทำงานพร้อมกันด้วย FreeRTOS
 * 
 * วงจร (ESP32 ICON-32):
 * 
 * === 7-Segment 4-Digit (Common Cathode) ===
 * Segments (a-g + dp): GPIO 14,27,26,25,33,32,23,12
 * Digits (1-4):         GPIO 22,21,19,18 → NPN → GND
 * 
 * === DHT11 ===
 * DATA: GPIO 4
 * 
 * === Buttons (INPUT_PULLUP) ===
 * MODE: GPIO 5
 * UP:   GPIO 16
 * DOWN: GPIO 17
 * 
 * === LED ===
 * WARNING LED: GPIO 13
 * 
 * Tasks:
 * 1. TaskDisplay    - สแกนจอ @ 2ms
 * 2. TaskButtons    - อ่านปุ่ม 3 ปุ่ม @ 10ms
 * 3. TaskSensor     - อ่าน DHT11 @ 1s
 * 4. TaskMonitor    - ตรวจสอบ threshold @ 500ms
 * 
 * Author: Micro-Docs Library
 * License: MIT
 */

#include <DHT.h>
#include <Button.h>

// ==================== Configuration ====================
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pins
int seg[8] = {14, 27, 26, 25, 33, 32, 23, 12};  // a-g + dp
int digit[4] = {22, 21, 19, 18};
const int BTN_MODE = 5;
const int BTN_UP = 16;
const int BTN_DOWN = 17;
const int LED_WARNING = 13;

// Buttons
Button btnMode(BTN_MODE);
Button btnUp(BTN_UP);
Button btnDown(BTN_DOWN);

// ==================== Segment Patterns ====================
byte numbers[10][7] = {
  {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1}, {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

// ==================== Shared Variables ====================
volatile float g_temp = 0.0;
volatile float g_humi = 0.0;
volatile int g_mode = 0;              // 0=Temp, 1=Humi
volatile int g_threshold = 300;       // อุณหภูมิ threshold (×10) = 30.0°C
volatile bool g_warning = false;      // แจ้งเตือนเกิน threshold

// Display data
int displayDigits[4] = {0,0,0,0};
bool displayDP[4] = {false, false, false, false};  // จุดทศนิยม
int currentDigit = 0;

// ==================== Helper Functions ====================

void setSegments(int num, bool dp) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(seg[i], numbers[num][i]);
  }
  digitalWrite(seg[7], dp ? HIGH : LOW);  // Decimal point
}

void updateDisplay() {
  int value = 0;
  bool showDP = false;
  
  if (g_mode == 0) {
    // Temperature: xx.x°C
    value = (int)(g_temp * 10);
    // แสดงจุดหลังหลักที่ 2 (เช่น 27.3 → 2|7.|3)
    displayDigits[0] = (value / 100) % 10;
    displayDigits[1] = (value / 10) % 10;
    displayDigits[2] = value % 10;
    displayDigits[3] = 12;  // "C" symbol (or blank)
    
    displayDP[0] = false;
    displayDP[1] = true;   // จุดทศนิยม
    displayDP[2] = false;
    displayDP[3] = false;
    
  } else {
    // Humidity: xx%
    value = (int)g_humi;
    displayDigits[0] = 0;  // leading zero
    displayDigits[1] = (value / 10) % 10;
    displayDigits[2] = value % 10;
    displayDigits[3] = 11;  // "H" symbol (or blank)
    
    for (int i = 0; i < 4; i++) displayDP[i] = false;
  }
}

// ==================== Tasks ====================

void TaskDisplay(void *pvParameters) {
  (void) pvParameters;
  
  while (1) {
    // ปิดทุก digit
    for (int i = 0; i < 4; i++) {
      digitalWrite(digit[i], HIGH);
    }
    
    // แสดง digit ปัจจุบัน
    setSegments(displayDigits[currentDigit], displayDP[currentDigit]);
    digitalWrite(digit[currentDigit], LOW);
    
    currentDigit = (currentDigit + 1) % 4;
    
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

void TaskButtons(void *pvParameters) {
  (void) pvParameters;
  
  btnMode.begin();
  btnUp.begin();
  btnDown.begin();
  
  Serial.println("[Task] Buttons started");
  
  while (1) {
    btnMode.update();
    btnUp.update();
    btnDown.update();
    
    // MODE button - สลับโหมด
    if (btnMode.wasPressed()) {
      g_mode = (g_mode + 1) % 2;
      Serial.print("[Button] Mode: ");
      Serial.println(g_mode == 0 ? "Temperature" : "Humidity");
    }
    
    // UP button - เพิ่ม threshold
    if (btnUp.wasPressed()) {
      g_threshold += 10;  // +1.0°C
      if (g_threshold > 500) g_threshold = 500;  // Max 50.0°C
      Serial.print("[Button] Threshold: ");
      Serial.print(g_threshold / 10.0, 1);
      Serial.println("°C");
    }
    
    // DOWN button - ลด threshold
    if (btnDown.wasPressed()) {
      g_threshold -= 10;  // -1.0°C
      if (g_threshold < 100) g_threshold = 100;  // Min 10.0°C
      Serial.print("[Button] Threshold: ");
      Serial.print(g_threshold / 10.0, 1);
      Serial.println("°C");
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

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
      
      Serial.print("[Sensor] T:");
      Serial.print(t, 1);
      Serial.print("°C H:");
      Serial.print(h, 1);
      Serial.println("%");
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskMonitor(void *pvParameters) {
  (void) pvParameters;
  
  Serial.println("[Task] Monitor started");
  
  while (1) {
    // ตรวจสอบว่าอุณหภูมิเกิน threshold หรือไม่
    int tempValue = (int)(g_temp * 10);
    
    if (tempValue > g_threshold) {
      g_warning = true;
      digitalWrite(LED_WARNING, HIGH);
      
      // แจ้งเตือนทุก 5 วินาที
      static unsigned long lastWarn = 0;
      if (millis() - lastWarn > 5000) {
        Serial.println("⚠️  WARNING: Temperature exceeded threshold!");
        lastWarn = millis();
      }
    } else {
      g_warning = false;
      digitalWrite(LED_WARNING, LOW);
    }
    
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// ==================== Setup ====================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("===============================================");
  Serial.println("  EP7.2: FreeRTOS Temperature Display");
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
  
  pinMode(LED_WARNING, OUTPUT);
  digitalWrite(LED_WARNING, LOW);
  
  // Create Tasks
  xTaskCreate(TaskDisplay, "Display", 2048, NULL, 2, NULL);
  xTaskCreate(TaskButtons, "Buttons", 2048, NULL, 1, NULL);
  xTaskCreate(TaskSensor, "Sensor", 4096, NULL, 1, NULL);
  xTaskCreate(TaskMonitor, "Monitor", 2048, NULL, 1, NULL);
  
  Serial.println("✓ All tasks created");
  Serial.println();
  Serial.println("Controls:");
  Serial.println("  MODE button: Switch Temp/Humidity");
  Serial.println("  UP button:   Increase threshold");
  Serial.println("  DOWN button: Decrease threshold");
  Serial.println();
}

void loop() {
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

/*
 * ==================== Features ====================
 * 
 * 1. แสดงอุณหภูมิพร้อมจุดทศนิยม (27.3°C)
 * 2. สลับแสดงความชื้น (65%)
 * 3. ปรับ threshold ได้ด้วยปุ่ม UP/DOWN
 * 4. LED แจ้งเตือนเมื่ออุณหภูมิเกิน threshold
 * 5. ทุก Task ทำงานพร้อมกัน
 * 
 * ==================== การต่อยอด ====================
 * 
 * เพิ่ม Features:
 * - บันทึกข้อมูลลง SD card
 * - ส่งข้อมูลขึ้น Cloud (MQTT/HTTP)
 * - เพิ่ม Relay ควบคุมพัดลม/ฮีตเตอร์
 * - แสดง min/max temperature
 * - Graph แบบ LED bar
 */
