#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ESP32Servo.h>

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_ADDR 0x3C
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =====================================================
// HC-SR04
// =====================================================
#define TRIG_PIN 5
#define ECHO_PIN 18

// =====================================================
// Sensors
// =====================================================
#define WATER_LEVEL_PIN 34
#define GAS_PIN         35
#define SOIL_PIN        32
#define LDR_PIN         33

// =====================================================
// DHT11
// =====================================================
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// Outputs
// =====================================================
#define LED_PIN    2
#define BUZZER_PIN 25
#define SERVO_PIN  13

Servo drainageServo;

// =====================================================
// Variables
// =====================================================
float waterDistance = 0;

int waterLevelValue = 0;
int gasValue = 0;
int soilValue = 0;
int ldrValue = 0;

float temperature = 0;
float humidity = 0;

// =====================================================
// Thresholds
// =====================================================
#define WATER_HIGH_DISTANCE   10
#define WATER_MEDIUM_DISTANCE 20

#define GAS_THRESHOLD 2000
#define SOIL_THRESHOLD 2500

// =====================================================
// OLED Page Control
// =====================================================
int oledPage = 0;

unsigned long lastOLEDUpdate = 0;

#define OLED_PAGE_TIME 2000

// =====================================================
// Ultrasonic Function
// =====================================================
float readDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return -1;
  }

  float distance = duration * 0.0343 / 2.0;

  return distance;
}

// =====================================================
// OLED HEADER
// =====================================================
void oledHeader()
{
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("SMART UNDERGROUND");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
}

// =====================================================
// OLED PAGE 1
// =====================================================
void showPage1()
{
  display.clearDisplay();

  oledHeader();

  display.setCursor(0, 17);
  display.print("Water Dist: ");

  if (waterDistance > 0)
  {
    display.print(waterDistance, 1);
    display.println(" cm");
  }
  else
  {
    display.println("ERROR");
  }

  display.setCursor(0, 29);
  display.print("Water Level: ");
  display.println(waterLevelValue);

  display.setCursor(0, 41);
  display.print("Gas MQ135: ");
  display.println(gasValue);

  display.setCursor(0, 53);
  display.println("Page 1 / 3");

  display.display();
}

// =====================================================
// OLED PAGE 2
// =====================================================
void showPage2()
{
  display.clearDisplay();

  oledHeader();

  display.setCursor(0, 17);
  display.print("Temperature: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 29);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 41);
  display.print("Soil Moist: ");
  display.println(soilValue);

  display.setCursor(0, 53);
  display.println("Page 2 / 3");

  display.display();
}

// =====================================================
// OLED PAGE 3
// =====================================================
void showPage3()
{
  display.clearDisplay();

  oledHeader();

  display.setCursor(0, 17);
  display.print("LDR: ");
  display.println(ldrValue);

  display.setCursor(0, 29);

  if (ldrValue < 1000)
  {
    display.println("Light: DARK");
  }
  else
  {
    display.println("Light: BRIGHT");
  }

  display.setCursor(0, 41);
  display.print("Status: ");

  if (gasValue >= GAS_THRESHOLD)
  {
    display.println("GAS DANGER");
  }
  else if (waterDistance > 0 &&
           waterDistance <= WATER_HIGH_DISTANCE)
  {
    display.println("WATER HIGH");
  }
  else if (soilValue >= SOIL_THRESHOLD)
  {
    display.println("LEAKAGE");
  }
  else
  {
    display.println("NORMAL");
  }

  display.setCursor(0, 53);
  display.println("Page 3 / 3");

  display.display();
}

// =====================================================
// GAS DANGER DISPLAY
// =====================================================
void showGasDanger()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(15, 0);
  display.println("DANGER!");

  display.setTextSize(1);

  display.setCursor(0, 25);
  display.println("GAS LEAK");

  display.setCursor(0, 38);
  display.println("DETECTED!");

  display.setCursor(0, 52);
  display.print("Gas: ");
  display.println(gasValue);

  display.display();
}

// =====================================================
// OLED SCROLL FUNCTION
// =====================================================
void updateOLED()
{
  if (millis() - lastOLEDUpdate < OLED_PAGE_TIME)
  {
    return;
  }

  lastOLEDUpdate = millis();

  // Gas danger has priority
  if (gasValue >= GAS_THRESHOLD)
  {
    showGasDanger();
    return;
  }

  if (oledPage == 0)
  {
    showPage1();
  }
  else if (oledPage == 1)
  {
    showPage2();
  }
  else if (oledPage == 2)
  {
    showPage3();
  }

  oledPage++;

  if (oledPage > 2)
  {
    oledPage = 0;
  }
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);

  // ===================================================
  // I2C
  // ===================================================
  Wire.begin(SDA_PIN, SCL_PIN);

  // ===================================================
  // OLED
  // ===================================================
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED NOT FOUND!");

    while (1);
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10, 20);
  display.println("Smart Underground");

  display.setCursor(25, 35);
  display.println("Infrastructure");

  display.display();

  delay(2000);

  // ===================================================
  // SENSOR PINS
  // ===================================================
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(WATER_LEVEL_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(SOIL_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // ===================================================
  // OUTPUT PINS
  // ===================================================
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // ===================================================
  // DHT11
  // ===================================================
  dht.begin();

  // ===================================================
  // SERVO
  // ===================================================
  drainageServo.attach(SERVO_PIN);

  // Start at 0 degrees
  drainageServo.write(0);

  Serial.println();
  Serial.println("================================");
  Serial.println("SMART UNDERGROUND INFRASTRUCTURE");
  Serial.println("SYSTEM STARTED");
  Serial.println("================================");
  Serial.println("Servo Control:");
  Serial.println("Water <= 10 cm  -> Servo 180");
  Serial.println("Water > 10 cm   -> Servo 0");
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
  // ===================================================
  // READ SENSORS
  // ===================================================

  waterDistance = readDistance();

  waterLevelValue = analogRead(WATER_LEVEL_PIN);

  gasValue = analogRead(GAS_PIN);

  soilValue = analogRead(SOIL_PIN);

  ldrValue = analogRead(LDR_PIN);

  temperature = dht.readTemperature();

  humidity = dht.readHumidity();

  // ===================================================
  // DHT ERROR CHECK
  // ===================================================

  if (isnan(temperature))
  {
    temperature = 0;
  }

  if (isnan(humidity))
  {
    humidity = 0;
  }

  // ===================================================
  // SERIAL MONITOR
  // ===================================================

  Serial.println();
  Serial.println("================================");

  Serial.print("Water Distance : ");

  if (waterDistance > 0)
  {
    Serial.print(waterDistance, 1);
    Serial.println(" cm");
  }
  else
  {
    Serial.println("ERROR");
  }

  Serial.print("Water Sensor   : ");
  Serial.println(waterLevelValue);

  Serial.print("MQ-135 A0      : ");
  Serial.println(gasValue);

  Serial.print("Temperature    : ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity       : ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("Soil Moisture  : ");
  Serial.println(soilValue);

  Serial.print("LDR            : ");
  Serial.println(ldrValue);

  // ===================================================
  // CONDITIONS
  // ===================================================

  bool waterHigh = false;

  if (waterDistance > 0 &&
      waterDistance <= WATER_HIGH_DISTANCE)
  {
    waterHigh = true;
  }

  bool gasAlert = false;

  if (gasValue >= GAS_THRESHOLD)
  {
    gasAlert = true;
  }

  bool leakage = false;

  if (soilValue >= SOIL_THRESHOLD)
  {
    leakage = true;
  }

  // ===================================================
  // 1. GAS DANGER
  // ===================================================

  if (gasAlert)
  {
    Serial.println();
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("!!!       DANGER           !!!");
    Serial.println("!!!   GAS LEAK DETECTED    !!!");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    // Gas danger position
    drainageServo.write(90);
  }

  // ===================================================
  // 2. WATER LEVEL HIGH
  // ===================================================

  else if (waterHigh)
  {
    Serial.println();
    Serial.println("!!! WATER LEVEL HIGH !!!");
    Serial.println("!!! SERVO -> 180 DEG !!!");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    // Water reaches 10 cm
    // Servo immediately moves to 180 degrees
    drainageServo.write(180);
  }

  // ===================================================
  // 3. LEAKAGE
  // ===================================================

  else if (leakage)
  {
    Serial.println();
    Serial.println("!!! POSSIBLE LEAKAGE !!!");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);

    drainageServo.write(45);
  }

  // ===================================================
  // 4. MEDIUM WATER
  // ===================================================

  else if (waterDistance > 0 &&
           waterDistance <= WATER_MEDIUM_DISTANCE)
  {
    Serial.println();
    Serial.println("WATER LEVEL: MEDIUM");

    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);

    // Below high-water condition
    drainageServo.write(0);
  }

  // ===================================================
  // 5. NORMAL
  // ===================================================

  else
  {
    Serial.println();
    Serial.println("SYSTEM STATUS: NORMAL");

    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // Normal position
    drainageServo.write(0);
  }

  // ===================================================
  // LDR STATUS
  // ===================================================

  if (ldrValue < 1000)
  {
    Serial.println("Underground: DARK");
  }
  else
  {
    Serial.println("Underground: LIGHT");
  }

  // ===================================================
  // UPDATE OLED
  // ===================================================

  updateOLED();

  delay(200);
}