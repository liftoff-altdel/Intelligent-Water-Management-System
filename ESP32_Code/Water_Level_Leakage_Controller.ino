#include <Arduino.h>

// =======================================
// --- Ultrasonic Sensor ---
#define TRIG_PIN 12
#define ECHO_PIN 14

// --- L298N Motor Pins ---
#define IN1 13
#define IN2 15
#define IN3 16
#define IN4 17
#define ENA 2   // PWM for inlet motor
#define ENB 4   // PWM for outlet motor

// --- PID Parameters ---
float Kp = 2.0;
float Ki = 0.4;
float Kd = 1.0;

// --- Level Control Parameters ---
float setpoint = 16.0;      // Target water level (cm)
float tolerance = 1.0;      // Allowed deviation (±1 cm)
float hysteresis = 0.5;     // Hysteresis buffer
float prevError = 0;
float integral = 0;

// --- Debounce Parameters ---
int stableCount = 0;          // Counts consistent readings
const int debounceLimit = 2;  // Require 2 stable readings before switching state

// --- System States ---
enum SystemState { FILLING, DRAINING, BALANCED, IDLE };
SystemState currentState = IDLE;
SystemState nextState = BALANCED;

// =======================================
// --- Dual Flow Sensor Setup ---
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;

unsigned long lastFlowTime = 0;
float flowRate1 = 0;
float flowRate2 = 0;

// Interrupt Service Routines
void IRAM_ATTR pulseCounter1() {
  pulseCount1++;
}
void IRAM_ATTR pulseCounter2() {
  pulseCount2++;
}

// =======================================
// --- Ultrasonic Reading Function ---
float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  float distance = (duration * 0.0343) / 2.0; // cm
  return distance;
}

// =======================================
// --- Motor Control Functions ---
void runInletMotor(int pwm) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwm);
}

void runOutletMotor(int pwm) {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, pwm);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =======================================
// --- Setup ---
void setup() {
  Serial.begin(115200);

  // Ultrasonic + Motor Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Flow Sensors
  pinMode(19, INPUT_PULLUP);  // Flow Sensor 1
  pinMode(32, INPUT_PULLUP);  // Flow Sensor 2
  attachInterrupt(digitalPinToInterrupt(19), pulseCounter1, RISING);
  attachInterrupt(digitalPinToInterrupt(32), pulseCounter2, RISING);

  Serial.println("System initialized with hysteresis + debounce (2 readings).");
}

// =======================================
// --- Main Loop ---
void loop() {
  // ===========================
  // --- Ultrasonic PID Logic ---
  float distance = readUltrasonic();

  float error = setpoint - distance;
  integral += error;
  float derivative = error - prevError;
  float output = (Kp * error) + (Ki * integral) + (Kd * derivative);
  prevError = error;

  int pwm = constrain(abs(output) * 10, 40, 180);  // Reduced speed
  int balancedPWM = 100;                           // Gentle balanced mode

  switch (currentState) {
    case IDLE:
      nextState = BALANCED;
      break;

    case FILLING:
      if (distance <= setpoint - tolerance + hysteresis)
        nextState = BALANCED;
      else
        nextState = FILLING;
      break;

    case DRAINING:
      if (distance >= setpoint + tolerance - hysteresis)
        nextState = BALANCED;
      else
        nextState = DRAINING;
      break;

    case BALANCED:
      if (distance > setpoint + tolerance + hysteresis)
        nextState = FILLING;
      else if (distance < setpoint - tolerance - hysteresis)
        nextState = DRAINING;
      else
        nextState = BALANCED;
      break;
  }

  // --- Debounce Logic ---
  if (nextState == currentState) {
    stableCount = 0;
  } else {
    stableCount++;
    if (stableCount >= debounceLimit) {
      currentState = nextState;
      stableCount = 0;
    }
  }

  // --- Display PID Values ---
  Serial.print("Distance: ");
  Serial.print(distance, 2);
  Serial.print(" cm | Error: ");
  Serial.print(error, 2);
  Serial.print(" | PWM: ");
  Serial.print(pwm);
  Serial.print(" | State: ");

  if (currentState == FILLING) {
    runInletMotor(pwm);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    Serial.println("FILLING tank...");
  } 
  else if (currentState == DRAINING) {
    runOutletMotor(pwm);
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    Serial.println("DRAINING tank...");
  } 
  else if (currentState == BALANCED) {
    runInletMotor(balancedPWM);
    runOutletMotor(balancedPWM);
    Serial.println("BALANCED — both motors ON (opposite directions).");
  }

  // ===========================
  // --- Flow Sensor Logic (1 sec interval) ---
  unsigned long currentTime = millis();
  if (currentTime - lastFlowTime > 1000) {
    detachInterrupt(digitalPinToInterrupt(19));
    detachInterrupt(digitalPinToInterrupt(32));

    // YF-S201: ~450 pulses per liter
    flowRate1 = (pulseCount1 / 450.0) * 60.0; // L/min
    flowRate2 = (pulseCount2 / 450.0) * 60.0; // L/min

    Serial.print("Flow 1 (GPIO19): ");
    Serial.print(flowRate1);
    Serial.print(" L/min   ");
    Serial.print("Flow 2 (GPIO32): ");
    Serial.print(flowRate2);
    Serial.println(" L/min");

    pulseCount1 = 0;
    pulseCount2 = 0;
    lastFlowTime = currentTime;

    attachInterrupt(digitalPinToInterrupt(19), pulseCounter1, RISING);
    attachInterrupt(digitalPinToInterrupt(32), pulseCounter2, RISING);
  }

  delay(500);
}
