#define BLYNK_TEMPLATE_ID "TMPL3QYm5wUfC"
#define BLYNK_TEMPLATE_NAME "Heart Rate"
#define BLYNK_AUTH_TOKEN "pqEaaeYYFJlWwGUpMkKuFq-rh0_YnOlR"

#include <Wire.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "G";
char pass[] = "123456789";

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

long readIR()
{
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x07);
  Wire.endTransmission(false);

  Wire.requestFrom(MAX30102_ADDR, 6);

  if (Wire.available() < 6)
    return 0;

  Wire.read();
  Wire.read();
  Wire.read();

  byte ir1 = Wire.read();
  byte ir2 = Wire.read();
  byte ir3 = Wire.read();

  long ir = ((long)ir1 << 16) | ((long)ir2 << 8) | ir3;

  return ir & 0x3FFFF;
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(21, 22);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("MAX30102 Heart Rate Monitor");

  Wire.beginTransmission(MAX30102_ADDR);

  if (Wire.endTransmission() != 0)
  {
    Serial.println("MAX30102 NOT FOUND!");
    while (1);
  }

  Serial.println("MAX30102 FOUND!");
  Serial.println("Place your finger on the sensor.");

  // Reset MAX30102
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x09);
  Wire.write(0x40);
  Wire.endTransmission();

  delay(100);

  // FIFO configuration
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x08);
  Wire.write(0x0F);
  Wire.endTransmission();

  // Red + IR mode
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x09);
  Wire.write(0x03);
  Wire.endTransmission();

  // SpO2 configuration
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0A);
  Wire.write(0x27);
  Wire.endTransmission();

  // Red LED
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0C);
  Wire.write(0x24);
  Wire.endTransmission();

  // IR LED
  Wire.beginTransmission(MAX30102_ADDR);
  Wire.write(0x0D);
  Wire.write(0x24);
  Wire.endTransmission();
}

void loop()
{
  Blynk.run();

  irValue = readIR();

  if (irValue < 10000)
  {
    if (millis() - lastPrintTime >= 1000)
    {
      Serial.println("Place finger on sensor...");

      Blynk.virtualWrite(V0, 0);

      lastPrintTime = millis();
    }

    previousIR = irValue;
    return;
  }

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

  if (difference < -100)
  {
    beatDetected = false;
  }

  previousIR = irValue;

  if (millis() - lastPrintTime >= 1000)
  {
    if (averageBPM > 0)
    {
      Serial.print("Heart Pulse: ");
      Serial.print(averageBPM, 0);
      Serial.println(" BPM");

      Blynk.virtualWrite(V0, averageBPM);
    }
    else
    {
      Serial.println("Detecting Heart Pulse...");
    }

    lastPrintTime = millis();
  }

  delay(10);
}
