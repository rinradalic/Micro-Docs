/**
 * Smart Counter System
 * 
 * โปรเจคใหญ่ ระบบนับเลขอัจฉริยะ
 * รวม ESP32 + 7493 Counter + CD4511 Decoder + 7-Segment Display
 * 
 * คุณสมบัติ:
 * - นับเลข 0-9 แสดงบน 7-segment display
 * - ปุ่ม COUNT: นับเพิ่มทีละ 1
 * - ปุ่ม RESET: รีเซ็ตกลับไปเป็น 0
 * - โหมดอัตโนมัติ: นับเลขอัตโนมัติ (ปรับความเร็วได้)
 * - แสดงผลทั้ง 7-segment และ Serial Monitor
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - IC 7493 (4-bit binary counter)
 * - IC CD4511 (BCD to 7-segment decoder)
 * - 7-segment display (Common Cathode)
 * - สวิตช์กด 2 ตัว
 * - ตัวต้านทาน 220Ω จำนวน 7 ตัว
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจรแบบเต็ม:
 * 
 * ESP32 ──► 7493 ──► CD4511 ──► 7-Segment
 * 
 * รายละเอียด:
 * 
 * 1) ESP32 to 7493:
 *    GPIO 25 ────► 7493 pin 14 (CLK A)
 *    GPIO 26 ────► 7493 pin 2,3 (Reset - tied together)
 * 
 * 2) 7493 to CD4511 (BCD connection):
 *    7493 pin 12 (Q0) ──► CD4511 pin 7  (A)
 *    7493 pin 9  (Q1) ──► CD4511 pin 1  (B)
 *    7493 pin 8  (Q2) ──► CD4511 pin 2  (C)
 *    7493 pin 11 (Q3) ──► CD4511 pin 6  (D)
 * 
 * 3) CD4511 to 7-Segment:
 *    CD4511 segments (a-g) ──► ตัวต้านทาน ──► 7-segment
 *    (ดูรายละเอียดการต่อใน SevenSegment_CD4511 example)
 * 
 * 4) Buttons:
 *    GPIO 4 ──► SW1 (COUNT) ──► GND
 *    GPIO 5 ──► SW2 (RESET) ──► GND
 *    (ใช้ INPUT_PULLUP ไม่ต้องต่อตัวต้านทาน pull-up ภายนอก)
 * 
 * 5) Power:
 *    7493 pin 5, CD4511 pin 16 ──► VCC (3.3V หรือ 5V)
 *    7493 pin 10, CD4511 pin 8 ──► GND
 * 
 * การทำงาน:
 * - กด COUNT → ESP32 ส่ง pulse → 7493 นับเพิ่ม → CD4511 แปลง BCD → 7-seg แสดงเลข
 * - กด RESET → ESP32 รีเซ็ต 7493 → กลับไปเป็น 0
 * - โหมด AUTO → นับอัตโนมัติตามความเร็วที่กำหนด
 * 
 * นี่คือ "ระบบดิจิทัลผสมไมโครคอนโทรลเลอร์" แบบสมบูรณ์!
 */

#include <DigitalLogic.h>

// Pin definitions
const int CLOCK_PIN = 25;      // Clock output to 7493
const int RESET_PIN = 26;      // Reset output to 7493
const int BTN_COUNT = 4;       // COUNT button (active LOW)
const int BTN_RESET = 5;       // RESET button (active LOW)

// Counter controller
CounterController counter(CLOCK_PIN, RESET_PIN);

// Mode control
enum CountMode {
  MODE_MANUAL,    // กดปุ่มนับทีละครั้ง
  MODE_AUTO       // นับอัตโนมัติ
};

CountMode currentMode = MODE_MANUAL;
int displayValue = 0;
bool autoRunning = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("╔═══════════════════════════════════════════╗");
  Serial.println("║   SMART COUNTER SYSTEM                    ║");
  Serial.println("║   ESP32 + 7493 + CD4511 + 7-Segment       ║");
  Serial.println("╚═══════════════════════════════════════════╝\n");
  
  // Setup buttons
  pinMode(BTN_COUNT, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  // Initialize counter (1 Hz default for auto mode)
  counter.begin(1);
  
  Serial.println("✓ System initialized");
  Serial.println("  Clock: GPIO 25");
  Serial.println("  Reset: GPIO 26");
  Serial.println("  Button COUNT: GPIO 4");
  Serial.println("  Button RESET: GPIO 5\n");
  
  Serial.println("📌 Current Mode: MANUAL");
  Serial.println("   Press COUNT button to increment");
  Serial.println("   Press RESET button to reset to 0\n");
  
  Serial.println("Serial Commands:");
  Serial.println("  m - Manual mode");
  Serial.println("  a - Auto mode");
  Serial.println("  s - Start auto counting");
  Serial.println("  x - Stop auto counting");
  Serial.println("  r - Reset counter");
  Serial.println("  1-9 - Set auto speed (Hz)");
  Serial.println("  i - Show info\n");
  
  // Reset counter to 0
  counter.reset();
  delay(100);
  displayValue = 0;
  
  printDisplay(displayValue);
}

void loop() {
  // Update counter clock (for auto mode)
  if (currentMode == MODE_AUTO && autoRunning) {
    counter.update();
    updateAutoDisplay();
  }
  
  // Check physical buttons
  checkButtons();
  
  // Check serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
}

void checkButtons() {
  // COUNT button
  static bool lastCountState = HIGH;
  bool countState = digitalRead(BTN_COUNT);
  
  if (countState == LOW && lastCountState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_COUNT) == LOW) {
      handleCountButton();
    }
  }
  lastCountState = countState;
  
  // RESET button
  static bool lastResetState = HIGH;
  bool resetState = digitalRead(BTN_RESET);
  
  if (resetState == LOW && lastResetState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_RESET) == LOW) {
      handleResetButton();
    }
  }
  lastResetState = resetState;
}

void handleCountButton() {
  if (currentMode == MODE_MANUAL) {
    // Manual count: send single pulse
    counter.singleStep();
    delay(20); // Short delay for 7493 to update
    
    displayValue = (displayValue + 1) % 10; // 0-9 only
    printDisplay(displayValue);
  } else if (currentMode == MODE_AUTO) {
    // In auto mode, COUNT button toggles start/stop
    if (autoRunning) {
      counter.stop();
      autoRunning = false;
      Serial.println("■ Auto counting STOPPED");
    } else {
      counter.start();
      autoRunning = true;
      Serial.println("▶ Auto counting STARTED");
    }
  }
}

void handleResetButton() {
  counter.reset();
  displayValue = 0;
  Serial.println("⟲ Counter RESET to 0");
  printDisplay(displayValue);
  
  if (currentMode == MODE_AUTO && autoRunning) {
    // Restart auto counting after reset
    counter.start();
  }
}

void updateAutoDisplay() {
  static unsigned long lastUpdate = 0;
  unsigned long freq = counter.getClock()->getFrequency();
  unsigned long interval = 1000 / freq;
  
  if (millis() - lastUpdate >= interval) {
    displayValue = (displayValue + 1) % 10;
    printDisplay(displayValue);
    lastUpdate = millis();
  }
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 'm':
    case 'M':
      setManualMode();
      break;
      
    case 'a':
    case 'A':
      setAutoMode();
      break;
      
    case 's':
    case 'S':
      if (currentMode == MODE_AUTO) {
        counter.start();
        autoRunning = true;
        Serial.println("▶ Auto counting STARTED");
      } else {
        Serial.println("⚠ Switch to AUTO mode first (press 'a')");
      }
      break;
      
    case 'x':
    case 'X':
      if (currentMode == MODE_AUTO) {
        counter.stop();
        autoRunning = false;
        Serial.println("■ Auto counting STOPPED");
      }
      break;
      
    case 'r':
    case 'R':
      handleResetButton();
      break;
      
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (currentMode == MODE_AUTO) {
        int speed = cmd - '0';
        counter.setFrequency(speed);
        Serial.print("⚙ Auto speed set to ");
        Serial.print(speed);
        Serial.println(" Hz");
      } else {
        Serial.println("⚠ Speed control only available in AUTO mode");
      }
      break;
      
    case 'i':
    case 'I':
      showInfo();
      break;
  }
}

void setManualMode() {
  if (currentMode != MODE_MANUAL) {
    counter.stop();
    autoRunning = false;
    currentMode = MODE_MANUAL;
    Serial.println("\n📌 Mode: MANUAL");
    Serial.println("   Press COUNT button to increment manually\n");
  }
}

void setAutoMode() {
  if (currentMode != MODE_AUTO) {
    currentMode = MODE_AUTO;
    Serial.println("\n📌 Mode: AUTO");
    Serial.println("   Press 's' to start, 'x' to stop");
    Serial.println("   Press 1-9 to adjust speed\n");
  }
}

void printDisplay(int value) {
  Serial.print("╔═════╗\n║  ");
  Serial.print(value);
  Serial.println("  ║\n╚═════╝");
}

void showInfo() {
  Serial.println("\n========== System Info ==========");
  Serial.print("Mode: ");
  Serial.println(currentMode == MODE_MANUAL ? "MANUAL" : "AUTO");
  
  if (currentMode == MODE_AUTO) {
    Serial.print("Auto Status: ");
    Serial.println(autoRunning ? "RUNNING" : "STOPPED");
    Serial.print("Speed: ");
    Serial.print(counter.getClock()->getFrequency());
    Serial.println(" Hz");
  }
  
  Serial.print("Current Value: ");
  Serial.println(displayValue);
  Serial.println("=================================\n");
}
