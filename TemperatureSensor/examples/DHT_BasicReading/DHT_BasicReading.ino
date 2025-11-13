/*
 * DHT_BasicReading
 * 
 * Basic temperature and humidity reading from DHT11 sensor.
 * Displays values every second in Serial Monitor.
 * 
 * Hardware (ESP32/ICON-32):
 * - DHT11 VCC → 3.3V (or 5V)
 * - DHT11 GND → GND
 * - DHT11 DATA → GPIO 4
 * 
 * Circuit:
 *   ICON-32          DHT11
 *   ┌─────────┐      ┌────┐
 *   │ 3.3V ───┼──────┤ VCC│
 *   │         │      │    │
 *   │ GPIO4 ──┼──────┤DATA│
 *   │         │      │    │
 *   │ GND ────┼──────┤ GND│
 *   └─────────┘      └────┘
 * 
 * Required Libraries:
 * - DHT sensor library by Adafruit
 * - Adafruit Unified Sensor
 * 
 * Install via: Tools → Manage Libraries → Search "DHT"
 * 
 * Learning objectives (EP5):
 * - Understand DHT11 sensor operation
 * - Read temperature and humidity
 * - Handle sensor errors
 */

#include <TemperatureSensor.h>

const int DHT_PIN = 4; // GPIO 4

TemperatureSensor sensor(DHT_PIN, DHT11);

void setup() {
  Serial.begin(115200);
  sensor.begin();
  
  Serial.println("=== DHT11 Basic Reading Example ===");
  Serial.println("Reading temperature and humidity...");
  Serial.println();
}

void loop() {
  if (sensor.read()) {
    float temp = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    Serial.print("Temperature: ");
    Serial.print(temp, 1);
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
  } else {
    Serial.println("❌ Failed to read from DHT11 sensor!");
  }
  
  delay(1000);
}
