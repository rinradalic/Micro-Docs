/*
 * SmartFan_ManualOverride
 * 
 * Smart fan control with AUTO/MANUAL mode toggle.
 * - AUTO mode: Fan controlled by temperature (hysteresis)
 * - MANUAL mode: Fan always ON (override temperature control)
 * 
 * Press button to switch between modes.
 * 
 * Hardware (ESP32):
 * - DHT11 → GPIO 4
 * - Relay → GPIO 26 (transistor driver circuit)
 * - Button → GPIO 5 (with internal pullup)
 * - LED → GPIO 2 (status indicator)
 * 
 * Mode Indicator:
 * - LED solid ON = MANUAL mode (fan forced ON)
 * - LED blinks slow = AUTO mode, fan ON
 * - LED OFF = AUTO mode, fan OFF
 */

#include <TemperatureSensor.h>
#include <RelayController.h>
#include <LEDController.h>
#include <Button.h>

const int DHT_PIN = 4;
const int RELAY_PIN = 26;
const int LED_PIN = 2;
const int BUTTON_PIN = 5;

const float TEMP_ON = 30.0;
const float TEMP_OFF = 28.0;

enum ControlMode {
  MODE_AUTO = 0,
  MODE_MANUAL = 1
};

TemperatureSensor sensor(DHT_PIN, DHT11);
RelayController fan(RELAY_PIN);
LEDController led(LED_PIN);
Button button(BUTTON_PIN, true, 80);

ControlMode currentMode = MODE_AUTO;
bool fanRunning = false;

void setup() {
  Serial.begin(115200);
  
  sensor.begin();
  sensor.setThresholds(TEMP_ON, TEMP_OFF);
  
  fan.begin();
  led.begin();
  button.begin();
  
  Serial.println("=== Smart Fan with Manual Override ===");
  Serial.println("Press button to toggle mode:");
  Serial.println("  AUTO = Temperature controlled");
  Serial.println("  MANUAL = Always ON");
  Serial.println();
}

void loop() {
  button.update();
  led.update();
  
  // Check for mode switch
  if (button.wasPressed()) {
    currentMode = (currentMode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
    
    Serial.print("Mode changed to: ");
    Serial.println(currentMode == MODE_AUTO ? "AUTO" : "MANUAL");
    
    if (currentMode == MODE_MANUAL) {
      // MANUAL mode: force fan ON
      fanRunning = true;
      fan.on();
      led.on(); // Solid ON in manual mode
    }
  }
  
  // Read sensor
  if (sensor.read()) {
    float temp = sensor.getTemperature();
    float humidity = sensor.getHumidity();
    
    if (currentMode == MODE_AUTO) {
      // AUTO mode: temperature-based control with hysteresis
      if (!fanRunning && sensor.isAboveHighThreshold()) {
        fanRunning = true;
        fan.on();
        led.startBlink(1000); // Slow blink in auto mode
        Serial.println("🔥 AUTO: Temperature HIGH → Fan ON");
      } 
      else if (fanRunning && sensor.isBelowLowThreshold()) {
        fanRunning = false;
        fan.off();
        led.off();
        Serial.println("❄️  AUTO: Temperature OK → Fan OFF");
      }
    }
    // MANUAL mode: no action needed (fan stays ON)
    
    // Display status
    Serial.print("Mode: ");
    Serial.print(currentMode == MODE_AUTO ? "AUTO" : "MANUAL");
    Serial.print("  |  Temp: ");
    Serial.print(temp, 1);
    Serial.print(" °C  |  Humidity: ");
    Serial.print(humidity, 1);
    Serial.print(" %  |  Fan: ");
    Serial.println(fanRunning ? "ON" : "OFF");
  } else {
    Serial.println("❌ Sensor read error!");
  }
  
  delay(1000);
}
