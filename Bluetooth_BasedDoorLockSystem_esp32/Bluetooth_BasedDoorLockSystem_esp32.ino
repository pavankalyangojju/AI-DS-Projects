#include "BluetoothSerial.h"
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo doorServo;

#define SERVO_PIN 18
#define GREEN_LED 26
#define RED_LED 27
#define BUZZER 25

bool doorOpen = false;

void setup() {
  Serial.begin(115200);

  SerialBT.begin("ESP32_DoorLock");

  doorServo.attach(SERVO_PIN);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  doorServo.write(0);

  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);

  SerialBT.println("ESP32 Door Lock Ready");
  SerialBT.println("Send OPEN, CLOSE or STATUS");
}

void loop() {

  if (SerialBT.available()) {

    String command = SerialBT.readStringUntil('\n');
    command.trim();
    command.toUpperCase();

    if (command == "OPEN") {

      doorServo.write(90);

      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);

      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);

      doorOpen = true;

      SerialBT.println("DOOR UNLOCKED");
    }

    else if (command == "CLOSE") {

      doorServo.write(0);

      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);

      digitalWrite(BUZZER, HIGH);
      delay(200);
      digitalWrite(BUZZER, LOW);

      doorOpen = false;

      SerialBT.println("DOOR LOCKED");
    }

    else if (command == "STATUS") {

      if (doorOpen) {
        SerialBT.println("DOOR STATUS: UNLOCKED");
      } else {
        SerialBT.println("DOOR STATUS: LOCKED");
      }
    }

    else {
      SerialBT.println("INVALID COMMAND");
    }
  }
}