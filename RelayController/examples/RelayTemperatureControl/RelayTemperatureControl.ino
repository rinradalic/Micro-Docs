/*
 * RelayTemperatureControl
 * 
 * Automatic relay control based on temperature sensor.
 * Simulates DHT11 temperature.
 * 
 * Behavior:
 * - Temperature > 30°C → Relay ON (turn on fan/cooler)
 * - Temperature ≤ 30°C → Relay OFF
 * 
 * Hardware:
 * - (ESP32) or Arduino
 * - Relay module on pin 26
 * - DHT11/DHT22 sensor on pin 4 (requires DHT library)
 * - Load: 5V/6V fan or indicator LED
 * 
 * Installation:
 * 1. Install DHT sensor library (DHT sensor library by Adafruit)
 * 2. Install Adafruit Unified Sensor library
 * 3. Connect DHT11: VCC → 3.3V, GND → GND, DATA → GPIO4
 * 
 * Note: This example requires DHT sensor library.
 *       Uncomment the DHT code below after installing the library.
 */

#include <RelayController.h>
// Uncomment these lines after installing DHT library:
// #include <DHT.h>
// #define DHTPIN 4
// #define DHTTYPE DHT11
// DHT dht(DHTPIN, DHTTYPE);

const int RELAY_PIN = 26;
const float TEMP_THRESHOLD = 30.0; // Temperature threshold in Celsius

RelayController relay(RELAY_PIN);

void setup() {
  Serial.begin(115200);
  relay.begin();
  // Uncomment after installing DHT library:
  // dht.begin();
  
  Serial.println("Relay Temperature Control Example");
  Serial.println("Temperature threshold: 30°C");
  Serial.println("Relay ON when temp > 30°C");
  Serial.println();
  
  // Simulated mode message
  Serial.println("SIMULATION MODE (no DHT sensor)");
  Serial.println("Random temperature values generated");
  Serial.println("Install DHT library and uncomment code for real sensor");
  Serial.println();
}

void loop() {
  // SIMULATION MODE: Generate random temperature
  // Replace this with real DHT reading after installing library
  float temp = random(250, 350) / 10.0; // Random temp 25.0 - 35.0°C
  
  // REAL DHT11 CODE (uncomment after installing library):
  // float temp = dht.readTemperature();
  // if (isnan(temp)) {
  //   Serial.println("Failed to read from DHT sensor!");
  //   delay(2000);
  //   return;
  // }
  
  // Control relay based on temperature
  if (temp > TEMP_THRESHOLD) {
    if (!relay.isOn()) {
      relay.on();
      Serial.println("🔥 Temperature HIGH - Relay ON (Fan/Cooler activated)");
    }
  } else {
    if (relay.isOn()) {
      relay.off();
      Serial.println("❄️  Temperature OK - Relay OFF");
    }
  }
  
  // Display current status
  Serial.print("Temp: ");
  Serial.print(temp, 1);
  Serial.print("°C | Relay: ");
  Serial.println(relay.isOn() ? "ON" : "OFF");
  
  delay(2000); // Check every 2 seconds
}
