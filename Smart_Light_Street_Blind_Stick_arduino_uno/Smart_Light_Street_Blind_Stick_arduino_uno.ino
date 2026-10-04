int trigPin = 9;
int echoPin = 10;
int buzzer = 13;
int ldr = 7;
int button = 6;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(button, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {

  long duration;
  int distance;
  int lightStatus;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = duration * 0.034 / 2;

  lightStatus = digitalRead(ldr);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | LDR: ");

  if (lightStatus == HIGH) {
    Serial.println("DARK");
  }
  else {
    Serial.println("BRIGHT");
  }

  // Push Button
  if (digitalRead(button) == LOW) {

    digitalWrite(buzzer, HIGH);
    delay(10000);
    digitalWrite(buzzer, LOW);

  }

  // Object detected
  else if (distance > 0 && distance <= 20) {

    for (int i = 0; i < 2; i++) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
    }

  }

  // Dark surroundings
  else if (lightStatus == HIGH) {

    for (int i = 0; i < 3; i++) {
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
    }

  }

  delay(500);
}