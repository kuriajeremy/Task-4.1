// Task4.1Interrupts_Wokwi.ino
// SIT210 - Task 4.1P — Handling Interrupts (Wokwi Version)
//
// SUBSTITUTIONS FOR WOKWI:
//   BH1750 replaced with photoresistor (LDR) on A0
//   PIR Motion Sensor — use the real PIR part in Wokwi (search "pir")
//   Slider switch = push button on D2
//
// CALIBRATION (from Task 3.1P):
//   LDR reading > 900 = DARK
//   LDR reading <= 900 = BRIGHT
//
// HOW PIR WORKS IN WOKWI:
//   Click the PIR sensor in the simulation to trigger a motion event
//   The OUT pin goes HIGH briefly — this fires the interrupt on D3
//
// BEHAVIOUR:
//   PIR triggers motion interrupt on D3
//     → if dark (LDR reading > 900): both LEDs ON for 60 seconds
//     → if bright (LDR reading <= 900): lights stay OFF
//   Button on D2 = manual override
//     → LEDs ON regardless of light level
//   After 60 seconds: LEDs turn OFF automatically

// ─── PIN DEFINITIONS ────────────────────────────────────────────────────────
const int PIR_PIN    = 3;   // PIR sensor OUT pin
const int SLIDER_PIN = 2;   // Push button — manual override
const int LED1_PIN   = 4;   // Porch light
const int LED2_PIN   = 5;   // Hallway light
const int LIGHT_PIN  = A0;  // LDR photoresistor (substitutes BH1750)

// ─── CALIBRATED THRESHOLD ───────────────────────────────────────────────────
const int DARK_THRESHOLD = 900;  // LDR reading above this = dark

// ─── TIMING ─────────────────────────────────────────────────────────────────
const unsigned long LIGHT_DURATION = 60000;  // 60 seconds

// ─── VOLATILE FLAGS (set inside ISRs) ───────────────────────────────────────
volatile bool motionDetected  = false;
volatile bool sliderTriggered = false;

// ─── STATE ──────────────────────────────────────────────────────────────────
bool lightsOn = false;
unsigned long lightsOnTime = 0;

// ─── SETUP ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  setupPins();
  attachInterrupts();
  Serial.println("=== Interrupt System Ready ===");
  Serial.println("PIR on D3 — click PIR in Wokwi to simulate motion");
  Serial.println("Button on D2 — click button for manual override");
  Serial.println("LDR on A0  — drag slider to set light level");
  Serial.println("Reading > 900 = DARK  |  Reading <= 900 = BRIGHT");
  Serial.println("==============================");
}

// ─── MAIN LOOP ──────────────────────────────────────────────────────────────
void loop() {
  handleMotionTrigger();
  handleSliderTrigger();
  handleLightTimeout();
}

// ─── MODULE: Pin Setup ───────────────────────────────────────────────────────
void setupPins() {
  pinMode(PIR_PIN,    INPUT);         // PIR output is active HIGH — no pullup needed
  pinMode(SLIDER_PIN, INPUT_PULLUP);  // Button pulls to GND when pressed
  pinMode(LED1_PIN,   OUTPUT);
  pinMode(LED2_PIN,   OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
}

// ─── MODULE: Interrupt Attachment ───────────────────────────────────────────
// PIR fires on RISING (motion detected = OUT goes HIGH)
// Button fires on FALLING (pressed = pulled to GND)
void attachInterrupts() {
  attachInterrupt(digitalPinToInterrupt(PIR_PIN),    onMotionDetected, RISING);
  attachInterrupt(digitalPinToInterrupt(SLIDER_PIN), onSliderToggled,  FALLING);
  Serial.println("Interrupts attached on D2 and D3.");
}

// ─── ISR: Motion Detected ────────────────────────────────────────────────────
// Minimal — sets flag only, no Serial or delay inside ISR
void onMotionDetected() {
  motionDetected = true;
}

// ─── ISR: Slider / Manual Override ──────────────────────────────────────────
// Minimal — sets flag only
void onSliderToggled() {
  sliderTriggered = true;
}

// ─── MODULE: Motion Trigger Handler ─────────────────────────────────────────
// Checks darkness before activating lights
void handleMotionTrigger() {
  if (motionDetected) {
    motionDetected = false;
    int lightValue = analogRead(LIGHT_PIN);
    Serial.print("Motion detected! LDR reading: " + String(lightValue));
    Serial.println(lightValue > DARK_THRESHOLD ? " (DARK)" : " (BRIGHT — too bright, lights not needed)");
    if (lightValue > DARK_THRESHOLD) {
      activateLights("PIR motion sensor");
    }
  }
}

// ─── MODULE: Slider Trigger Handler ─────────────────────────────────────────
// Manual override — bypasses darkness check
void handleSliderTrigger() {
  if (sliderTriggered) {
    sliderTriggered = false;
    Serial.println("Manual override — slider switch pressed.");
    activateLights("manual slider switch");
  }
}

// ─── MODULE: Light Activation ────────────────────────────────────────────────
void activateLights(String source) {
  if (!lightsOn) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    lightsOn = true;
    lightsOnTime = millis();
    Serial.println(">>> Lights ON via " + source + " — turning off in 60s.");
  } else {
    Serial.println("Lights already ON — ignoring.");
  }
}

// ─── MODULE: Light Timeout ───────────────────────────────────────────────────
void handleLightTimeout() {
  if (lightsOn && (millis() - lightsOnTime >= LIGHT_DURATION)) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    lightsOn = false;
    Serial.println(">>> Lights OFF — 60 second timeout.");
  }
}
