#include <Servo.h>

int waterSensor = A0;
int rainSensor = A1;

int redLED = 13;
int greenLED = 12;
int buzzer = 8;

Servo gateServo;

void setup() {
  Serial.begin(9600);

  gateServo.attach(9);
  gateServo.write(0);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  digitalWrite(buzzer, LOW);
}

void loop() {

  int waterValue = analogRead(waterSensor);
  int rainValue = analogRead(rainSensor);

  Serial.print("Water: ");
  Serial.print(waterValue);
  Serial.print(" | Level: ");

  if (waterValue < 200) {

    Serial.print("LOW");
    gateServo.write(0);

    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(buzzer, LOW);

  }
  else if (waterValue < 400) {

    Serial.print("MEDIUM");
    gateServo.write(0);

    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(buzzer, LOW);

  }
  else {

    Serial.print("HIGH");
    gateServo.write(180);

    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(buzzer, HIGH);

  }

  Serial.print(" | Rain Sensor: ");
  Serial.print(rainValue);

  Serial.print(" | Rain: ");

  if (rainValue > 110) {
    Serial.println("RAIN");
  }
  else {
    Serial.println("NO RAIN");
  }

  delay(500);
}