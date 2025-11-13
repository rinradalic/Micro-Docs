/*
 * TempLevel_Indicator
 * 
 * Multi-level temperature status indicator with 3 LEDs.
 * Shows COOL/WARM/HOT status based on temperature ranges.
 * 
 * Hardware (ESP32):
 * - DHT11 → GPIO 4
 * - LED Green (COOL) → GPIO 2
 * - LED Yellow (WARM) → GPIO 15
 * - LED Red (HOT) → GPIO 16
 * (Each LED via 220Ω resistor to GND)
 * 
 * Temperature Levels:
 * - COOL: < 28°C → Green LED
 * - WARM: 28-32°C → Yellow LED
 * - HOT: > 32°C → Red LED
 */

#include <TemperatureSensor.h>
#include <LEDController.h>

const int DHT_PIN = 4;
const int LED_GREEN_PIN = 2;   // COOL
const int LED_YELLOW_PIN = 15; // WARM
const int LED_RED_PIN = 16;    // HOT

const float COOL_MAX = 28.0;   // Below this = COOL
const float WARM_MAX = 32.0;   // Below this = WARM, above = HOT

TemperatureSensor sensor(DHT_PIN, DHT11);
LEDController ledGreen(LED_GREEN_PIN);
LEDController ledYellow(LED_YELLOW_PIN);
LEDController ledRed(LED_RED_PIN);

void setup() {
  Serial.begin(115200);
  
  sensor.begin();
  ledGreen.begin();
  ledYellow.begin();
  ledRed.begin();
  
  Serial.println("=== Temperature Level Indicator ===");
  Serial.println("Temperature ranges:");
  Serial.print("  COOL (Green):  < ");
  Serial.print(COOL_MAX);
  Serial.println(" °C");
  Serial.print("  WARM (Yellow): ");
  Serial.print(COOL_MAX);
  Serial.print(" - ");
  Serial.print(WARM_MAX);
  Serial.println(" °C");
  Serial.print("  HOT (Red):     > ");
  Serial.print(WARM_MAX);
  Serial.println(" °C");
  Serial.println();
}

void updateLEDs(TemperatureSensor::TempLevel level) {
  // Turn off all LEDs first
  ledGreen.off();
  ledYellow.off();
  ledRed.off();
  
  // Turn on appropriate LED
  switch (level) {
    case TemperatureSensor::TEMP_COOL:
      ledGreen.on();
      break;
    case TemperatureSensor::TEMP_WARM:
      ledYellow.on();
      break;
    case TemperatureSensor::TEMP_HOT:
      ledRed.on();
      break;
  }
}

void loop() {
  if (sensor.read()) {
    float temp = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    TemperatureSensor::TempLevel level = sensor.getTemperatureLevel(COOL_MAX, WARM_MAX);
    const char* levelStr = sensor.getTemperatureLevelString(COOL_MAX, WARM_MAX);
    
    // Update LED indicators
    updateLEDs(level);
    
    // Display status with emoji
    Serial.print("Status: ");
    if (level == TemperatureSensor::TEMP_COOL) {
      Serial.print("❄️  ");
    } else if (level == TemperatureSensor::TEMP_WARM) {
      Serial.print("🌤️  ");
    } else {
      Serial.print("🔥 ");
    }
    Serial.print(levelStr);
    Serial.print("  |  Temp: ");
    Serial.print(temp, 1);
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
  } else {
    Serial.println("❌ Sensor read error!");
    
    // Blink all LEDs on error
    ledGreen.toggle();
    ledYellow.toggle();
    ledRed.toggle();
  }
  
  delay(1000);
}
