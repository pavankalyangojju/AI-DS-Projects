#include <Servo.h>

#define TRIG_PIN 9
#define ECHO_PIN 10
#define SERVO_PIN 6
#define BUZZER_PIN 7

Servo lidServo;

long duration;
int distance;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lidServo.attach(SERVO_PIN);

  // Start with lid closed
  lidServo.write(0);

  Serial.begin(9600);
}

void loop() {

  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  // Read echo
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate distance
  distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // If hand/object is detected
  if (distance > 0 && distance <= 20) {

    // Open lid
    lidServo.write(90);

    // Beep
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    // Keep lid open
    delay(3000);

    // Close lid
    lidServo.write(0);

    delay(1000);
  }

  delay(100);
}