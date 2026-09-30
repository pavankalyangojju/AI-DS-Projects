#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// =====================================================
// OLED DISPLAY
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// ULTRASONIC SENSOR 1 - HUMAN DETECTION
// =====================================================

#define TRIG_HAND 5
#define ECHO_HAND 18

// =====================================================
// ULTRASONIC SENSOR 2 - GARBAGE LEVEL
// =====================================================

#define TRIG_LEVEL 4
#define ECHO_LEVEL 2

// =====================================================
// SERVO MOTOR
// =====================================================

#define SERVO_PIN 17

Servo lidServo;

// Servo positions
#define LID_CLOSED 0
#define LID_OPEN 90

// Hand detection distance
#define HAND_DISTANCE 10

// =====================================================
// LEDS
// =====================================================

#define GREEN_LED 27
#define RED_LED 14

// =====================================================
// DUSTBIN HEIGHT
// =====================================================

#define DUSTBIN_HEIGHT 30

// =====================================================
// SERVO FUNCTION
// =====================================================

void openLid()
{
  Serial.println("LID OPENING");

  // ---------------------------------------------------
  // OPEN LID
  // ---------------------------------------------------

  for (int angle = LID_CLOSED; angle <= LID_OPEN; angle++)
  {
    lidServo.write(angle);
    delay(15);
  }

  Serial.println("LID OPEN");

  // ---------------------------------------------------
  // KEEP LID OPEN FOR 3 SECONDS
  // ---------------------------------------------------

  delay(3000);

  Serial.println("3 SECONDS COMPLETED");

  // ---------------------------------------------------
  // CLOSE LID
  // ---------------------------------------------------

  Serial.println("LID CLOSING");

  for (int angle = LID_OPEN; angle >= LID_CLOSED; angle--)
  {
    lidServo.write(angle);
    delay(15);
  }

  Serial.println("LID CLOSED");

  delay(500);
}

// =====================================================
// ULTRASONIC DISTANCE FUNCTION
// =====================================================

float getDistance(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  // No echo received
  if (duration == 0)
  {
    return -1;
  }

  float distance = duration * 0.0343 / 2;

  return distance;
}

// =====================================================
// CALCULATE GARBAGE LEVEL
// =====================================================

int calculateGarbageLevel(float gap)
{
  if (gap < 0)
  {
    return -1;
  }

  // Empty
  if (gap >= DUSTBIN_HEIGHT)
  {
    return 0;
  }

  // Full
  if (gap <= 5)
  {
    return 100;
  }

  int level = map(
    (int)gap,
    DUSTBIN_HEIGHT,
    5,
    0,
    100
  );

  level = constrain(level, 0, 100);

  return level;
}

// =====================================================
// UPDATE OLED DISPLAY
// =====================================================

void updateDisplay(
  float handDistance,
  float garbageGap,
  int garbageLevel
)
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  // -----------------------------
  // TITLE
  // -----------------------------

  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("SMART DUSTBIN");

  // -----------------------------
  // HAND DISTANCE
  // -----------------------------

  display.setCursor(0, 13);
  display.print("Hand: ");

  if (handDistance < 0)
  {
    display.println("ERROR");
  }
  else
  {
    display.print(handDistance, 1);
    display.println(" cm");
  }

  // -----------------------------
  // GARBAGE GAP
  // -----------------------------

  display.setCursor(0, 26);
  display.print("Gap: ");

  if (garbageGap < 0)
  {
    display.println("ERROR");
  }
  else
  {
    display.print(garbageGap, 1);
    display.println(" cm");
  }

  // -----------------------------
  // GARBAGE LEVEL
  // -----------------------------

  display.setCursor(0, 39);
  display.print("Level: ");

  if (garbageLevel < 0)
  {
    display.println("ERROR");
  }
  else
  {
    display.print(garbageLevel);
    display.println("%");
  }

  // -----------------------------
  // STATUS
  // -----------------------------

  display.setCursor(0, 52);

  if (garbageLevel < 0)
  {
    display.println("SENSOR ERROR");
  }
  else if (garbageLevel >= 90)
  {
    display.println("STATUS: FULL");
  }
  else if (garbageLevel >= 70)
  {
    display.println("STATUS: ALMOST FULL");
  }
  else if (garbageLevel >= 30)
  {
    display.println("STATUS: MEDIUM");
  }
  else
  {
    display.println("STATUS: EMPTY");
  }

  display.display();
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  // ===================================================
  // ULTRASONIC SENSOR 1
  // ===================================================

  pinMode(TRIG_HAND, OUTPUT);
  pinMode(ECHO_HAND, INPUT);

  // ===================================================
  // ULTRASONIC SENSOR 2
  // ===================================================

  pinMode(TRIG_LEVEL, OUTPUT);
  pinMode(ECHO_LEVEL, INPUT);

  // ===================================================
  // LEDS
  // ===================================================

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  // ===================================================
  // SERVO
  // ===================================================

  lidServo.setPeriodHertz(50);

  lidServo.attach(
    SERVO_PIN,
    500,
    2400
  );

  // Start with lid closed
  lidServo.write(LID_CLOSED);

  delay(1000);

  // ===================================================
  // OLED
  // ===================================================

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS))
  {
    Serial.println("OLED NOT FOUND");

    while (true)
    {
      delay(100);
    }
  }

  // ===================================================
  // WELCOME SCREEN
  // ===================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 15);
  display.println("WELCOME");

  display.setTextSize(1);
  display.setCursor(30, 40);
  display.println("SMART DUSTBIN");

  display.display();

  delay(3000);

  // ===================================================
  // SERIAL MESSAGE
  // ===================================================

  Serial.println();
  Serial.println("==============================");
  Serial.println("     SMART DUSTBIN STARTED");
  Serial.println("==============================");
  Serial.println();

  Serial.println("Servo: GPIO 23");
  Serial.println("Hand Sensor: GPIO 5 / GPIO 18");
  Serial.println("Level Sensor: GPIO 4 / GPIO 2");
  Serial.println("Green LED: GPIO 27");
  Serial.println("Red LED: GPIO 14");
  Serial.println();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // ===================================================
  // READ HAND SENSOR
  // ===================================================

  float handDistance = getDistance(
    TRIG_HAND,
    ECHO_HAND
  );

  // Small delay to avoid ultrasonic interference
  delay(50);

  // ===================================================
  // READ GARBAGE LEVEL SENSOR
  // ===================================================

  float garbageGap = getDistance(
    TRIG_LEVEL,
    ECHO_LEVEL
  );

  // ===================================================
  // CALCULATE GARBAGE LEVEL
  // ===================================================

  int garbageLevel = calculateGarbageLevel(
    garbageGap
  );

  // ===================================================
  // HAND DETECTION
  // =====================================================

  static bool handDetected = false;

  if (handDistance > 0 && handDistance < HAND_DISTANCE)
  {
    if (!handDetected)
    {
      handDetected = true;

      Serial.println();
      Serial.println("HAND DETECTED - LESS THAN 10 CM");

      openLid();
    }
  }
  else
  {
    // Hand moved away
    handDetected = false;
  }

  // ===================================================
  // LED STATUS
  // ===================================================

  if (garbageLevel >= 90)
  {
    // FULL
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }
  else
  {
    // SPACE AVAILABLE
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }

  // ===================================================
  // SERIAL MONITOR
  // =====================================================

  Serial.print("Hand: ");

  if (handDistance < 0)
  {
    Serial.print("ERROR");
  }
  else
  {
    Serial.print(handDistance, 1);
    Serial.print(" cm");
  }

  Serial.print(" | Gap: ");

  if (garbageGap < 0)
  {
    Serial.print("ERROR");
  }
  else
  {
    Serial.print(garbageGap, 1);
    Serial.print(" cm");
  }

  Serial.print(" | Level: ");

  if (garbageLevel < 0)
  {
    Serial.println("ERROR");
  }
  else
  {
    Serial.print(garbageLevel);
    Serial.println("%");
  }

  // ===================================================
  // OLED
  // ===================================================

  updateDisplay(
    handDistance,
    garbageGap,
    garbageLevel
  );

  delay(500);
}