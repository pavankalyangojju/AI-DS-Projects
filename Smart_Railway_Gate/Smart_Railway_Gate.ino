#include <Servo.h>

#define IR_ENTRY 2
#define IR_EXIT 3

#define RED_LED 11
#define GREEN_LED 13

#define BUZZER 6

#define SERVO_PIN 9

Servo gateServo;

int OPEN_POSITION = 90;
int CLOSE_POSITION = 0;

bool trainDetected = false;

void setup() {

  Serial.begin(9600);

  pinMode(IR_ENTRY, INPUT);
  pinMode(IR_EXIT, INPUT);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  // Attach servo
  gateServo.attach(SERVO_PIN);

  // Start with gate OPEN
  gateServo.write(OPEN_POSITION);

  // Green ON
  digitalWrite(GREEN_LED, HIGH);

  // Red OFF
  digitalWrite(RED_LED, LOW);

  // Buzzer OFF
  digitalWrite(BUZZER, LOW);

  Serial.println("Smart Railway Gate Started");
  Serial.println("Railway OPEN");
}

void loop() {

  int entrySensor = digitalRead(IR_ENTRY);
  int exitSensor = digitalRead(IR_EXIT);

  // =========================================
  // TRAIN ARRIVES
  // =========================================

  if (entrySensor == LOW && trainDetected == false) {

    Serial.println("TRAIN ARRIVED!");
    Serial.println("Gate Closing");

    // Red LED ON
    digitalWrite(RED_LED, HIGH);

    // Green LED OFF
    digitalWrite(GREEN_LED, LOW);

    // Buzzer ON
    digitalWrite(BUZZER, HIGH);

    // Close gate
    gateServo.attach(SERVO_PIN);
    gateServo.write(CLOSE_POSITION);

    trainDetected = true;

    // Buzzer sounds for 2 seconds
    delay(2000);

    // Buzzer OFF
    digitalWrite(BUZZER, LOW);

    Serial.println("Gate CLOSED");

    delay(500);
  }

  // =========================================
  // TRAIN PASSES EXIT
  // =========================================

  if (exitSensor == LOW && trainDetected == true) {

    Serial.println("TRAIN PASSED!");
    Serial.println("Gate Opening");

    // Red LED OFF
    digitalWrite(RED_LED, LOW);

    // Green LED ON
    digitalWrite(GREEN_LED, HIGH);

    // Buzzer ON
    digitalWrite(BUZZER, HIGH);

    // Open gate
    gateServo.write(OPEN_POSITION);

    delay(2000);

    // Buzzer OFF
    digitalWrite(BUZZER, LOW);

    Serial.println("Gate OPEN");

    // Turn servo control OFF
    gateServo.detach();

    Serial.println("Servo OFF");

    trainDetected = false;

    delay(1000);
  }

  delay(100);
}