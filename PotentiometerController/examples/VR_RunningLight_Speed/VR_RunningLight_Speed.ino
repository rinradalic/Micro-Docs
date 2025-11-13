/*
 * VR_RunningLight_Speed
 * 
 * Control running light speed using VR (potentiometer).
 * Combines EP2 (running lights) with EP4 (analog control).
 * 
 * Hardware (ESP32/ICON-32):
 * - VR → GPIO 34 (ADC)
 * - LED1 → GPIO 2
 * - LED2 → GPIO 4
 * - LED3 → GPIO 5
 * - LED4 → GPIO 18
 * (Each LED via 220Ω resistor to GND)
 * 
 * Behavior:
 * - VR low → slow running light
 * - VR high → fast running light
 * - Delay ranges from 50ms to 1000ms
 * 
 * Learning objectives (EP4):
 * - Use VR to control timing
 * - Combine multiple concepts
 * - Practical application of analog input
 */

#include <PotentiometerController.h>

const int VR_PIN = 34;
const int LED_PINS[] = {2, 4, 5, 18};
const int NUM_LEDS = 4;

PotentiometerController vr(VR_PIN);

int currentLED = 0;
unsigned long lastChange = 0;
int delayTime = 500;

void setup() {
  Serial.begin(115200);
  vr.begin();
  
  // Setup LED pins
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }
  
  Serial.println("=== VR-Controlled Running Light ===");
  Serial.println("Rotate VR to change speed");
  Serial.println();
}

void loop() {
  // Read VR and map to delay time (inverted: high VR = fast = low delay)
  delayTime = vr.readMapped(1000, 50); // 1000ms (slow) to 50ms (fast)
  
  // Running light logic
  unsigned long now = millis();
  if (now - lastChange >= delayTime) {
    lastChange = now;
    
    // Turn off all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      digitalWrite(LED_PINS[i], LOW);
    }
    
    // Turn on current LED
    digitalWrite(LED_PINS[currentLED], HIGH);
    
    // Move to next LED
    currentLED = (currentLED + 1) % NUM_LEDS;
    
    // Display status
    Serial.print("Speed: ");
    Serial.print(vr.readPercent());
    Serial.print("%  |  Delay: ");
    Serial.print(delayTime);
    Serial.println(" ms");
  }
}
