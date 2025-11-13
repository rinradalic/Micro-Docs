/**
 * Clock Generator Demo
 * 
 * สร้างสัญญาณ clock (square wave) เพื่อใช้ขับ IC Counter 
 * หรือวงจร sequential logic อื่น ๆ
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - LED + R 220Ω (แสดงสัญญาณ clock)
 * - (ตัวเลือก) IC 7493 หรือ 7473 counter
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร:
 * 
 *   ESP32 GPIO 25 ────► LED ──► R 220Ω ──► GND
 *                  └──► 7493 pin 14 (CLK A)
 * 
 * คุณสมบัติ:
 * - ปรับความถี่ได้ 1-1000 Hz
 * - Start/Stop clock
 * - Single pulse mode
 * - Non-blocking operation
 */

#include <DigitalLogic.h>

const int CLOCK_PIN = 25;

ClockGenerator clock(CLOCK_PIN);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("  Clock Generator Demo - ESP32");
  Serial.println("========================================\n");
  
  // เริ่มต้น clock ที่ 1 Hz
  clock.begin(1);
  
  Serial.println("คำสั่งควบคุม:");
  Serial.println("  s - Start clock");
  Serial.println("  x - Stop clock");
  Serial.println("  p - Single pulse");
  Serial.println("  1-9 - Set frequency (1-9 Hz)");
  Serial.println("  + - Increase frequency");
  Serial.println("  - - Decrease frequency");
  Serial.println("  i - Show info\n");
  
  showClockInfo();
}

void loop() {
  // ต้องเรียก update() ใน loop เพื่อให้ clock ทำงาน
  clock.update();
  
  // ตรวจสอบคำสั่งจาก Serial
  if (Serial.available()) {
    char cmd = Serial.read();
    handleCommand(cmd);
  }
}

void handleCommand(char cmd) {
  switch (cmd) {
    case 's':
    case 'S':
      clock.start();
      Serial.println("▶ Clock STARTED");
      showClockInfo();
      break;
      
    case 'x':
    case 'X':
      clock.stop();
      Serial.println("■ Clock STOPPED");
      showClockInfo();
      break;
      
    case 'p':
    case 'P':
      clock.pulse();
      Serial.println("⚡ Single pulse sent");
      break;
      
    case '1':
      clock.setFrequency(1);
      showClockInfo();
      break;
      
    case '2':
      clock.setFrequency(2);
      showClockInfo();
      break;
      
    case '3':
      clock.setFrequency(3);
      showClockInfo();
      break;
      
    case '4':
      clock.setFrequency(4);
      showClockInfo();
      break;
      
    case '5':
      clock.setFrequency(5);
      showClockInfo();
      break;
      
    case '6':
      clock.setFrequency(6);
      showClockInfo();
      break;
      
    case '7':
      clock.setFrequency(7);
      showClockInfo();
      break;
      
    case '8':
      clock.setFrequency(8);
      showClockInfo();
      break;
      
    case '9':
      clock.setFrequency(9);
      showClockInfo();
      break;
      
    case '+':
      {
        unsigned long newFreq = clock.getFrequency() + 1;
        if (newFreq > 1000) newFreq = 1000;
        clock.setFrequency(newFreq);
        showClockInfo();
      }
      break;
      
    case '-':
      {
        unsigned long newFreq = clock.getFrequency();
        if (newFreq > 1) newFreq--;
        clock.setFrequency(newFreq);
        showClockInfo();
      }
      break;
      
    case 'i':
    case 'I':
      showClockInfo();
      break;
      
    default:
      // Ignore other characters
      break;
  }
}

void showClockInfo() {
  Serial.print("ℹ Clock: ");
  Serial.print(clock.isRunning() ? "RUNNING" : "STOPPED");
  Serial.print(" | Frequency: ");
  Serial.print(clock.getFrequency());
  Serial.print(" Hz");
  
  if (clock.getFrequency() > 0) {
    float period = 1000.0 / clock.getFrequency();
    Serial.print(" (Period: ");
    Serial.print(period, 2);
    Serial.print(" ms)");
  }
  
  Serial.println();
}
