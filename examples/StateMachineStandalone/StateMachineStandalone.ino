/*
 * StateMachineStandalone
 * 
 * Self-contained version with Button and LEDController classes embedded.
 * Demonstrates a simple state machine with 4 states:
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

// ----- Button Class -----
class Button {
  private:
    int pin;
    bool usePullup;
    unsigned long lastChangeTime;
    unsigned long debounceMs;
    bool lastStableState;
    bool currentState;
    bool pressedEvent;

  public:
    Button(int pinNumber, bool enablePullup = true, unsigned long debounce = 50) {
      pin = pinNumber;
      usePullup = enablePullup;
      debounceMs = debounce;
      lastChangeTime = 0;
      lastStableState = HIGH;
      currentState = HIGH;
      pressedEvent = false;
    }

    void begin() {
      if (usePullup) {
        pinMode(pin, INPUT_PULLUP);
        lastStableState = HIGH;
      } else {
        pinMode(pin, INPUT);
        lastStableState = LOW;
      }
      currentState = lastStableState;
    }

    void update() {
      int reading = digitalRead(pin);
      unsigned long now = millis();
      pressedEvent = false;

      if (reading != currentState) {
        currentState = reading;
        lastChangeTime = now;
      }

      if (now - lastChangeTime > debounceMs) {
        if (lastStableState != currentState) {
          if (usePullup) {
            if (lastStableState == HIGH && currentState == LOW) {
              pressedEvent = true;
            }
          } else {
            if (lastStableState == LOW && currentState == HIGH) {
              pressedEvent = true;
            }
          }
          lastStableState = currentState;
        }
      }
    }

    bool wasPressed() { return pressedEvent; }
};


// ----- LEDController Class -----
class LEDController {
  private:
    int pin;
    bool state;
    bool blinking;
    unsigned long interval;
    unsigned long lastToggle;

  public:
    LEDController(int pinNumber) {
      pin = pinNumber;
      state = false;
      blinking = false;
      interval = 500;
      lastToggle = 0;
    }

    void begin() {
      pinMode(pin, OUTPUT);
      off();
    }

    void on() {
      state = true;
      blinking = false;
      digitalWrite(pin, HIGH);
    }

    void off() {
      state = false;
      blinking = false;
      digitalWrite(pin, LOW);
    }

    void toggle() {
      state = !state;
      blinking = false;
      digitalWrite(pin, state ? HIGH : LOW);
    }

    void startBlink(unsigned long ms) {
      interval = ms;
      blinking = true;
      lastToggle = millis();
    }

    void stopBlink() {
      blinking = false;
      digitalWrite(pin, state ? HIGH : LOW);
    }

    void update() {
      if (blinking) {
        unsigned long now = millis();
        if (now - lastToggle >= interval) {
          lastToggle = now;
          state = !state;
          digitalWrite(pin, state ? HIGH : LOW);
        }
      }
    }
};

// ----- System States -----
enum SystemState {
  STATE_IDLE = 0,   // ไฟดับ
  STATE_ON,         // ไฟติดค้าง
  STATE_BLINK,      // ไฟกระพริบช้า
  STATE_ALERT       // ไฟกระพริบเร็ว
};

// ----- Configuration -----
Button btn(4, true, 80);   // ปุ่มที่ขา 4
LEDController led(2);      // LED ที่ขา 2

SystemState currentState = STATE_IDLE;

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
      led.startBlink(600);   // กระพริบช้า
      Serial.println("→ STATE_BLINK (600ms)");
      break;
    case STATE_ALERT:
      led.startBlink(150);   // กระพริบเร็ว
      Serial.println("→ STATE_ALERT (150ms)");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== State Machine Example ===");
  Serial.println("Press button to cycle through states:");
  Serial.println("  0: IDLE (off)");
  Serial.println("  1: ON (solid)");
  Serial.println("  2: BLINK (slow)");
  Serial.println("  3: ALERT (fast)");
  Serial.println();
  
  btn.begin();
  led.begin();
  applyState(currentState);   // ตั้งสถานะเริ่มต้น
}

void loop() {
  // อัปเดต input / output
  btn.update();
  led.update();

  // เช็คว่ามีการกดปุ่มเพื่อ "เปลี่ยนสถานะ" หรือไม่
  if (btn.wasPressed()) {
    // เปลี่ยนสถานะถัดไปแบบวนลูป
    currentState = (SystemState)((currentState + 1) % 4);

    Serial.print("Button pressed! State changed to: ");
    Serial.print(currentState);
    Serial.print(" ");

    applyState(currentState);
  }
}
