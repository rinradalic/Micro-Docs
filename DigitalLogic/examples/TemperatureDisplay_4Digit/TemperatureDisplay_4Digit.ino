/**
 * Temperature Display with DHT11/DHT22 on 4-Digit 7-Segment
 * 
 * แสดงอุณหภูมิและความชื้นจากเซนเซอร์ DHT11/DHT22 
 * บนจอ 7-segment 4 หลักแบบสลับกัน
 * 
 * อุปกรณ์:
 * - (ESP32)
 * - DHT11 หรือ DHT22 temperature sensor
 * - 7-segment display 4 หลัก แบบ Common Cathode
 * - ทรานซิสเตอร์ NPN 4 ตัว
 * - ตัวต้านทาน 220Ω x 7, 10kΩ x 5 (4 สำหรับทรานซิสเตอร์ + 1 สำหรับ DHT)
 * - Breadboard และสายจั๊มเปอร์
 * 
 * การต่อวงจร:
 * 
 * DHT11/DHT22:
 *   VCC ──► 3.3V
 *   DATA ─► GPIO 15 (ผ่าน R 10kΩ pull-up to VCC)
 *   GND ──► GND
 * 
 * 7-Segment Display:
 *   Segments: GPIO 14,27,26,25,33,32,23 (a-g)
 *   Digits: GPIO 22,21,19,18 (D1-D4)
 * 
 * Button (สลับโหมด):
 *   GPIO 4 ──► SW MODE ──► GND
 * 
 * การแสดงผล:
 * - โหมด 1: อุณหภูมิ (เช่น 27.5°C แสดงเป็น "27.5")
 * - โหมด 2: ความชื้น (เช่น 65% แสดงเป็น "65.0")
 * - โหมด 3: สลับทั้งสอง (temp 3 วินาที, humidity 3 วินาที)
 * 
 * ต้องติดตั้ง Library:
 * - DHT sensor library by Adafruit
 * - Adafruit Unified Sensor
 */

#include <DigitalLogic.h>
#include <DHT.h>

// DHT Sensor settings
#define DHTPIN 15
#define DHTTYPE DHT11  // เปลี่ยนเป็น DHT22 ถ้าใช้ DHT22

DHT dht(DHTPIN, DHTTYPE);

// Pin definitions - Segments
const int SEG_A = 14;
const int SEG_B = 27;
const int SEG_C = 26;
const int SEG_D = 25;
const int SEG_E = 33;
const int SEG_F = 32;
const int SEG_G = 23;

// Pin definitions - Digit Control
const int DIGIT_1 = 22;
const int DIGIT_2 = 21;
const int DIGIT_3 = 19;
const int DIGIT_4 = 18;

// Button
const int BTN_MODE = 4;

// Create display object
SevenSegmentMultiplex display(
  SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G,
  DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4,
  true
);

// Display modes
enum DisplayMode {
  MODE_TEMP,      // Show temperature only
  MODE_HUMIDITY,  // Show humidity only
  MODE_ALTERNATE  // Alternate between temp and humidity
};

DisplayMode currentMode = MODE_ALTERNATE;
const char* modeNames[] = {"Temperature", "Humidity", "Alternate"};

// Sensor reading
float temperature = 0.0;
float humidity = 0.0;
bool sensorOK = false;

// Timing
unsigned long lastRead = 0;
unsigned long lastSwitch = 0;
const unsigned long readInterval = 2000;   // Read sensor every 2 seconds
const unsigned long switchInterval = 3000; // Switch display every 3 seconds

bool showingTemp = true; // For alternate mode

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("================================================");
  Serial.println("  Temperature Display - DHT11/DHT22 + 7-Segment");
  Serial.println("================================================\n");
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize display
  display.begin();
  display.setLeadingZeros(false);
  
  // Initialize button
  pinMode(BTN_MODE, INPUT_PULLUP);
  
  Serial.println("✓ DHT sensor initialized");
  Serial.println("✓ 4-digit display initialized\n");
  
  Serial.println("Display Modes:");
  Serial.println("  1. Temperature only (XX.X °C)");
  Serial.println("  2. Humidity only (XX.X %)");
  Serial.println("  3. Alternate (switch every 3s)\n");
  
  Serial.println("Controls:");
  Serial.println("  Button MODE (GPIO 4) - Switch display mode\n");
  
  Serial.println("Serial Commands:");
  Serial.println("  m - Change mode");
  Serial.println("  r - Read sensor now");
  Serial.println("  i - Show info\n");
  
  Serial.println("Reading sensor...\n");
  
  // Initial reading
  readSensor();
}

void loop() {
  // Update display scanning (CRITICAL!)
  display.update();
  
  // Check if it's time to read sensor
  unsigned long now = millis();
  if (now - lastRead >= readInterval) {
    lastRead = now;
    readSensor();
  }
  
  // Handle alternate mode switching
  if (currentMode == MODE_ALTERNATE) {
    if (now - lastSwitch >= switchInterval) {
      lastSwitch = now;
      showingTemp = !showingTemp;
      updateDisplay();
    }
  }
  
  // Check button
  checkButton();
  
  // Check serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
}

void readSensor() {
  // Read from DHT sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Check if reading failed
  if (isnan(h) || isnan(t)) {
    Serial.println("✗ Failed to read from DHT sensor!");
    sensorOK = false;
    display.showNumber(8888); // Error pattern
    return;
  }
  
  sensorOK = true;
  temperature = t;
  humidity = h;
  
  // Print to serial
  Serial.print("🌡 Temperature: ");
  Serial.print(temperature, 1);
  Serial.print("°C  |  💧 Humidity: ");
  Serial.print(humidity, 1);
  Serial.println("%");
  
  // Update display
  updateDisplay();
}

void updateDisplay() {
  if (!sensorOK) {
    display.showNumber(8888); // Error
    return;
  }
  
  float valueToShow = 0.0;
  
  switch (currentMode) {
    case MODE_TEMP:
      valueToShow = temperature;
      break;
      
    case MODE_HUMIDITY:
      valueToShow = humidity;
      break;
      
    case MODE_ALTERNATE:
      valueToShow = showingTemp ? temperature : humidity;
      break;
  }
  
  // Show value with 1 decimal place
  display.showTemperature(valueToShow, 1);
}

void checkButton() {
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BTN_MODE);
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50); // Debounce
    if (digitalRead(BTN_MODE) == LOW) {
      changeMode();
    }
  }
  lastButtonState = buttonState;
}

void changeMode() {
  currentMode = (DisplayMode)((currentMode + 1) % 3);
  lastSwitch = millis(); // Reset switch timer
  showingTemp = true;    // Reset to temp for alternate mode
  
  Serial.print("📺 Mode: ");
  Serial.println(modeNames[currentMode]);
  
  updateDisplay();
}

void handleSerialCommand() {
  char cmd = Serial.read();
  
  switch (cmd) {
    case 'm':
    case 'M':
      changeMode();
      break;
      
    case 'r':
    case 'R':
      Serial.println("Reading sensor...");
      readSensor();
      break;
      
    case 'i':
    case 'I':
      showInfo();
      break;
      
    case 'c':
    case 'C':
      // Toggle Celsius/Fahrenheit
      {
        static bool useFahrenheit = false;
        useFahrenheit = !useFahrenheit;
        if (useFahrenheit) {
          temperature = temperature * 9.0 / 5.0 + 32.0;
          Serial.println("Unit: Fahrenheit (°F)");
        } else {
          Serial.println("Unit: Celsius (°C)");
        }
        updateDisplay();
      }
      break;
  }
}

void showInfo() {
  Serial.println("\n========== System Info ==========");
  
  Serial.print("Sensor: DHT");
  Serial.println(DHTTYPE == DHT11 ? "11" : "22");
  
  Serial.print("Status: ");
  Serial.println(sensorOK ? "✓ OK" : "✗ ERROR");
  
  if (sensorOK) {
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" °C");
    
    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
    
    // Calculate heat index
    float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
    Serial.print("Heat Index: ");
    Serial.print(heatIndex, 1);
    Serial.println(" °C");
  }
  
  Serial.print("\nDisplay Mode: ");
  Serial.println(modeNames[currentMode]);
  
  if (currentMode == MODE_ALTERNATE) {
    Serial.print("Currently showing: ");
    Serial.println(showingTemp ? "Temperature" : "Humidity");
  }
  
  Serial.print("\nRead Interval: ");
  Serial.print(readInterval / 1000);
  Serial.println(" seconds");
  
  if (currentMode == MODE_ALTERNATE) {
    Serial.print("Switch Interval: ");
    Serial.print(switchInterval / 1000);
    Serial.println(" seconds");
  }
  
  Serial.println("\nMultiplexing:");
  Serial.println("  Scan rate: ~250 Hz");
  Serial.println("  Smooth non-blocking display");
  
  Serial.println("=================================\n");
}
