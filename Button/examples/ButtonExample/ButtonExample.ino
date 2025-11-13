#include <Button.h>

// Adjust pin for your board
const int buttonPin = 2;

Button button(buttonPin, true, 50);

void setup() {
  Serial.begin(115200);
  button.begin();
  Serial.println("Button example started");
}

void loop() {
  button.update();

  if (button.wasPressed()) {
    Serial.println("Button was pressed!");
  }

  // optional: print current state every second
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last > 1000) {
    last = now;
    Serial.print("isPressed(): ");
    Serial.println(button.isPressed() ? "YES" : "NO");
  }
}
