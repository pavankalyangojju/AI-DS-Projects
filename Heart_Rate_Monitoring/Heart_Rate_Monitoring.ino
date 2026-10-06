#define BLYNK_TEMPLATE_ID "TMPL3QYm5wUfC"
#define BLYNK_TEMPLATE_NAME "Heart Rate"
#define BLYNK_AUTH_TOKEN "pqEaaeYYFJlWwGUpMkKuFq-rh0_YnOlR"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

char ssid[] = "GOJJU";
char pass[] = "0987654321";

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// =====================================================
// MAX30102
// =====================================================
#define MAX30102_ADDR 0x57

long irValue = 0;
long previousIR = 0;

unsigned long lastBeatTime = 0;
unsigned long lastPrintTime = 0;

float bpm = 0;
float averageBPM = 0;

float bpmValues[4] = {0, 0, 0, 0};
byte bpmIndex = 0;

bool beatDetected = false;

// =====================================================
// READ MAX30102 IR
// =====================================================
long readIR()
{
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x07);
  Wire.endTransmission(false);

  Wire.requestFrom(MAX30102_ADDR, 6);

  if (Wire.available() < 6)
    return 0;

  // Skip first 3 bytes
  Wire.read();
  Wire.read();
  Wire.read();

  byte ir1 = Wire.read();
  byte ir2 = Wire.read();
  byte ir3 = Wire.read();

  long ir = ((long)ir1 << 16) |
            ((long)ir2 << 8) |
            ir3;

  return ir & 0x3FFFF;
}

// =====================================================
// OLED FUNCTIONS
// =====================================================
void showOLED(String line1, String line2)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(line1);

  display.setTextSize(2);
  display.setCursor(0, 25);
  display.println(line2);

  display.display();
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);

  // I2C
  Wire.begin(21, 22);

  // ===================================================
  // OLED START
  // ===================================================
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("OLED NOT FOUND!");

    while (1);
  }

  Serial.println("OLED FOUND!");

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(15, 10);
  display.println("HEART");

  display.setCursor(15, 35);
  display.println("RATE");

  display.display();

  delay(2000);

  // ===================================================
  // BLYNK
  // ===================================================
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("MAX30102 Heart Rate Monitor");

  // ===================================================
  // CHECK MAX30102
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);

  if (Wire.endTransmission() != 0)
  {
    Serial.println("MAX30102 NOT FOUND!");

    showOLED("MAX30102", "NOT FOUND");

    while (1);
  }

  Serial.println("MAX30102 FOUND!");

  showOLED("MAX30102", "READY");

  delay(1500);

  // ===================================================
  // RESET MAX30102
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x09);
  Wire.write(0x40);
  Wire.endTransmission();

  delay(100);

  // ===================================================
  // FIFO CONFIGURATION
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x08);
  Wire.write(0x0F);
  Wire.endTransmission();

  // ===================================================
  // RED + IR MODE
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x09);
  Wire.write(0x03);
  Wire.endTransmission();

  // ===================================================
  // SPO2 CONFIGURATION
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0A);
  Wire.write(0x27);
  Wire.endTransmission();

  // ===================================================
  // RED LED
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0C);
  Wire.write(0x24);
  Wire.endTransmission();

  // ===================================================
  // IR LED
  // ===================================================
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0D);
  Wire.write(0x24);
  Wire.endTransmission();

  showOLED("Place Finger", "ON SENSOR");
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
  Blynk.run();

  irValue = readIR();

  // ===================================================
  // NO FINGER
  // ===================================================
  if (irValue < 10000)
  {
    if (millis() - lastPrintTime >= 1000)
    {
      Serial.println("Place finger on sensor...");

      Blynk.virtualWrite(V0, 0);

      showOLED("Heart Rate", "PLACE FINGER");

      lastPrintTime = millis();
    }

    previousIR = irValue;

    return;
  }

  // ===================================================
  // BEAT DETECTION
  // ===================================================
  long difference = irValue - previousIR;

  if (difference > 300 && !beatDetected)
  {
    beatDetected = true;

    unsigned long currentTime = millis();

    if (lastBeatTime > 0)
    {
      unsigned long interval = currentTime - lastBeatTime;

      if (interval >= 300 && interval <= 2000)
      {
        bpm = 60000.0 / interval;

        bpmValues[bpmIndex] = bpm;

        bpmIndex++;

        if (bpmIndex >= 4)
          bpmIndex = 0;

        float total = 0;
        int count = 0;

        for (int i = 0; i < 4; i++)
        {
          if (bpmValues[i] > 0)
          {
            total += bpmValues[i];
            count++;
          }
        }

        if (count > 0)
          averageBPM = total / count;
      }
    }

    lastBeatTime = currentTime;
  }

  // ===================================================
  // RESET BEAT DETECTION
  // ===================================================
  if (difference < -100)
  {
    beatDetected = false;
  }

  previousIR = irValue;

  // ===================================================
  // DISPLAY BPM
  // ===================================================
  if (millis() - lastPrintTime >= 1000)
  {
    if (averageBPM > 0)
    {
      Serial.print("Heart Pulse: ");
      Serial.print(averageBPM, 0);
      Serial.println(" BPM");

      Blynk.virtualWrite(V0, averageBPM);

      display.clearDisplay();

      display.setTextColor(SSD1306_WHITE);

      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("HEART RATE");

      display.setTextSize(3);
      display.setCursor(20, 20);
      display.print(averageBPM, 0);

      display.setTextSize(2);
      display.setCursor(85, 28);
      display.println("BPM");

      display.display();
    }
    else
    {
      Serial.println("Detecting Heart Pulse...");

      showOLED("Heart Rate", "DETECTING");
    }

    lastPrintTime = millis();
  }

  delay(10);
}
