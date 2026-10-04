#define BLYNK_TEMPLATE_ID "TMPL3XiDicJ96"
#define BLYNK_TEMPLATE_NAME "Irregation System"
#define BLYNK_AUTH_TOKEN "z1SKdxiWCwZS7747BGh6_M8vNbRWQqUy"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ================================
// Wi-Fi Credentials
// ================================
char ssid[] = "NTRguna";
char pass[] = "Guna@8688";

// ================================
// Pin Connections
// ================================
#define SOIL_SENSOR_PIN 34
#define RELAY_PIN 26

// ================================
// Soil Moisture Threshold
// ================================
#define DRY_THRESHOLD 2500

// ================================
// Blynk Timer
// ================================
BlynkTimer timer;

// ================================
// Motor Status
// ================================
bool motorStatus = false;


// ==========================================
// Read Soil Moisture and Send to Blynk
// ==========================================
void sendSensorData()
{
  int moistureValue = analogRead(SOIL_SENSOR_PIN);

  Serial.print("Raw Soil Value: ");
  Serial.println(moistureValue);

  // Convert sensor value to percentage
  int moisturePercent = map(moistureValue, 4095, 0, 0, 100);

  // Keep percentage between 0 and 100
  moisturePercent = constrain(moisturePercent, 0, 100);

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  // Send soil moisture to Blynk
  Blynk.virtualWrite(V0, moisturePercent);


  // ==========================================
  // AUTOMATIC MOTOR CONTROL
  // ==========================================

  if (moistureValue > DRY_THRESHOLD)
  {
    // ======================================
    // SOIL IS DRY
    // ======================================

    Serial.println("Soil is DRY");
    Serial.println("Motor ON");

    // Relay ON
    digitalWrite(RELAY_PIN, LOW);

    motorStatus = true;

    // Numeric status
    Blynk.virtualWrite(V1, 1);

    // Text status
    Blynk.virtualWrite(V2, "ON");
  }
  else
  {
    // ======================================
    // SOIL IS MOIST
    // ======================================

    Serial.println("Soil is MOIST");
    Serial.println("Motor OFF");

    // Relay OFF
    digitalWrite(RELAY_PIN, HIGH);

    motorStatus = false;

    // Numeric status
    Blynk.virtualWrite(V1, 0);

    // Text status
    Blynk.virtualWrite(V2, "OFF");
  }

  Serial.println("----------------------------");
}


// ==========================================
// Setup
// ==========================================
void setup()
{
  Serial.begin(115200);

  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Motor OFF at startup
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Automatic Plant Watering System");
  Serial.println("Connecting to Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Update every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}


// ==========================================
// Main Loop
// ==========================================
void loop()
{
  Blynk.run();
  timer.run();
}