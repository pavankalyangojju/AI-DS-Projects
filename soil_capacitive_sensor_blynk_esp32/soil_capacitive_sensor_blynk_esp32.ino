#define BLYNK_TEMPLATE_ID "TMPL3PtWX3Vh2"
#define BLYNK_TEMPLATE_NAME "soil"
#define BLYNK_AUTH_TOKEN "zEfEcfDwGAsyNo_Um5Ph4uqKRvzGPuEh"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =====================================================
// WIFI
// =====================================================

char ssid[] = "GOJJU";
char pass[] = "0987654321";

// =====================================================
// PINS
// =====================================================

#define SOIL_PIN    34      // Capacitive Sensor AO/AOUT
#define DHT_PIN     4
#define RELAY_PIN   26
#define BUZZER_PIN  25
#define RED_LED     2
#define GREEN_LED   15

#define DHT_TYPE DHT11

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// SOIL SENSOR CALIBRATION
// =====================================================
// Higher value = Dry
// Lower value  = Wet
//
// Change these values according to your sensor.
// =====================================================

#define CAP_DRY_VALUE 3000
#define CAP_WET_VALUE 1300

// Pump ON below this percentage
#define DRY_THRESHOLD 40

// =====================================================
// BLYNK TIMER
// =====================================================

BlynkTimer timer;


// =====================================================
// READ SOIL MOISTURE
// =====================================================

int getSoilMoisture(int rawValue) {

  int moisture = map(
    rawValue,
    CAP_DRY_VALUE,
    CAP_WET_VALUE,
    0,
    100
  );

  moisture = constrain(
    moisture,
    0,
    100
  );

  return moisture;
}


// =====================================================
// SEND DATA TO BLYNK
// =====================================================

void sendSensorData() {

  // ---------------------------------------------------
  // Soil Sensor
  // ---------------------------------------------------

  int soilRaw = analogRead(SOIL_PIN);

  int moisture = getSoilMoisture(soilRaw);


  // ---------------------------------------------------
  // DHT11
  // ---------------------------------------------------

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();


  // ---------------------------------------------------
  // Pump Control
  // ---------------------------------------------------

  bool soilIsDry = moisture < DRY_THRESHOLD;

  if (soilIsDry) {

    // Pump ON
    digitalWrite(RELAY_PIN, LOW);

    // Red LED ON
    digitalWrite(RED_LED, HIGH);

    // Green LED OFF
    digitalWrite(GREEN_LED, LOW);

    // Buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);

  } 
  else {

    // Pump OFF
    digitalWrite(RELAY_PIN, HIGH);

    // Red LED OFF
    digitalWrite(RED_LED, LOW);

    // Green LED ON
    digitalWrite(GREEN_LED, HIGH);

    // Buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);
  }


  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.println("-----------------------------");

  Serial.print("Raw Soil Value: ");
  Serial.println(soilRaw);

  Serial.print("Soil Moisture: ");
  Serial.print(moisture);
  Serial.println("%");

  if (!isnan(temperature)) {

    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.println(" C");

  } 
  else {

    Serial.println("Temperature: Sensor Error");
  }


  if (!isnan(humidity)) {

    Serial.print("Humidity: ");
    Serial.print(humidity, 1);
    Serial.println("%");

  } 
  else {

    Serial.println("Humidity: Sensor Error");
  }


  if (soilIsDry) {

    Serial.println("Soil Status: DRY");
    Serial.println("Water Pump: ON");

  } 
  else {

    Serial.println("Soil Status: MOIST");
    Serial.println("Water Pump: OFF");
  }


  // ===================================================
  // SEND TO BLYNK
  // ===================================================

  Blynk.virtualWrite(V0, moisture);

  Blynk.virtualWrite(V4, soilRaw);


  if (!isnan(temperature)) {
    Blynk.virtualWrite(V1, temperature);
  }

  if (!isnan(humidity)) {
    Blynk.virtualWrite(V2, humidity);
  }

  // Pump status
  if (soilIsDry) {
    Blynk.virtualWrite(V3, 1);
  } 
  else {
    Blynk.virtualWrite(V3, 0);
  }


  // ===================================================
  // OLED DISPLAY
  // ===================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Title
  display.setCursor(10, 0);
  display.println("SMART IRRIGATION");

  // Soil
  display.setCursor(0, 15);
  display.print("Soil: ");
  display.print(moisture);
  display.println("%");

  // Temperature
  display.setCursor(0, 27);
  display.print("Temp: ");

  if (isnan(temperature)) {
    display.println("Error");
  } 
  else {
    display.print(temperature, 1);
    display.println(" C");
  }

  // Humidity
  display.setCursor(0, 39);
  display.print("Humidity: ");

  if (isnan(humidity)) {
    display.println("Error");
  } 
  else {
    display.print(humidity, 1);
    display.println("%");
  }

  // Pump
  display.setCursor(0, 53);

  if (soilIsDry) {
    display.print("Pump: ON");
  } 
  else {
    display.print("Pump: OFF");
  }

  display.display();
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);


  // ===================================================
  // PIN MODES
  // ===================================================

  pinMode(SOIL_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);


  // ===================================================
  // INITIAL STATES
  // ===================================================

  // Pump OFF
  digitalWrite(RELAY_PIN, HIGH);

  // Buzzer OFF
  digitalWrite(BUZZER_PIN, LOW);

  // LEDs OFF
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);


  // ===================================================
  // I2C
  // ===================================================

  Wire.begin(21, 22);


  // ===================================================
  // OLED
  // ===================================================

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED not found!");

    while (1);
  }


  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(20, 25);
  display.println("SMART IRRIGATION");

  display.display();


  // ===================================================
  // DHT11
  // ===================================================

  dht.begin();


  // ===================================================
  // WIFI + BLYNK
  // ===================================================

  Serial.println("Connecting to WiFi...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  Serial.println("Blynk Connected!");


  // ===================================================
  // TIMER
  // ===================================================

  timer.setInterval(
    2000L,
    sendSensorData
  );
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  Blynk.run();

  timer.run();
}