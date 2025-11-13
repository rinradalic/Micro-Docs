/*
 * VR_LED_Brightness
 * 
 * Control LED brightness using potentiometer (VR) and PWM.
 * Demonstrates analog-to-PWM conversion on ESP32.
 * 
 * Hardware (ESP32):
 * - VR Pin 1 → 3.3V
 * - VR Pin 2 (Center/Wiper) → GPIO 34 (ADC pin)
 * - VR Pin 3 → GND
 * - LED Anode → GPIO 2 (via 220Ω resistor)
 * - LED Cathode → GND
 * 
 * Circuit:
 *   3.3V ───┐
 *           │
 *         [ VR ]
 *           │
 *   GND ────┘
 *     Wiper → GPIO 34
 * 
 *   GPIO 2 ──[220Ω]──►|── GND
 *                    LED
 * 
 * Behavior:
 * - Rotate VR clockwise → LED brighter
 * - Rotate VR counter-clockwise → LED dimmer
 */

#include <PotentiometerController.h>

const int VR_PIN = 34;    // ADC pin
const int LED_PIN = 2;    // PWM output pin

// PWM configuration for ESP32
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;      // 5 kHz
const int PWM_RESOLUTION = 8;   // 8-bit (0-255)

PotentiometerController vr(VR_PIN);

void setup() {
  Serial.begin(115200);
  vr.begin();
  
  // Setup PWM on ESP32
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
  
  Serial.println("=== VR → LED Brightness Control ===");
  Serial.println("Rotate VR to control LED brightness");
  Serial.println();
}

void loop() {
  // Read VR and map to PWM range (0-255)
  int pwmValue = vr.readMapped(0, 255);
  
  // Set LED brightness
  ledcWrite(PWM_CHANNEL, pwmValue);
  
  // Display values
  Serial.print("RAW: ");
  Serial.print(vr.getRaw());
  Serial.print("  →  PWM: ");
  Serial.print(pwmValue);
  Serial.print("  →  Brightness: ");
  Serial.print(map(pwmValue, 0, 255, 0, 100));
  Serial.println("%");
  
  delay(50);
}
