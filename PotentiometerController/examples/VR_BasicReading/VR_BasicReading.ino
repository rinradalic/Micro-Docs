/*
 * VR_BasicReading
 * 
 * Basic potentiometer (VR) reading example.
 * Reads analog value and displays raw ADC value and percentage.
 * 
 * Hardware (ESP32/ICON-32):
 * - VR Pin 1 → 3.3V
 * - VR Pin 2 (Center/Wiper) → GPIO 34 (ADC pin)
 * - VR Pin 3 → GND
 * 
 * Behavior:
 * - Rotate VR clockwise → values increase
 * - Rotate VR counter-clockwise → values decrease
 * - Serial Monitor shows both raw (0-4095) and percentage (0-100%)
 * 
 * Learning objectives (EP4):
 * - Understand voltage divider concept
 * - Learn to use analogRead() on ESP32
 * - Learn to use map() function
 */

#include <PotentiometerController.h>

const int VR_PIN = 34; // ADC pin on ESP32

PotentiometerController vr(VR_PIN);

void setup() {
  Serial.begin(115200);
  vr.begin();
  
  Serial.println("=== VR Basic Reading Example ===");
  Serial.println("Rotate the potentiometer and observe values");
  Serial.println();
}

void loop() {
  int rawValue = vr.read();
  int percent = vr.readPercent();
  
  Serial.print("RAW = ");
  Serial.print(rawValue);
  Serial.print(" (0-");
  Serial.print(vr.getMaxRaw());
  Serial.print(")");
  Serial.print("  |  PERCENT = ");
  Serial.print(percent);
  Serial.println(" %");
  
  delay(200);
}
