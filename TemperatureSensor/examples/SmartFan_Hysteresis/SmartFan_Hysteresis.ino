/*
 * SmartFan_Hysteresis
 * 
 * Automatic fan control based on temperature with hysteresis.
 * Uses DHT11 + RelayController to control a fan via transistor driver circuit.
 * 
 * Hardware (ESP32/ICON-32):
 * - DHT11 → GPIO 4 (DATA pin)
 * - Relay control → GPIO 26 (via transistor driver - see EP3)
 * - LED indicator → GPIO 2 (shows fan status)
 * - 6V fan connected via relay (NO contact)
 * 
 * Transistor Driver Circuit (from EP3):
 *   GPIO26 ──[1kΩ]──► Base (NPN transistor)
 *                      Collector → Relay Coil (-)
 *                      Emitter → GND
 *   Relay Coil (+) → +6V
 *   Diode 1N4007 across relay coil (flyback protection)
 * 
 * Fan Connection:
 *   +6V → COM (relay)
 *   NO (relay) → Fan (+)
 *   Fan (-) → GND
 * 
 * Hysteresis Logic:
 * - Fan turns ON when temp >= 30°C
 * - Fan turns OFF when temp <= 28°C
 * - This prevents rapid on/off cycling
 * 
 * Learning objectives (EP5):
 * - Combine DHT11 with relay control
 * - Understand hysteresis concept
 * - Build automatic control system
 */

#include <TemperatureSensor.h>
#include <RelayController.h>
#include <LEDController.h>

const int DHT_PIN = 4;
const int RELAY_PIN = 26;
const int LED_PIN = 2;

// Temperature thresholds (Hysteresis)
const float TEMP_ON = 30.0;   // Turn fan ON at this temp
const float TEMP_OFF = 28.0;  // Turn fan OFF at this temp

TemperatureSensor sensor(DHT_PIN, DHT11);
RelayController fan(RELAY_PIN);
LEDController led(LED_PIN);

bool fanRunning = false;

void setup() {
  Serial.begin(115200);
  
  sensor.begin();
  sensor.setThresholds(TEMP_ON, TEMP_OFF);
  
  fan.begin();
  led.begin();
  
  Serial.println("=== Smart Fan Control with Hysteresis ===");
  Serial.println("Temperature thresholds:");
  Serial.print("  Fan ON:  >= ");
  Serial.print(TEMP_ON);
  Serial.println(" °C");
  Serial.print("  Fan OFF: <= ");
  Serial.print(TEMP_OFF);
  Serial.println(" °C");
  Serial.println();
}

void loop() {
  if (sensor.read()) {
    float temp = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    // Hysteresis control logic
    if (!fanRunning && sensor.isAboveHighThreshold()) {
      // Temperature too high → turn fan ON
      fanRunning = true;
      fan.on();
      led.on();
      Serial.println("🔥 Temperature HIGH → Fan ON");
    } 
    else if (fanRunning && sensor.isBelowLowThreshold()) {
      // Temperature cooled down → turn fan OFF
      fanRunning = false;
      fan.off();
      led.off();
      Serial.println("❄️  Temperature OK → Fan OFF");
    }
    
    // Display current status
    Serial.print("Temp: ");
    Serial.print(temp, 1);
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity, 1);
    Serial.print(" %  |  Fan: ");
    Serial.println(fanRunning ? "ON" : "OFF");
  } else {
    Serial.println("❌ Failed to read sensor!");
  }
  
  delay(1000);
}
