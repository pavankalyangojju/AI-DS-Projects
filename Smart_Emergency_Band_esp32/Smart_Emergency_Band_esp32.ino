#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =====================================================
// I2C
// =====================================================
#define SDA_PIN 21
#define SCL_PIN 22

// =====================================================
// MPU6050
// =====================================================
#define MPU_ADDR 0x68

#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B

// =====================================================
// DHT11
// =====================================================
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// PULSE SENSOR
// =====================================================
#define PULSE_PIN 34

int pulseValue = 0;
int pulseThreshold = 2000;

bool beatDetected = false;

unsigned long lastBeatTime = 0;

int bpm = 0;

// =====================================================
// MPU VALUES
// =====================================================
int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;

float gx, gy, gz;

// =====================================================
// DHT VALUES
// =====================================================
float temperature = 0;
float humidity = 0;

// =====================================================
// GYRO FALL DETECTION
// =====================================================

// Gyroscope magnitude
float gyroMagnitude = 0;

// Adjust this value according to your sensor
float gyroFallThreshold = 250.0;

bool possibleFall = false;
bool fallDetected = false;

unsigned long fallStartTime = 0;


// =====================================================
// WRITE MPU REGISTER
// =====================================================
void writeMPU(byte reg, byte data) {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}


// =====================================================
// READ MPU
// =====================================================
void readMPU() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14);

  if (Wire.available() == 14) {

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();

    // Skip temperature
    Wire.read();
    Wire.read();

    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();
  }

  // Gyroscope conversion
  gx = GyX / 131.0;
  gy = GyY / 131.0;
  gz = GyZ / 131.0;

  // Calculate total gyro movement
  gyroMagnitude = sqrt(
    (gx * gx) +
    (gy * gy) +
    (gz * gz)
  );
}


// =====================================================
// GYRO FALL DETECTION
// =====================================================
void detectFall() {

  // Detect sudden rotational movement
  if (gyroMagnitude > gyroFallThreshold &&
      !possibleFall &&
      !fallDetected) {

    possibleFall = true;

    fallStartTime = millis();

    Serial.println();
    Serial.println("POSSIBLE FALL DETECTED!");
  }


  // Confirm after sudden movement
  if (possibleFall) {

    // If another strong movement occurs
    if (gyroMagnitude > gyroFallThreshold) {

      fallDetected = true;

      possibleFall = false;

      fallStartTime = millis();

      Serial.println();
      Serial.println("********************************");
      Serial.println("       FALL DETECTED!");
      Serial.println("********************************");
    }


    // Cancel after 1 second
    if (millis() - fallStartTime > 1000) {

      possibleFall = false;
    }
  }


  // Keep fall message for 5 seconds
  if (fallDetected) {

    if (millis() - fallStartTime > 5000) {

      fallDetected = false;
    }
  }
}


// =====================================================
// HEART RATE
// =====================================================
void readHeartRate() {

  pulseValue = analogRead(PULSE_PIN);

  unsigned long currentTime = millis();

  if (pulseValue > pulseThreshold &&
      !beatDetected) {

    beatDetected = true;

    if (lastBeatTime > 0) {

      unsigned long beatInterval =
        currentTime - lastBeatTime;

      if (beatInterval >= 300 &&
          beatInterval <= 2000) {

        bpm = 60000 / beatInterval;
      }
    }

    lastBeatTime = currentTime;
  }


  if (pulseValue < pulseThreshold) {

    beatDetected = false;
  }


  if (bpm < 30 || bpm > 220) {

    bpm = 0;
  }
}


// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  delay(500);


  // ===================================================
  // OLED
  // ===================================================

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR)) {

    Serial.println("OLED NOT FOUND!");

    while (1);
  }


  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SMART EMERGENCY BAND");

  display.setCursor(0, 20);
  display.println("Starting sensors...");

  display.display();

  delay(1500);


  // ===================================================
  // MPU6050
  // ===================================================

  writeMPU(PWR_MGMT_1, 0x00);

  delay(100);

  Serial.println();
  Serial.println("SMART EMERGENCY BAND");
  Serial.println("--------------------");

  Serial.println("MPU6050 initialized");


  // ===================================================
  // DHT11
  // ===================================================

  dht.begin();

  Serial.println("DHT11 initialized");


  // ===================================================
  // PULSE SENSOR
  // ===================================================

  pinMode(PULSE_PIN, INPUT);

  analogReadResolution(12);

  Serial.println("Pulse sensor initialized");

  delay(2000);
}


// =====================================================
// LOOP
// =====================================================
void loop() {

  // MPU
  readMPU();

  // Gyroscope fall detection
  detectFall();

  // DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Heart rate
  readHeartRate();


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.print("Temp: ");

  if (!isnan(temperature)) {
    Serial.print(temperature, 1);
    Serial.print(" C");
  }
  else {
    Serial.print("ERROR");
  }


  Serial.print(" | BPM: ");
  Serial.print(bpm);


  Serial.print(" | GYRO X: ");
  Serial.print(gx, 1);

  Serial.print(" Y: ");
  Serial.print(gy, 1);

  Serial.print(" Z: ");
  Serial.print(gz, 1);


  Serial.print(" | GYRO MAG: ");
  Serial.print(gyroMagnitude, 1);


  Serial.print(" | Status: ");

  if (fallDetected) {

    Serial.println("FALL DETECTED!");

  }
  else if (possibleFall) {

    Serial.println("POSSIBLE FALL");

  }
  else {

    Serial.println("SAFE");
  }


  // ===================================================
  // OLED
  // ===================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);


  // Title
  display.setCursor(0, 0);
  display.println("SMART EMERGENCY BAND");

  display.drawLine(
    0, 10,
    127, 10,
    SSD1306_WHITE
  );


  // Temperature
  display.setCursor(0, 14);

  display.print("Temp: ");

  if (!isnan(temperature)) {

    display.print(temperature, 1);
    display.println(" C");

  }
  else {

    display.println("ERROR");
  }


  // Heart
  display.setCursor(0, 25);

  display.print("Heart: ");

  if (bpm > 0) {

    display.print(bpm);
    display.println(" BPM");

  }
  else {

    display.println("-- BPM");
  }


  // Gyroscope
  display.setCursor(0, 36);

  display.print("GX:");
  display.print(gx, 0);

  display.print(" GY:");
  display.print(gy, 0);


  display.setCursor(0, 46);

  display.print("GZ:");
  display.print(gz, 0);


  // Status
  display.setCursor(0, 57);

  if (fallDetected) {

    display.print("FALL DETECTED!");

  }
  else if (possibleFall) {

    display.print("CHECK MOVEMENT");

  }
  else {

    display.print("STATUS: SAFE");

  }


  display.display();


  // Fast sampling for movement detection
  delay(500);
}