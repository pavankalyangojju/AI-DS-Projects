#define GAS_SENSOR 34
#define FAN_PIN 26

int threshold = 260;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  Serial.println("ESP32 Gas Sensor Test");
}

void loop() {

  int gasValue = analogRead(GAS_SENSOR);

  Serial.print("Gas Sensor Value: ");
  Serial.println(gasValue);

  if (gasValue >= threshold) {

    digitalWrite(FAN_PIN, HIGH);

    Serial.println("WARNING: GAS DETECTED!");
    Serial.println("FAN: ON");

  } else {

    digitalWrite(FAN_PIN, LOW);

    Serial.println("Gas Level: Normal");
    Serial.println("FAN: OFF");
  }

  delay(1000);
}