/*
 * VR_DualLED_Control
 * 
 * Advanced VR control example with two LEDs:
 * - LED1: Normal brightness (follows VR position)
 * - LED2: Inverted brightness (opposite of VR position)
 * 
 * Hardware (ESP32/ICON-32):
 * - VR → GPIO 34 (ADC)
 * - LED1 → GPIO 2 (via 220Ω resistor)
 * - LED2 → GPIO 4 (via 220Ω resistor)
 * 
 * Circuit:
 *   GPIO 2 ──[220Ω]──►|── GND  (LED1)
 *   GPIO 4 ──[220Ω]──►|── GND  (LED2)
 * 
 * Behavior:
 * - VR high → LED1 bright, LED2 dim
 * - VR low → LED1 dim, LED2 bright
 * 
 * Learning objectives (EP4):
 * - Multiple PWM channels
 * - Inverted/reversed mapping
 * - Complementary outputs
 */

#include <PotentiometerController.h>

const int VR_PIN = 34;
const int LED1_PIN = 2;   // Normal LED
const int LED2_PIN = 4;   // Inverted LED

// PWM channels
const int PWM_CH1 = 0;
const int PWM_CH2 = 1;
const int PWM_FREQ = 5000;
const int PWM_RES = 8;

PotentiometerController vr(VR_PIN);

void setup() {
  Serial.begin(115200);
  vr.begin();
  
  // Setup two PWM channels
  ledcSetup(PWM_CH1, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED1_PIN, PWM_CH1);
  
  ledcSetup(PWM_CH2, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED2_PIN, PWM_CH2);
  
  Serial.println("=== Dual LED Control with VR ===");
  Serial.println("LED1: Normal | LED2: Inverted");
  Serial.println();
}

void loop() {
  int pwm1 = vr.readMapped(0, 255);  // Normal
  int pwm2 = 255 - pwm1;             // Inverted
  
  ledcWrite(PWM_CH1, pwm1);
  ledcWrite(PWM_CH2, pwm2);
  
  Serial.print("VR: ");
  Serial.print(vr.readPercent());
  Serial.print("%  →  LED1: ");
  Serial.print(map(pwm1, 0, 255, 0, 100));
  Serial.print("%  |  LED2: ");
  Serial.print(map(pwm2, 0, 255, 0, 100));
  Serial.println("%");
  
  delay(100);
}
