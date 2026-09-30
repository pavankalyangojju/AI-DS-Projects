#include <Servo.h>

// =====================================================
// Ultrasonic Sensor
// =====================================================
#define TRIG_PIN 9
#define ECHO_PIN 10

// =====================================================
// L298N Motor Driver
// =====================================================
#define IN1 2
#define IN2 3

#define IN3 4
#define IN4 5

// =====================================================
// Servo
// =====================================================
#define SERVO_PIN 6

Servo scanServo;

// =====================================================
// Settings
// =====================================================
#define OBSTACLE_DISTANCE 20

// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(9600);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Motors
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Servo
  scanServo.attach(SERVO_PIN);

  // Start facing forward
  scanServo.write(90);

  stopCar();

  delay(1000);

  Serial.println("Object Avoiding Car Ready!");
}


// =====================================================
// Main Loop
// =====================================================
void loop() {

  // Look forward
  scanServo.write(90);
  delay(150);

  long frontDistance = getDistance();

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.println(" cm");


  // ===================================================
  // No obstacle
  // ===================================================

  if (frontDistance > OBSTACLE_DISTANCE) {

    forward();
  }


  // ===================================================
  // Obstacle detected
  // ===================================================

  else {

    stopCar();
    delay(300);

    // Move backward
    backward();
    delay(400);

    stopCar();
    delay(300);


    // =================================================
    // Check LEFT
    // =================================================

    scanServo.write(150);
    delay(500);

    long leftDistance = getDistance();

    Serial.print("Left: ");
    Serial.print(leftDistance);
    Serial.println(" cm");


    // =================================================
    // Check RIGHT
    // =================================================

    scanServo.write(30);
    delay(500);

    long rightDistance = getDistance();

    Serial.print("Right: ");
    Serial.print(rightDistance);
    Serial.println(" cm");


    // Return to center
    scanServo.write(90);
    delay(300);


    // =================================================
    // Decide direction
    // =================================================

    if (leftDistance > rightDistance) {

      Serial.println("Turning LEFT");

      turnLeft();
      delay(600);
    }

    else {

      Serial.println("Turning RIGHT");

      turnRight();
      delay(600);
    }

    stopCar();
    delay(200);
  }

  delay(50);
}


// =====================================================
// Ultrasonic Distance
// =====================================================

long getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 400;
  }

  long distance = duration * 0.034 / 2;

  return distance;
}


// =====================================================
// FORWARD
// =====================================================

void forward() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


// =====================================================
// BACKWARD
// =====================================================

void backward() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}


// =====================================================
// STOP
// =====================================================

void stopCar() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =====================================================
// TURN LEFT
// =====================================================

void turnLeft() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}


// =====================================================
// TURN RIGHT
// =====================================================

void turnRight() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}