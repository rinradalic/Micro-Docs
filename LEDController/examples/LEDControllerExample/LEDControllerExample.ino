#include <LEDController.h>

// Use built-in LED pin for many boards; change if needed
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

LEDController led(LED_BUILTIN);

void setup() {
  Serial.begin(115200);
  led.begin();
  Serial.println("LEDController example started");

  // Start blinking at 300ms
  led.startBlink(300);
}

unsigned long lastAction = 0;
int step = 0;

void loop() {
  led.update();

  unsigned long now = millis();
  // Demonstrate API every 5 seconds
  if (now - lastAction > 5000) {
    lastAction = now;
    step = (step + 1) % 4;
    switch (step) {
      case 0:
        Serial.println("on()");
        led.on();
        break;
      case 1:
        Serial.println("off()");
        led.off();
        break;
      case 2:
        Serial.println("toggle()");
        led.toggle();
        break;
      case 3:
        Serial.println("startBlink(200)");
        led.startBlink(200);
        break;
    }
  }
}
