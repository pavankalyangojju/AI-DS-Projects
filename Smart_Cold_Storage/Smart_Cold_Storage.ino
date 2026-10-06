#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =====================================================
// DHT11
// =====================================================
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// MQ-135
// =====================================================
#define MQ135_PIN 34

// =====================================================
// LEDs
// =====================================================
#define GREEN_LED 15
#define RED_LED 2

// =====================================================
// RELAY
// =====================================================
#define RELAY_PIN 26

// Most relay modules are ACTIVE LOW
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// =====================================================
// Temperature limit
// =====================================================
#define TEMP_LIMIT 8.0

// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);

  // I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // DHT
  dht.begin();

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Relay
  pinMode(RELAY_PIN, OUTPUT);

  // Initial state
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED not found!");

    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(20, 10);
  display.println("SMART COLD");

  display.setCursor(25, 25);
  display.println("STORAGE");

  display.setCursor(30, 45);
  display.println("SYSTEM");

  display.display();

  delay(2000);
}

// =====================================================
// Loop
// =====================================================
void loop() {

  // Read DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read MQ-135
  int mq135Value = analogRead(MQ135_PIN);

  // ===================================================
  // Check DHT11
  // ===================================================
  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("DHT11 reading failed!");

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(10, 20);
    display.println("DHT11 SENSOR");

    display.setCursor(30, 40);
    display.println("ERROR!");

    display.display();

    delay(2000);

    return;
  }

  // ===================================================
  // Temperature Control
  // ===================================================

  String status;
  String fanStatus;

  if (temperature <= TEMP_LIMIT) {

    // =========================
    // SAFE
    // =========================

    status = "SAFE";
    fanStatus = "OFF";

    // Green ON
    digitalWrite(GREEN_LED, HIGH);

    // Red OFF
    digitalWrite(RED_LED, LOW);

    // Relay OFF
    digitalWrite(RELAY_PIN, RELAY_OFF);
  }

  else {

    // =========================
    // DANGER
    // =========================

    status = "DANGER";
    fanStatus = "ON";

    // Green OFF
    digitalWrite(GREEN_LED, LOW);

    // Red ON
    digitalWrite(RED_LED, HIGH);

    // Relay ON
    digitalWrite(RELAY_PIN, RELAY_ON);
  }

  // ===================================================
  // Serial Monitor
  // ===================================================

  Serial.println("============================");

  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("MQ-135      : ");
  Serial.println(mq135Value);

  Serial.print("Status      : ");
  Serial.println(status);

  Serial.print("Fan         : ");
  Serial.println(fanStatus);

  // ===================================================
  // OLED Display
  // ===================================================

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SMART COLD STORAGE");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Temperature
  display.setCursor(0, 16);
  display.print("Temp : ");
  display.print(temperature, 1);
  display.println(" C");

  // Humidity
  display.setCursor(0, 28);
  display.print("Hum  : ");
  display.print(humidity, 1);
  display.println(" %");

  // MQ-135
  display.setCursor(0, 40);
  display.print("Air  : ");
  display.println(mq135Value);

  // Status
  display.setCursor(0, 52);
  display.print("Status: ");
  display.println(status);

  display.display();

  delay(2000);
}