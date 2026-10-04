#include <Wire.h>
#include <WiFi.h>
#include <MPU6050.h>

#define led 2

MPU6050 mpu;

const char* ssid = "ESP32_Car";
const char* password = "12345678";
const char* host = "192.168.4.1"; // Receiver IP

WiFiClient client;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  pinMode(led,OUTPUT);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(led, LOW);
  }
  Serial.println("Connected!");
  digitalWrite(led, HIGH);
}

void loop() {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  String command = "";

  if (ay > 8000) command = "R";       // Tilt forward
  else if (ay < -8000) command = "L"; // Tilt backward
  else if (ax > 8000) command = "B";  // Tilt right
  else if (ax < -8000) command = "F"; // Tilt left
  else command = "S";                 // Stop

  if (client.connect(host, 80)) {
    client.println(command);
    client.stop();
  }

  delay(200);
}
