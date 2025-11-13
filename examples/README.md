# EP7.2: FreeRTOS Multi-tasking

**สแกนจอ 7-Segment + อ่านปุ่ม + อ่านเซนเซอร์ "พร้อมกัน" บน ESP32**

---

## 🎯 เป้าหมายการเรียนรู้

เมื่อจบ EP7.2 นักเรียนจะสามารถ:

### เข้าใจ FreeRTOS
- รู้ว่า ESP32 รันอยู่บน FreeRTOS อยู่แล้ว
- เข้าใจ concept ของ **Multi-tasking**
- เข้าใจ **Cooperative Scheduling**

### ใช้งาน FreeRTOS API
- `xTaskCreate()` - สร้าง Task ใหม่
- `vTaskDelay()` - หน่วงเวลาแบบไม่บล็อก
- เข้าใจ **Task Priority**, **Stack Size**

### ออกแบบระบบ Multi-task
- แยกงานเป็น Tasks หลายตัว
- จัดการ **Shared Variables** (volatile)
- หลีกเลี่ยง **Race Conditions**

### บูรณาการ Libraries
- ใช้หลาย Libraries ร่วมกัน
- สร้างระบบจริงแบบ **Production-ready**

---

## 🧠 แนวคิด: จาก loop() เดียว → หลาย Task

### ปกติ (Arduino Style)
```cpp
void setup() { ... }

void loop() {
  // ทุกอย่างรวมกันในนี้
  scanDisplay();
  readButton();
  readSensor();
  controlRelay();
  // ... มีโอกาสชนกัน
}
```

**ปัญหา:**
- `delay()` บล็อกทั้งระบบ
- งานทั้งหมดต้องรอกัน
- ยากต่อการจัดการ timing

### FreeRTOS Style
```cpp
void setup() {
  xTaskCreate(TaskDisplay, ...);
  xTaskCreate(TaskButtons, ...);
  xTaskCreate(TaskSensor, ...);
}

void loop() {
  // ว่างเปล่า - Tasks จัดการเอง
}
```

**ข้อดี:**
- แต่ละ Task มีลูปของตัวเอง
- `vTaskDelay()` ไม่บล็อก Task อื่น
- จัดการ timing ง่าย

---

## ⚙️ FreeRTOS API หลัก

### 1. xTaskCreate()

สร้าง Task ใหม่

```cpp
xTaskCreate(
  TaskFunction_t pvTaskCode,   // ฟังก์ชันของ Task
  const char * const pcName,   // ชื่อ Task (debug)
  const uint16_t usStackDepth, // ขนาด Stack (words)
  void *pvParameters,          // พารามิเตอร์ส่งเข้า Task
  UBaseType_t uxPriority,      // ความสำคัญ (1-25)
  TaskHandle_t *pxCreatedTask  // Handle (ไม่ใช้ = NULL)
);
```

**ตัวอย่าง:**
```cpp
xTaskCreate(
  TaskDisplay,  // ฟังก์ชัน
  "Display",    // ชื่อ
  2048,         // Stack 2048 words
  NULL,         // ไม่ส่ง parameter
  2,            // Priority = 2
  NULL          // ไม่เก็บ handle
);
```

### 2. vTaskDelay()

หน่วงเวลาแบบไม่บล็อก CPU

```cpp
vTaskDelay(ms / portTICK_PERIOD_MS);
```

**❌ ห้ามใช้ในงาน Task:**
```cpp
delay(1000);  // บล็อก Task นี้ แต่ไม่ปล่อย CPU
```

**✅ ใช้แทน:**
```cpp
vTaskDelay(1000 / portTICK_PERIOD_MS);  // ปล่อย CPU ให้ Task อื่น
```

### 3. Task Function Template

```cpp
void MyTask(void *pvParameters) {
  (void) pvParameters;  // ละเลยถ้าไม่ใช้
  
  // Setup (ทำครั้งเดียว)
  pinMode(...);
  
  while (1) {
    // ลูปหลัก
    // ... ทำงาน ...
    
    vTaskDelay(ms / portTICK_PERIOD_MS);
  }
}
```

---

## 📂 โครงสร้างตัวอย่าง

### 1. FreeRTOS_MultiTask_Basic

**พื้นฐาน 3 Tasks**

```
TaskDisplay  (Priority 2) → สแกน 7-segment @ 2ms
TaskButtons  (Priority 1) → อ่านปุ่ม @ 10ms
TaskSensor   (Priority 1) → อ่าน DHT11 @ 1s
```

**Hardware:**
- 7-segment 4-digit
- DHT11 sensor
- 1 ปุ่ม (MODE)

**การทำงาน:**
- แสดงอุณหภูมิ/ความชื้น
- กดปุ่มสลับโหมด

---

### 2. FreeRTOS_TempDisplay

**4 Tasks + Decimal Point**

```
TaskDisplay  (Priority 2) → สแกน 7-segment @ 2ms
TaskButtons  (Priority 1) → อ่านปุ่ม 3 ปุ่ม @ 10ms
TaskSensor   (Priority 1) → อ่าน DHT11 @ 1s
TaskMonitor  (Priority 1) → ตรวจสอบ threshold @ 500ms
```

**Features:**
- แสดง xx.x°C (จุดทศนิยม)
- ปรับ threshold ด้วย UP/DOWN
- LED แจ้งเตือนเมื่อเกิน threshold

---

### 3. FreeRTOS_SmartController

**6 Tasks - โปรเจกต์บูรณาการ**

```
TaskDisplay    (Priority 3) → สแกนจอ @ 2ms
TaskButtons    (Priority 2) → อ่านปุ่ม 4 ปุ่ม @ 10ms
TaskSensor     (Priority 1) → อ่าน DHT22 @ 2s
TaskController (Priority 2) → ควบคุมพัดลม @ 100ms
TaskLEDs       (Priority 1) → อัปเดต LED @ 100ms
TaskLogger     (Priority 1) → Serial log @ 5s
```

**Features:**
- ควบคุมพัดลมด้วย Hysteresis
- โหมด AUTO/MANUAL
- LED status 4 ดวง
- Serial logging

**Integration:**
- DigitalLogic (7-segment)
- Button (AdvancedButton)
- TemperatureSensor (DHT)
- RelayController
- LEDController

---

## 🎛 Task Priority Design

### Priority Levels

| Priority | Use Case | Example |
|----------|----------|---------|
| 0 | Idle Task (อย่าใช้) | - |
| 1 | งานธรรมดา | Sensor reading, Logging |
| 2 | งานสำคัญ | Button handling, Control logic |
| 3 | งานเร่งด่วน | Display scanning |
| 4-5 | Real-time | Critical timing |
| > 5 | สูงมาก (ระวัง!) | Hardware interrupts |

### ตัวอย่างการออกแบบ Priority

**SmartController:**
```
Priority 3: TaskDisplay    → ต้องสแกนเร็วมาก
Priority 2: TaskButtons    → ต้องตอบสนองไว
Priority 2: TaskController → ควบคุม real-time
Priority 1: TaskSensor     → อ่านช้าได้
Priority 1: TaskLEDs       → ไม่เร่งด่วน
Priority 1: TaskLogger     → ไม่เร่งด่วน
```

**หลักการ:**
- งานที่ทำบ่อยและต้อง timing แม่นยำ → Priority สูง
- งานที่ช้าได้, ไม่เร่งด่วน → Priority ต่ำ

---

## 🧰 Stack Size Guidelines

### เลือก Stack Size

| Size (words) | Use Case |
|--------------|----------|
| 1024 | Task เล็ก ๆ (LED blink) |
| 2048 | งานทั่วไป (display, buttons) |
| 4096 | ใช้ Libraries (DHT, WiFi) |
| 8192+ | Buffer ใหญ่, Complex calculations |

**⚠️ Stack Overflow:**
- ถ้า Stack ไม่พอ → ESP32 จะ panic/reset
- เพิ่ม Stack size จนกว่าจะพอ

**ตรวจสอบ Stack:**
```cpp
Serial.print("Stack watermark: ");
Serial.println(uxTaskGetStackHighWaterMark(NULL));
```

---

## 🔄 Shared Variables

### ใช้ volatile

```cpp
volatile float g_temperature = 0.0;  // ✅ ถูกต้อง
```

**เพราะ:**
- Compiler ไม่ optimize ออก
- Task อื่น ๆ เห็นค่าล่าสุดเสมอ

### ⚠️ Race Conditions

**ปัญหา:**
```cpp
// Task 1
g_counter++;  // อ่าน → +1 → เขียน

// Task 2
g_counter++;  // อาจเกิดพร้อมกัน!
```

**วิธีแก้:**
1. ใช้ `portENTER_CRITICAL()` / `portEXIT_CRITICAL()`
2. ใช้ Mutex / Semaphore
3. ออกแบบให้ Task เดียวเขียน

---

## 📚 ตัวอย่าง Code Patterns

### Pattern 1: Sensor Reading Task

```cpp
void TaskSensor(void *pvParameters) {
  (void) pvParameters;
  
  dht.begin();
  vTaskDelay(2000 / portTICK_PERIOD_MS);  // รอเสถียร
  
  while (1) {
    float t = dht.readTemperature();
    
    if (!isnan(t)) {
      g_temperature = t;  // อัปเดตตัวแปรแชร์
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // อ่านทุก 1s
  }
}
```

### Pattern 2: Display Scanning Task

```cpp
void TaskDisplay(void *pvParameters) {
  (void) pvParameters;
  
  int digit = 0;
  
  while (1) {
    // ปิดทุก digit
    for (int i = 0; i < 4; i++) {
      digitalWrite(digitPin[i], HIGH);
    }
    
    // เปิด digit ปัจจุบัน
    setSegments(displayData[digit]);
    digitalWrite(digitPin[digit], LOW);
    
    digit = (digit + 1) % 4;
    
    vTaskDelay(2 / portTICK_PERIOD_MS);  // สแกนเร็ว
  }
}
```

### Pattern 3: Button Handling Task

```cpp
void TaskButtons(void *pvParameters) {
  (void) pvParameters;
  
  button.begin();
  
  while (1) {
    button.update();
    
    if (button.wasPressed()) {
      // ทำงานเมื่อกดปุ่ม
      g_mode = (g_mode + 1) % 2;
    }
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
```

---

## 🔧 Troubleshooting

### ESP32 panic/reset
**สาเหตุ:** Stack overflow
**แก้ไข:** เพิ่ม Stack size ใน xTaskCreate()

### Task ไม่ทำงาน
**สาเหตุ:** Priority ต่ำเกิน
**แก้ไข:** เพิ่ม Priority

### จอกระพริบ
**สาเหตุ:** TaskDisplay ช้า (Priority ต่ำ หรือ delay นาน)
**แก้ไข:** เพิ่ม Priority, ลด delay

### ค่า sensor กระโดด
**สาเหตุ:** Race condition
**แก้ไข:** ใช้ Critical Section หรือ Mutex

### Serial.print() ช้า
**สาเหตุ:** print มากเกินไปใน Task ความถี่สูง
**แก้ไข:** print ใน Task แยก (TaskLogger)

---

## 📖 Teaching Guide (สำหรับครู)

### EP7.2 Lesson Plan (4-6 hours)

#### Part 1: FreeRTOS Concepts (1 hour)
**Objectives:**
- เข้าใจ Multi-tasking
- เข้าใจ Task, Priority, Stack

**Activities:**
1. อธิบาย Single loop vs Multi-task
2. ยกตัวอย่าง real-world (คนทำงานหลายอย่างพร้อมกัน)
3. อธิบาย FreeRTOS Scheduler

**Teaching Tip:**
- ใช้อุปมา: "โรงงาน" (ESP32) มี "คนงาน" (Tasks) หลายคน
- แต่ละคนทำงานของตัวเอง แต่ใช้ "ของรวม" (Shared Variables)

---

#### Part 2: สร้าง Task แรก (1-1.5 hours)
**Objectives:**
- ใช้ xTaskCreate()
- ใช้ vTaskDelay()

**Activities:**
1. Live code: สร้าง TaskBlink (กระพริบ LED)
2. เพิ่ม Task ที่ 2: TaskBlink2 (LED อีกดวง)
3. สังเกต: 2 LEDs กระพริบไม่พร้อมกัน

**Code:**
```cpp
void TaskBlink1(void *pvParameters) {
  pinMode(2, OUTPUT);
  while (1) {
    digitalWrite(2, !digitalRead(2));
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void TaskBlink2(void *pvParameters) {
  pinMode(15, OUTPUT);
  while (1) {
    digitalWrite(15, !digitalRead(15));
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}
```

---

#### Part 3: Multi-task Basic (2-2.5 hours)
**Objectives:**
- สร้าง 3 Tasks ทำงานพร้อมกัน
- ใช้ Shared Variables

**Activities:**
1. Run ตัวอย่าง: FreeRTOS_MultiTask_Basic
2. อธิบาย code ทีละ Task
3. ให้นักเรียนแก้:
   - เปลี่ยน timing
   - เพิ่ม LED status
   - เพิ่มปุ่มอีก 1 ปุ่ม

**Challenges:**
- เพิ่ม TaskBlink ให้ LED กระพริบเฉพาะเมื่ออุณหภูมิ > 30°C
- สร้าง Task นับเลขถอยหลัง

---

#### Part 4: โปรเจกต์บูรณาการ (1-2 hours)
**Objectives:**
- บูรณาการ Libraries ทั้งหมด
- ออกแบบระบบจริง

**Activities:**
1. อธิบาย SmartController architecture
2. Run ตัวอย่าง FreeRTOS_SmartController
3. ให้นักเรียนเพิ่ม feature:
   - เปลี่ยนสี LED ตามอุณหภูมิ
   - เพิ่ม Buzzer แจ้งเตือน
   - บันทึกข้อมูลลง SD card

---

### Assessment Ideas

**Basic:**
- สร้าง 2 Tasks: Blink + Button
- อธิบาย Priority คืออะไร

**Intermediate:**
- สร้างระบบแสดงอุณหภูมิบน 7-segment
- ใช้ 3 Tasks: Display, Sensor, Buttons

**Advanced:**
- โปรเจกต์ SmartController ครบ 6 Tasks
- เพิ่ม WiFi ส่งข้อมูลขึ้น Cloud
- ออกแบบ State Machine ซับซ้อน

---

## 🔥 การต่อยอด

### Level 1: เพิ่ม Tasks
- TaskBlink - กระพริบ status LED
- TaskBuzzer - เสียงแจ้งเตือน
- TaskAnimation - แสดงผล animation

### Level 2: Advanced Features
- TaskWiFi - ส่งข้อมูลขึ้น MQTT
- TaskSD - บันทึก data logging
- TaskBluetooth - Control ผ่าน mobile app

### Level 3: Synchronization
- Queue - ส่งข้อมูลระหว่าง Tasks
- Semaphore - ป้องกัน race condition
- Mutex - จัดการ shared resources

---

## 📊 เปรียบเทียบ: Loop vs FreeRTOS

| หัวข้า | Single Loop | FreeRTOS |
|--------|-------------|----------|
| **Complexity** | ง่าย | ซับซ้อนขึ้น |
| **Timing Control** | ยาก | ง่าย |
| **Scalability** | จำกัด | ดีมาก |
| **Code Organization** | รกง่าย | เป็นระบบ |
| **Debug** | ง่าย | ยากขึ้น |
| **Production Ready** | พอใช้ | เหมาะมาก |

**สรุป:**
- โปรเจกต์เล็ก → Single Loop พอ
- โปรเจกต์ใหญ่/ซับซ้อน → ใช้ FreeRTOS

---

## 📄 License

MIT License - Free for educational and commercial use.

---

**Micro-Docs Library Collection**  
Educational Arduino/ESP32 libraries for hands-on learning
