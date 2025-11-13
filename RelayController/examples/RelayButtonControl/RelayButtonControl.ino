/*
 * RelayButtonControl
 * 
 * Control relay with a push button - toggle relay state on each press.
 * Uses debounced Button class from Button library.
 * 
 * Hardware:
 * - ICON-32 (ESP32) or Arduino
 * - Relay module or transistor-driven relay on pin 26
 * - Push button on pin 4 (with internal pullup)
 * - Optional: Load connected to relay (LED, fan, etc.)
 * 
 * Behavior:
 * - Press button → relay toggles ON/OFF
 * - LED indicator shows current state
 * 
 * Safety: Use only low-voltage loads (5V/6V) for testing.
 */

#include <Button.h>
#include <RelayController.h>

const int RELAY_PIN = 26;
const int BUTTON_PIN = 4;

RelayController relay(RELAY_PIN);
Button button(BUTTON_PIN, true, 80); // Pullup enabled, 80ms debounce

void setup() {
  Serial.begin(115200);
  relay.begin();
  button.begin();
  
  Serial.println("Relay Button Control Example");
  Serial.println("Press button to toggle relay");
  Serial.println("Initial state: OFF");
}

void loop() {
  button.update();
  
  if (button.wasPressed()) {
    relay.toggle();
    
    Serial.print("Button pressed! Relay is now: ");
    Serial.println(relay.isOn() ? "ON" : "OFF");
  }
}
