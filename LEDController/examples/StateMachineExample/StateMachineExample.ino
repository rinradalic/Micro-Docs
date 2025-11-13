/*
 * StateMachineExample
 * 
 * Demonstrates using Button and LEDController together to implement
 * a simple state machine with 4 states:
 * - STATE_IDLE: LED off
 * - STATE_ON: LED on (solid)
 * - STATE_BLINK: LED blinking slowly (600ms)
 * - STATE_ALERT: LED blinking fast (150ms)
 * 
 * Press the button to cycle through states.
 * 
 * Hardware:
 * - Button on pin 4 (with internal pullup)
 * - LED on pin 2 (or use LED_BUILTIN)
 */

#include <Button.h>
#include <LEDController.h>

// Define system states
enum SystemState {
  STATE_IDLE = 0,   // ไฟดับ
  STATE_ON,         // ไฟติดค้าง
  STATE_BLINK,      // ไฟกระพริบช้า
  STATE_ALERT       // ไฟกระพริบเร็ว
};

// Configure pins
const int BUTTON_PIN = 4;
const int LED_PIN = 2;  // Change to LED_BUILTIN if needed

// Create instances
Button btn(BUTTON_PIN, true, 80);   // Pin 4, pullup enabled, 80ms debounce
LEDController led(LED_PIN);         // Pin 2

// Current system state
SystemState currentState = STATE_IDLE;

// Apply the current state to the LED
void applyState(SystemState s) {
  switch (s) {
    case STATE_IDLE:
      led.off();
      Serial.println("→ STATE_IDLE (LED off)");
      break;
    case STATE_ON:
      led.on();
      Serial.println("→ STATE_ON (LED solid)");
      break;
    case STATE_BLINK:
      led.startBlink(600);   // Slow blink
      Serial.println("→ STATE_BLINK (slow)");
      break;
    case STATE_ALERT:
      led.startBlink(150);   // Fast blink
      Serial.println("→ STATE_ALERT (fast)");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("StateMachine Example Started");
  Serial.println("Press button to cycle through states");
  
  btn.begin();
  led.begin();
  applyState(currentState);   // Apply initial state
}

void loop() {
  // Update button and LED
  btn.update();
  led.update();

  // Check for button press to change state
  if (btn.wasPressed()) {
    // Cycle to next state (wraps around after STATE_ALERT)
    currentState = (SystemState)((currentState + 1) % 4);
    
    Serial.print("Button pressed! State: ");
    Serial.print(currentState);
    Serial.print(" ");
    
    applyState(currentState);
  }
}
