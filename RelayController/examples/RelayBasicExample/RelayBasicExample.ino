/*
 * RelayBasicExample
 * 
 * Basic relay control - toggles relay ON/OFF every second.
 * Listen for the "click" sound from the relay.
 * 
 * Hardware:
 * - ICON-32 (ESP32) or Arduino
 * - Relay module or transistor-driven relay on pin 26
 * - Optional: LED on relay to see state visually
 * 
 * Circuit:
 * - GPIO26 → [1kΩ] → Transistor Base (NPN: 2N2222/BC547)
 * - Transistor Collector → Relay Coil (-)
 * - Relay Coil (+) → +5V/6V
 * - Diode 1N4007 across relay coil (cathode to +V)
 * - Transistor Emitter → GND
 * 
 * Safety: Do NOT connect high voltage (220V) loads for testing.
 *         Use 5V/6V loads (LED, small fan, buzzer) only.
 */

#include <RelayController.h>

const int RELAY_PIN = 26; // GPIO26 on ESP32 (ICON-32)

RelayController relay(RELAY_PIN);

void setup() {
  Serial.begin(115200);
  relay.begin();
  Serial.println("Relay Basic Example");
  Serial.println("Relay will toggle every 1 second");
}

void loop() {
  relay.on();
  Serial.println("Relay ON");
  delay(1000);
  
  relay.off();
  Serial.println("Relay OFF");
  delay(1000);
}
