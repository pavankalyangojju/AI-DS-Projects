#include <Servo.h>

// =====================================================
// MOTOR DRIVER - L298N
// =====================================================

#define ENA 10
#define IN1 9
#define IN2 8

#define ENB 5
#define IN3 7
#define IN4 6

// =====================================================
// FLAME SENSORS
// =====================================================

#define FLAME_LEFT   A0
#define FLAME_CENTER A1
#define FLAME_RIGHT  A2

// =====================================================
// WATER PUMP RELAY
// =====================================================

#define RELAY_PIN 4

// =====================================================
// SERVO
// =====================================================

#define SERVO_PIN 3

Servo waterServo;

// =====================================================
// SETTINGS
// =====================================================

int motorSpeed = 200;

// No fire ≈ 1023
// Fire gives lower value
int fireThreshold = 900;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Flame sensors
  pinMode(FLAME_LEFT, INPUT);
  pinMode(FLAME_CENTER, INPUT);
  pinMode(FLAME_RIGHT, INPUT);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);

  // Pump OFF
  digitalWrite(RELAY_PIN, HIGH);

  // Servo
  waterServo.attach(SERVO_PIN);
  waterServo.write(90);

  Serial.begin(9600);

  stopMotors();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  int leftValue   = analogRead(FLAME_LEFT);
  int centerValue = analogRead(FLAME_CENTER);
  int rightValue  = analogRead(FLAME_RIGHT);

  Serial.print("LEFT: ");
  Serial.print(leftValue);

  Serial.print(" | CENTER: ");
  Serial.print(centerValue);

  Serial.print(" | RIGHT: ");
  Serial.println(rightValue);


  // ===================================================
  // NO FIRE
  // ===================================================

  if (leftValue >= fireThreshold &&
      centerValue >= fireThreshold &&
      rightValue >= fireThreshold) {

    Serial.println("NO FIRE");

    stopMotors();

    // Pump OFF
    digitalWrite(RELAY_PIN, HIGH);

    // Servo center
    waterServo.write(90);
  }


  // ===================================================
  // FIRE ON LEFT
  // ===================================================

  else if (leftValue < fireThreshold &&
           leftValue < centerValue &&
           leftValue < rightValue) {

    Serial.println("FIRE LEFT");

    // Turn toward fire
    turnLeft();

    // Pump ON
    digitalWrite(RELAY_PIN, LOW);

    // Point nozzle left
    waterServo.write(45);
  }


  // ===================================================
  // FIRE ON RIGHT
  // ===================================================

  else if (rightValue < fireThreshold &&
           rightValue < leftValue &&
           rightValue < centerValue) {

    Serial.println("FIRE RIGHT");

    // Turn toward fire
    turnRight();

    // Pump ON
    digitalWrite(RELAY_PIN, LOW);

    // Point nozzle right
    waterServo.write(135);
  }


  // ===================================================
  // FIRE IN CENTER
  // ===================================================

  else if (centerValue < fireThreshold) {

    Serial.println("FIRE CENTER");

    // Move toward fire
    forward();

    // Pump ON
    digitalWrite(RELAY_PIN, LOW);

    // Start sweeping nozzle
    sweepWater();
  }

  delay(100);
}


// =====================================================
// MOVE FORWARD
// =====================================================

void forward() {

  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


// =====================================================
// TURN LEFT
// =====================================================

void turnLeft() {

  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


// =====================================================
// TURN RIGHT
// =====================================================

void turnRight() {

  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}


// =====================================================
// STOP MOTORS
// =====================================================

void stopMotors() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =====================================================
// WATER NOZZLE SWEEP
// =====================================================

void sweepWater() {

  // Sweep left
  for (int angle = 60; angle <= 120; angle += 5) {

    waterServo.write(angle);
    delay(40);
  }

  // Sweep right
  for (int angle = 120; angle >= 60; angle -= 5) {

    waterServo.write(angle);
    delay(40);
  }
}