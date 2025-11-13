/**
 * Digital Clock with 4-Digit 7-Segment Display
 * 
 * โปรเจคนาฬิกาดิจิทัลแบบง่าย แสดงผลเวลา HH:MM บน 7-segment 4 หลัก
 * ใช้ millis() สำหรับจับเวลา (ไม่ต้องใช้ RTC module)
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - 7-segment display 4 หลัก แบบ Common Cathode
 * - ทรานซิสเตอร์ NPN 4 ตัว
 * - ตัวต้านทาน 220Ω x 7 และ 10kΩ x 4
 * - ปุ่มกด 3 ตัว (HOUR+, MIN+, MODE)
 * 
 * การต่อวงจร:
 * - Segments: GPIO 14,27,26,25,33,32,23 (a-g)
 * - Digits: GPIO 22,21,19,18 (D1-D4)
 * - Buttons: GPIO 4 (HOUR+), GPIO 5 (MIN+), GPIO 17 (MODE)
 * 
 * คุณสมบัติ:
 * - แสดงเวลา 24 ชั่วโมง (00:00 - 23:59)
 * - ปุ่มตั้งเวลา
 * - โหมด 12/24 ชั่วโมง
 * - กะพริบเครื่องหมาย ":" ทุกวินาที (ใช้ decimal point)
 * 
 * หมายเหตุ:
 * - นาฬิกานี้ใช้ millis() ดังนั้นความแม่นยำขึ้นกับ crystal ของ ESP32
 * - สำหรับความแม่นยำสูง ควรใช้ DS3231 RTC module
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
const int DIGIT_1 = 22;  // Hour tens
const int DIGIT_2 = 21;  // Hour ones
const int DIGIT_3 = 19;  // Minute tens
const int DIGIT_4 = 18;  // Minute ones

// Pin definitions - Buttons
const int BTN_HOUR = 4;
const int BTN_MIN = 5;
const int BTN_MODE = 17;

// Create display object
SevenSegmentMultiplex display(
  SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G,
  DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4,
  true
);

// Time variables
int hours = 0;
int minutes = 0;
int seconds = 0;
unsigned long lastSecond = 0;

// Display settings
bool format24Hour = true;  // true = 24-hour, false = 12-hour
bool colonBlink = true;    // Blink colon every second
bool colonState = true;

// Mode
bool settingMode = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("============================================");
  Serial.println("     Digital Clock - 4-Digit Display");
  Serial.println("============================================\n");
  
  // Initialize display
  display.begin();
  display.setLeadingZeros(true);  // Show "00:00" not "0:0"
  
  // Initialize buttons
  pinMode(BTN_HOUR, INPUT_PULLUP);
  pinMode(BTN_MIN, INPUT_PULLUP);
  pinMode(BTN_MODE, INPUT_PULLUP);
  
  Serial.println("✓ Digital clock initialized");
  Serial.println("  Format: 24-hour");
  Serial.println("  Time: 00:00:00\n");
  
  Serial.println("Controls:");
  Serial.println("  Button HOUR (GPIO 4)  - Increase hour");
  Serial.println("  Button MIN (GPIO 5)   - Increase minute");
  Serial.println("  Button MODE (GPIO 17) - Toggle setting mode\n");
  
  Serial.println("Serial Commands:");
  Serial.println("  h - Increase hour");
  Serial.println("  m - Increase minute");
  Serial.println("  f - Toggle 12/24 hour format");
  Serial.println("  s - Toggle setting mode");
  Serial.println("  t HHMM - Set time (e.g., 't 1430' for 14:30)");
  Serial.println("  i - Show info\n");
  
  // Set initial time (can be changed via serial or buttons)
  setTime(12, 0, 0);
  
  Serial.println("⏰ Clock started!\n");
}

void loop() {
  // Update display scanning
  display.update();
  
  // Update time
  updateTime();
  
  // Check buttons
  checkButtons();
  
  // Check serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
  
  // Update display with current time
  displayTime();
}

void updateTime() {
  unsigned long now = millis();
  
  if (now - lastSecond >= 1000) {
    lastSecond = now;
    
    if (!settingMode) {
      seconds++;
      
      if (seconds >= 60) {
        seconds = 0;
        minutes++;
        
        if (minutes >= 60) {
          minutes = 0;
          hours++;
          
          if (hours >= 24) {
            hours = 0;
          }
          
          printTime();
        }
      }
      
      // Toggle colon blink
      if (colonBlink) {
        colonState = !colonState;
      }
    }
  }
}

void displayTime() {
  int displayHours = hours;
  
  // Convert to 12-hour format if needed
  if (!format24Hour) {
    if (displayHours == 0) {
      displayHours = 12;
    } else if (displayHours > 12) {
      displayHours -= 12;
    }
  }
  
  // Show time as HHMM
  display.showTime(displayHours, minutes);
  
  // Set decimal point on digit 1 to simulate colon
  // (Note: This requires hardware support for individual decimal points)
  // display.setDecimalPoint(1, colonState);
}

void checkButtons() {
  // HOUR button
  static bool lastHourState = HIGH;
  bool hourState = digitalRead(BTN_HOUR);
  
  if (hourState == LOW && lastHourState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_HOUR) == LOW) {
      hours++;
      if (hours >= 24) hours = 0;
      printTime();
    }
  }
  lastHourState = hourState;
  
  // MIN button
  static bool lastMinState = HIGH;
  bool minState = digitalRead(BTN_MIN);
  
  if (minState == LOW && lastMinState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_MIN) == LOW) {
      minutes++;
      if (minutes >= 60) minutes = 0;
      seconds = 0; // Reset seconds when manually adjusting
      printTime();
    }
  }
  lastMinState = minState;
  
  // MODE button
  static bool lastModeState = HIGH;
  bool modeState = digitalRead(BTN_MODE);
  
  if (modeState == LOW && lastModeState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_MODE) == LOW) {
      settingMode = !settingMode;
      Serial.print(settingMode ? "⚙ Setting mode: ON" : "▶ Clock running");
      Serial.println();
    }
  }
  lastModeState = modeState;
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 'h':
    case 'H':
      hours++;
      if (hours >= 24) hours = 0;
      printTime();
      break;
      
    case 'm':
    case 'M':
      minutes++;
      if (minutes >= 60) minutes = 0;
      seconds = 0;
      printTime();
      break;
      
    case 'f':
    case 'F':
      format24Hour = !format24Hour;
      Serial.print("Format: ");
      Serial.println(format24Hour ? "24-hour" : "12-hour");
      break;
      
    case 's':
    case 'S':
      settingMode = !settingMode;
      Serial.print(settingMode ? "⚙ Setting mode: ON" : "▶ Clock running");
      Serial.println();
      break;
      
    case 't':
    case 'T':
      // Set time from serial (format: t HHMM)
      if (Serial.available() >= 4) {
        delay(10); // Wait for data
        int h = (Serial.read() - '0') * 10 + (Serial.read() - '0');
        int m = (Serial.read() - '0') * 10 + (Serial.read() - '0');
        if (h >= 0 && h < 24 && m >= 0 && m < 60) {
          setTime(h, m, 0);
          Serial.println("✓ Time set successfully");
        } else {
          Serial.println("✗ Invalid time format");
        }
      }
      break;
      
    case 'i':
    case 'I':
      showInfo();
      break;
  }
}

void setTime(int h, int m, int s) {
  hours = h;
  minutes = m;
  seconds = s;
  lastSecond = millis();
  printTime();
}

void printTime() {
  Serial.print("🕐 ");
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
  
  if (!format24Hour) {
    Serial.print(hours >= 12 ? " PM" : " AM");
  }
  
  Serial.println();
}

void showInfo() {
  Serial.println("\n========== Clock Info ==========");
  Serial.print("Time: ");
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.println(seconds);
  
  Serial.print("Format: ");
  Serial.println(format24Hour ? "24-hour" : "12-hour");
  
  Serial.print("Setting Mode: ");
  Serial.println(settingMode ? "ON" : "OFF");
  
  Serial.print("Uptime: ");
  unsigned long uptime = millis() / 1000;
  Serial.print(uptime / 3600);
  Serial.print("h ");
  Serial.print((uptime % 3600) / 60);
  Serial.print("m ");
  Serial.print(uptime % 60);
  Serial.println("s");
  
  Serial.println("\nNote: This clock uses millis()");
  Serial.println("For accurate timekeeping, use RTC module (DS3231)");
  Serial.println("================================\n");
}
