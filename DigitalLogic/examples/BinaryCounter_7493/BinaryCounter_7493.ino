/**
 * Binary Counter with 7493
 * 
 * ใช้ IC 7493 (4-bit binary counter) นับเลขแบบ Binary
 * แสดงผลด้วย LED 4 ดวง และ Serial Monitor
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - IC 7493 (4-bit binary counter)
 * - LED 4 ดวง + R 220Ω 4 ตัว
 * - สวิตช์กด 2 ตัว (START/STOP, RESET)
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร 7493:
 * 
 *   ESP32 GPIO 25 ────► 7493 pin 14 (CLK A)
 *   ESP32 GPIO 26 ────► 7493 pin 2,3 (R0(1), R0(2) - tied together for reset)
 *   
 *   7493 pin 12 (QA/Q0) ──► LED0 ──► R ──► GND
 *   7493 pin 9  (QB/Q1) ──► LED1 ──► R ──► GND
 *   7493 pin 8  (QC/Q2) ──► LED2 ──► R ──► GND
 *   7493 pin 11 (QD/Q3) ──► LED3 ──► R ──► GND
 *   
 *   7493 pin 10 ──► GND
 *   7493 pin 5  ──► VCC (3.3V หรือ 5V)
 * 
 * การทำงาน:
 * - Counter นับจาก 0000 ถึง 1111 (0-15 ในเลขฐาน 10)
 * - LED แสดงค่า Binary (LED0=LSB, LED3=MSB)
 * - ปุ่ม START/STOP ควบคุมการนับ
 * - ปุ่ม RESET รีเซ็ตเป็น 0
 */

#include <DigitalLogic.h>

// Pin definitions
const int CLOCK_PIN = 25;
const int RESET_PIN = 26;
const int BTN_START = 4;
const int BTN_RESET = 5;

// LED pins to visualize counter output (optional)
const int LED_Q0 = 19;  // LSB
const int LED_Q1 = 18;
const int LED_Q2 = 17;
const int LED_Q3 = 16;  // MSB

CounterController counter(CLOCK_PIN, RESET_PIN);

bool isRunning = false;
unsigned long lastCount = 0;
int currentCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("==========================================");
  Serial.println("  Binary Counter with 7493 - ESP32");
  Serial.println("==========================================\n");
  
  // Setup button pins
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  
  // Setup LED pins (to mirror 7493 output)
  pinMode(LED_Q0, OUTPUT);
  pinMode(LED_Q1, OUTPUT);
  pinMode(LED_Q2, OUTPUT);
  pinMode(LED_Q3, OUTPUT);
  
  // Initialize counter at 2 Hz
  counter.begin(2);
  
  Serial.println("✓ Counter initialized");
  Serial.println("  Frequency: 2 Hz");
  Serial.println("  Range: 0-15 (4-bit binary)\n");
  
  Serial.println("การใช้งาน:");
  Serial.println("  ปุ่ม GPIO 4  - START/STOP");
  Serial.println("  ปุ่ม GPIO 5  - RESET\n");
  
  Serial.println("คำสั่ง Serial:");
  Serial.println("  s - Start counting");
  Serial.println("  x - Stop counting");
  Serial.println("  r - Reset to 0");
  Serial.println("  p - Single step");
  Serial.println("  1-9 - Set speed (Hz)\n");
  
  // Reset counter to start at 0
  counter.reset();
  delay(100);
  
  Serial.println("Binary | Decimal | LEDs (Q3 Q2 Q1 Q0)");
  Serial.println("-------|---------|-------------------");
}

void loop() {
  // Update counter clock
  counter.update();
  
  // Check buttons
  checkButtons();
  
  // Check Serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
  
  // Update LED display (simulating 7493 output)
  if (isRunning) {
    updateLEDDisplay();
  }
}

void checkButtons() {
  // START/STOP button
  static bool lastStartState = HIGH;
  bool startState = digitalRead(BTN_START);
  
  if (startState == LOW && lastStartState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_START) == LOW) {
      toggleStartStop();
    }
  }
  lastStartState = startState;
  
  // RESET button
  static bool lastResetState = HIGH;
  bool resetState = digitalRead(BTN_RESET);
  
  if (resetState == LOW && lastResetState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_RESET) == LOW) {
      resetCounter();
    }
  }
  lastResetState = resetState;
}

void toggleStartStop() {
  if (isRunning) {
    counter.stop();
    isRunning = false;
    Serial.println("\n■ Counter STOPPED\n");
  } else {
    counter.start();
    isRunning = true;
    Serial.println("\n▶ Counter STARTED\n");
    Serial.println("Binary | Decimal | LEDs (Q3 Q2 Q1 Q0)");
    Serial.println("-------|---------|-------------------");
  }
}

void resetCounter() {
  counter.reset();
  currentCount = 0;
  updateLEDDisplay();
  Serial.println("\n⟲ Counter RESET to 0\n");
  if (isRunning) {
    Serial.println("Binary | Decimal | LEDs (Q3 Q2 Q1 Q0)");
    Serial.println("-------|---------|-------------------");
  }
}

void updateLEDDisplay() {
  // Simulate 7493 output incrementing
  unsigned long currentTime = millis();
  unsigned long freq = counter.getClock()->getFrequency();
  
  if (freq > 0) {
    unsigned long interval = 1000 / freq;
    
    if (currentTime - lastCount >= interval) {
      currentCount = (currentCount + 1) % 16; // 4-bit counter: 0-15
      
      // Update LEDs
      digitalWrite(LED_Q0, (currentCount & 0x01) ? HIGH : LOW);
      digitalWrite(LED_Q1, (currentCount & 0x02) ? HIGH : LOW);
      digitalWrite(LED_Q2, (currentCount & 0x04) ? HIGH : LOW);
      digitalWrite(LED_Q3, (currentCount & 0x08) ? HIGH : LOW);
      
      // Print to Serial
      printBinary(currentCount);
      Serial.print(" |   ");
      Serial.print(currentCount);
      Serial.print("    | ");
      printLEDState(currentCount);
      Serial.println();
      
      lastCount = currentTime;
    }
  }
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 's':
    case 'S':
      if (!isRunning) {
        counter.start();
        isRunning = true;
        Serial.println("\n▶ Counter STARTED\n");
        Serial.println("Binary | Decimal | LEDs (Q3 Q2 Q1 Q0)");
        Serial.println("-------|---------|-------------------");
      }
      break;
      
    case 'x':
    case 'X':
      if (isRunning) {
        counter.stop();
        isRunning = false;
        Serial.println("\n■ Counter STOPPED\n");
      }
      break;
      
    case 'r':
    case 'R':
      resetCounter();
      break;
      
    case 'p':
    case 'P':
      counter.singleStep();
      currentCount = (currentCount + 1) % 16;
      updateLEDDisplay();
      Serial.print("⚡ Single step: ");
      Serial.println(currentCount);
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
      {
        int speed = cmd - '0';
        counter.setFrequency(speed);
        Serial.print("⚙ Speed set to ");
        Serial.print(speed);
        Serial.println(" Hz");
      }
      break;
  }
}

void printBinary(int num) {
  for (int i = 3; i >= 0; i--) {
    Serial.print((num & (1 << i)) ? '1' : '0');
  }
}

void printLEDState(int num) {
  Serial.print((num & 0x08) ? "■" : "□");
  Serial.print(" ");
  Serial.print((num & 0x04) ? "■" : "□");
  Serial.print(" ");
  Serial.print((num & 0x02) ? "■" : "□");
  Serial.print(" ");
  Serial.print((num & 0x01) ? "■" : "□");
}
