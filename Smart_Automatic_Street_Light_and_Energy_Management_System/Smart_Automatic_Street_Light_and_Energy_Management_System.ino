//arduino uno code

int ldrPin = A0;
int pirPin = 2;

int led1 = 8;
int led2 = 9;
int led3 = 10;
int led4 = 11;

int currentPin = A1;
int voltagePin = A2;

float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;

unsigned long previousTime = 0;

int ldrThreshold = 500;

void setup() {

  pinMode(ldrPin, INPUT);
  pinMode(pirPin, INPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);

  Serial.begin(9600);

  previousTime = millis();

  Serial.println("=================================");
  Serial.println(" SMART AUTOMATIC STREET LIGHT");
  Serial.println(" ENERGY MANAGEMENT SYSTEM");
  Serial.println("=================================");
  Serial.println("PIR warming up...");
  
  delay(5000);

  Serial.println("System Ready!");
  Serial.println();
}

void loop() {

  int ldrValue = analogRead(ldrPin);
  int pirValue = digitalRead(pirPin);

  // DAY / NIGHT DETECTION
  bool night;

  if (ldrValue < ldrThreshold) {
    night = true;
  }
  else {
    night = false;
  }

  // STREET LIGHT CONTROL
  if (night && pirValue == HIGH) {

    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);

  }
  else {

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
  }

  // VOLTAGE
  int voltageRaw = analogRead(voltagePin);

  voltage = voltageRaw * (5.0 / 1023.0) * 5.0;

  // ACS712 CURRENT
  int currentRaw = analogRead(currentPin);

  float sensorVoltage = currentRaw * (5.0 / 1023.0);

  current = (sensorVoltage - 2.5) / 0.185;

  if (current < 0) {
    current = -current;
  }

  if (current < 0.05) {
    current = 0;
  }

  // POWER
  power = voltage * current;

  // ENERGY
  unsigned long currentTime = millis();

  float elapsedTime =
    (currentTime - previousTime) / 3600000.0;

  if (digitalRead(led1) == HIGH) {
    energy = energy + (power * elapsedTime);
  }

  previousTime = currentTime;

  // SERIAL MONITOR
  Serial.println("--------------------------------");

  Serial.print("LDR Value      : ");
  Serial.println(ldrValue);

  Serial.print("TIME           : ");

  if (night) {
    Serial.println("NIGHT TIME");
  }
  else {
    Serial.println("DAY TIME");
  }

  Serial.print("Motion         : ");

  if (pirValue == HIGH) {
    Serial.println("DETECTED");
  }
  else {
    Serial.println("NO MOTION");
  }

  Serial.print("LED 1          : ");
  Serial.println(digitalRead(led1) ? "ON" : "OFF");

  Serial.print("LED 2          : ");
  Serial.println(digitalRead(led2) ? "ON" : "OFF");

  Serial.print("LED 3          : ");
  Serial.println(digitalRead(led3) ? "ON" : "OFF");

  Serial.print("LED 4          : ");
  Serial.println(digitalRead(led4) ? "ON" : "OFF");

  Serial.print("Voltage        : ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  Serial.print("Current        : ");
  Serial.print(current, 2);
  Serial.println(" A");

  Serial.print("Power          : ");
  Serial.print(power, 2);
  Serial.println(" W");

  Serial.print("Energy         : ");
  Serial.print(energy, 4);
  Serial.println(" Wh");

  Serial.println("--------------------------------");

  delay(500);
}