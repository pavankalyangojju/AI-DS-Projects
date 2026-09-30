#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

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
// DS18B20
// =====================================================
#define DS18B20_PIN 4

OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);

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

// Automatic pulse detection
float pulseBaseline = 0;
float pulseThreshold = 0;

bool beatDetected = false;

unsigned long lastBeatTime = 0;

int bpm = 0;

// =====================================================
// MPU VALUES
// =====================================================
int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;

float gx = 0;
float gy = 0;
float gz = 0;

// =====================================================
// TEMPERATURE
// =====================================================
float temperature = 0;

// =====================================================
// GYROSCOPE FALL DETECTION
// =====================================================
float gyroMagnitude = 0;

float gyroFallThreshold = 250.0;

bool possibleFall = false;
bool fallDetected = false;

unsigned long fallStartTime = 0;

// =====================================================
// TIMERS
// =====================================================
unsigned long lastPulseSample = 0;
unsigned long lastTemperatureRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialUpdate = 0;

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
// READ MPU6050
// =====================================================
void readMPU() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14);

  if (Wire.available() == 14) {

    AcX = (Wire.read() << 8) | Wire.read();
    AcY = (Wire.read() << 8) | Wire.read();
    AcZ = (Wire.read() << 8) | Wire.read();

    // Skip MPU6050 temperature
    Wire.read();
    Wire.read();

    GyX = (Wire.read() << 8) | Wire.read();
    GyY = (Wire.read() << 8) | Wire.read();
    GyZ = (Wire.read() << 8) | Wire.read();
  }

  // Convert gyro raw values to degrees/second
  gx = GyX / 131.0;
  gy = GyY / 131.0;
  gz = GyZ / 131.0;

  // Gyroscope magnitude
  gyroMagnitude = sqrt(
    (gx * gx) +
    (gy * gy) +
    (gz * gz)
  );
}

// =====================================================
// FALL DETECTION USING ONLY GYROSCOPE
// =====================================================
void detectFall() {

  // First sudden rotational movement
  if (gyroMagnitude > gyroFallThreshold &&
      !possibleFall &&
      !fallDetected) {

    possibleFall = true;

    fallStartTime = millis();

    Serial.println();
    Serial.println("POSSIBLE FALL DETECTED!");
  }

  // Confirm fall if strong movement continues
  if (possibleFall) {

    if (gyroMagnitude > gyroFallThreshold) {

      fallDetected = true;

      possibleFall = false;

      fallStartTime = millis();

      Serial.println();
      Serial.println("********************************");
      Serial.println("       FALL DETECTED!");
      Serial.println("********************************");
    }

    // Cancel possible fall after 1 second
    if (millis() - fallStartTime > 1000) {

      possibleFall = false;
    }
  }

  // Keep fall status for 5 seconds
  if (fallDetected) {

    if (millis() - fallStartTime > 5000) {

      fallDetected = false;
    }
  }
}

// =====================================================
// INITIALIZE PULSE BASELINE
// =====================================================
void calibratePulseSensor() {

  Serial.println();
  Serial.println("Calibrating pulse sensor...");
  Serial.println("Place finger on sensor and keep still.");

  long total = 0;

  const int samples = 300;

  for (int i = 0; i < samples; i++) {

    int value = analogRead(PULSE_PIN);

    total += value;

    delay(5);
  }

  pulseBaseline = total / (float)samples;

  // Dynamic threshold
  pulseThreshold = pulseBaseline + 80;

  Serial.print("Pulse baseline: ");
  Serial.println(pulseBaseline);

  Serial.print("Pulse threshold: ");
  Serial.println(pulseThreshold);

  Serial.println("Pulse sensor ready.");
}

// =====================================================
// READ HEART RATE
// =====================================================
void readHeartRate() {

  unsigned long currentTime = micros();

  // Sample every 10 ms
  if (currentTime - lastPulseSample < 10000) {
    return;
  }

  lastPulseSample = currentTime;

  pulseValue = analogRead(PULSE_PIN);

  // Slowly update baseline
  pulseBaseline =
    (pulseBaseline * 0.995) +
    (pulseValue * 0.005);

  // Dynamic threshold
  pulseThreshold = pulseBaseline + 60;

  unsigned long now = millis();

  // ===================================================
  // BEAT DETECTION
  // ===================================================
  if (pulseValue > pulseThreshold &&
      !beatDetected) {

    beatDetected = true;

    if (lastBeatTime > 0) {

      unsigned long beatInterval =
        now - lastBeatTime;

      // Valid heartbeat interval
      if (beatInterval >= 300 &&
          beatInterval <= 2000) {

        int newBPM =
          60000 / beatInterval;

        // Basic smoothing
        if (bpm == 0) {

          bpm = newBPM;

        }
        else {

          bpm =
            (bpm * 0.7) +
            (newBPM * 0.3);
        }
      }
    }

    lastBeatTime = now;
  }

  // Reset beat detection
  if (pulseValue < pulseBaseline) {

    beatDetected = false;
  }

  // If no beat for 3 seconds
  if (lastBeatTime > 0 &&
      now - lastBeatTime > 3000) {

    bpm = 0;
  }

  // Safety limits
  if (bpm < 30 || bpm > 220) {

    bpm = 0;
  }
}

// =====================================================
// READ DS18B20
// =====================================================
void readTemperature() {

  if (millis() - lastTemperatureRead < 1000) {
    return;
  }

  lastTemperatureRead = millis();

  ds18b20.requestTemperatures();

  float temp =
    ds18b20.getTempCByIndex(0);

  if (temp != DEVICE_DISCONNECTED_C) {

    temperature = temp;
  }
}

// =====================================================
// OLED
// =====================================================
void updateOLED() {

  if (millis() - lastDisplayUpdate < 200) {
    return;
  }

  lastDisplayUpdate = millis();

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // ===================================================
  // TITLE
  // ===================================================

  display.setCursor(0, 0);

  display.println("SMART EMERGENCY BAND");

  display.drawLine(
    0, 10,
    127, 10,
    SSD1306_WHITE
  );

  // ===================================================
  // TEMPERATURE
  // ===================================================

  display.setCursor(0, 14);

  display.print("Temp: ");

  if (temperature != DEVICE_DISCONNECTED_C) {

    display.print(temperature, 1);
    display.println(" C");

  }
  else {

    display.println("ERROR");
  }

  // ===================================================
  // HEART RATE
  // ===================================================

  display.setCursor(0, 25);

  display.print("Heart: ");

  if (bpm > 0) {

    display.print(bpm);
    display.println(" BPM");

  }
  else {

    display.println("-- BPM");
  }

  // ===================================================
  // GYROSCOPE
  // ===================================================

  display.setCursor(0, 36);

  display.print("GX:");
  display.print(gx, 0);

  display.print(" GY:");
  display.print(gy, 0);

  display.setCursor(0, 46);

  display.print("GZ:");
  display.print(gz, 0);

  // ===================================================
  // STATUS
  // ===================================================

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
}

// =====================================================
// SERIAL MONITOR
// =====================================================
void updateSerial() {

  if (millis() - lastSerialUpdate < 500) {
    return;
  }

  lastSerialUpdate = millis();

  Serial.print("Temp: ");

  if (temperature != DEVICE_DISCONNECTED_C) {

    Serial.print(temperature, 1);
    Serial.print(" C");

  }
  else {

    Serial.print("ERROR");
  }

  Serial.print(" | BPM: ");
  Serial.print(bpm);

  Serial.print(" | Pulse: ");
  Serial.print(pulseValue);

  Serial.print(" | Threshold: ");
  Serial.print(pulseThreshold);

  Serial.print(" | GX: ");
  Serial.print(gx, 1);

  Serial.print(" | GY: ");
  Serial.print(gy, 1);

  Serial.print(" | GZ: ");
  Serial.print(gz, 1);

  Serial.print(" | MAG: ");
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
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

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

  writeMPU(
    PWR_MGMT_1,
    0x00
  );

  delay(100);

  Serial.println();
  Serial.println("==============================");
  Serial.println("   SMART EMERGENCY BAND");
  Serial.println("==============================");

  Serial.println("MPU6050 initialized");

  // ===================================================
  // DS18B20
  // ===================================================

  ds18b20.begin();

  Serial.println("DS18B20 initialized");

  // ===================================================
  // PULSE SENSOR
  // ===================================================

  pinMode(
    PULSE_PIN,
    INPUT
  );

  analogReadResolution(12);

  Serial.println("Pulse sensor initialized");

  // ===================================================
  // PULSE CALIBRATION
  // ===================================================

  calibratePulseSensor();

  Serial.println();
  Serial.println("System Ready!");

  delay(1000);
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ===================================================
  // HEART RATE
  // ===================================================

  // Run continuously
  readHeartRate();

  // ===================================================
  // MPU6050
  // ===================================================

  readMPU();

  // ===================================================
  // FALL DETECTION
  // ===================================================

  detectFall();

  // ===================================================
  // TEMPERATURE
  // ===================================================

  readTemperature();

  // ===================================================
  // OUTPUTS
  // ===================================================

  updateSerial();

  updateOLED();

  // NO delay here
}