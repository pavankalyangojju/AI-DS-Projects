#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= Ultrasonic =================
#define TRIG_PIN 5
#define ECHO_PIN 18

// ================= LEDs =================
#define RED_LED 26
#define GREEN_LED 27

// ================= Buzzer =================
#define BUZZER_PIN 25

// Tank full threshold
#define FULL_DISTANCE 5.0

void setup() {

  Serial.begin(115200);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // LEDs
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(20, 10);
  display.println("Water Tank");
  display.setCursor(30, 30);
  display.println("Monitoring");

  display.display();

  delay(2000);
}

void loop() {

  // ================= Measure Distance =================

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Convert time to distance in cm
  float distance = duration * 0.0343 / 2;

  // If no echo is received
  if (duration == 0) {
    Serial.println("Ultrasonic sensor error");

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 20);
    display.println("Sensor Error");
    display.display();

    delay(500);
    return;
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // ================= Tank Full =================

  if (distance < FULL_DISTANCE) {

    // Red ON
    digitalWrite(RED_LED, HIGH);

    // Green OFF
    digitalWrite(GREEN_LED, LOW);

    // Buzzer ON
    digitalWrite(BUZZER_PIN, HIGH);

    // OLED
    display.clearDisplay();

    display.setTextSize(2);
    display.setCursor(15, 5);
    display.println("TANK");

    display.setCursor(15, 28);
    display.println("FULL!");

    display.setTextSize(1);
    display.setCursor(25, 52);
    display.print(distance, 1);
    display.println(" cm");

    display.display();
  }

  // ================= Tank Not Full =================

  else {

    // Red OFF
    digitalWrite(RED_LED, LOW);

    // Green ON
    digitalWrite(GREEN_LED, HIGH);

    // Buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);

    // OLED
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(25, 5);
    display.println("WATER TANK");

    display.setTextSize(2);
    display.setCursor(5, 25);
    display.println("NOT FULL");

    display.setTextSize(1);
    display.setCursor(25, 52);
    display.print(distance, 1);
    display.println(" cm");

    display.display();
  }

  delay(500);
}