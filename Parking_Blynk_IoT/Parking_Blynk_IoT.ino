// =====================================================
// BLYNK SETTINGS
// =====================================================

#define BLYNK_TEMPLATE_ID "TMPL3EYwRHZEc"
#define BLYNK_TEMPLATE_NAME "Smart Parking"
#define BLYNK_AUTH_TOKEN "jYPsxBXoh-7D02wA4PGH1lm_VvewN95F"


#define BLYNK_PRINT Serial

// =====================================================
// LIBRARIES
// =====================================================

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI SETTINGS
// =====================================================

char ssid[] = "NTRguna";
char pass[] = "Guna@8688";

// =====================================================
// BLYNK VIRTUAL PINS
// =====================================================

#define VPIN_SLOT1 V0
#define VPIN_SLOT2 V1
#define VPIN_SLOT3 V2
#define VPIN_SLOT4 V3
#define VPIN_FREE_SLOTS V4

BlynkTimer timer;

// =====================================================
// OLED SETTINGS
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// OLED I2C pins
#define OLED_SDA 21
#define OLED_SCL 22

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// IR SENSOR PINS
// =====================================================

#define IR1 16
#define IR2 17
#define IR3 18
#define IR4 19

// =====================================================
// PARKING STATUS
// =====================================================

bool slot1;
bool slot2;
bool slot3;
bool slot4;

bool previousSlot1;
bool previousSlot2;
bool previousSlot3;
bool previousSlot4;

// =====================================================
// READ IR SENSORS
// =====================================================

void readSensors() {

  // Most IR sensors:
  // LOW  = Object detected / FILLED
  // HIGH = No object / FREE

  slot1 = (digitalRead(IR1) == LOW);
  slot2 = (digitalRead(IR2) == LOW);
  slot3 = (digitalRead(IR3) == LOW);
  slot4 = (digitalRead(IR4) == LOW);
}

// =====================================================
// GET FREE SLOT COUNT
// =====================================================

int getFreeSlots() {

  int freeSlots = 0;

  if (!slot1) freeSlots++;
  if (!slot2) freeSlots++;
  if (!slot3) freeSlots++;
  if (!slot4) freeSlots++;

  return freeSlots;
}

// =====================================================
// UPDATE BLYNK
// =====================================================

void updateBlynk() {

  // 0 = FREE
  // 1 = FILLED / OCCUPIED

  Blynk.virtualWrite(
    VPIN_SLOT1,
    slot1 ? 1 : 0
  );

  Blynk.virtualWrite(
    VPIN_SLOT2,
    slot2 ? 1 : 0
  );

  Blynk.virtualWrite(
    VPIN_SLOT3,
    slot3 ? 1 : 0
  );

  Blynk.virtualWrite(
    VPIN_SLOT4,
    slot4 ? 1 : 0
  );

  // Number of available parking slots

  Blynk.virtualWrite(
    VPIN_FREE_SLOTS,
    getFreeSlots()
  );

  Serial.println("Blynk updated.");
}

// =====================================================
// WELCOME SCREEN
// =====================================================

void showWelcome() {

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 15);
  display.println("WELCOME");

  display.setCursor(20, 40);
  display.println("PARKING");

  display.display();

  delay(3000);
}

// =====================================================
// PARKING STATUS SCREEN
// =====================================================

void showParkingStatus() {

  int freeSlots = getFreeSlots();

  display.clearDisplay();

  // -----------------------------
  // TITLE
  // -----------------------------

  display.setTextSize(1);

  display.setCursor(27, 0);
  display.println("SMART PARKING");

  // -----------------------------
  // SLOT 1
  // -----------------------------

  display.setCursor(0, 16);

  display.print("S1: ");

  if (slot1) {
    display.print("FILLED");
  }
  else {
    display.print("FREE");
  }

  // -----------------------------
  // SLOT 3
  // -----------------------------

  display.setCursor(70, 16);

  display.print("S3: ");

  if (slot3) {
    display.print("FILLED");
  }
  else {
    display.print("FREE");
  }

  // -----------------------------
  // SLOT 2
  // -----------------------------

  display.setCursor(0, 31);

  display.print("S2: ");

  if (slot2) {
    display.print("FILLED");
  }
  else {
    display.print("FREE");
  }

  // -----------------------------
  // SLOT 4
  // -----------------------------

  display.setCursor(70, 31);

  display.print("S4: ");

  if (slot4) {
    display.print("FILLED");
  }
  else {
    display.print("FREE");
  }

  // -----------------------------
  // FREE SLOT COUNT
  // -----------------------------

  display.setCursor(25, 51);

  display.print("FREE SLOTS: ");
  display.print(freeSlots);

  display.display();
}

// =====================================================
// CHECK STATUS CHANGE
// =====================================================

bool statusChanged() {

  if (slot1 != previousSlot1) {
    return true;
  }

  if (slot2 != previousSlot2) {
    return true;
  }

  if (slot3 != previousSlot3) {
    return true;
  }

  if (slot4 != previousSlot4) {
    return true;
  }

  return false;
}

// =====================================================
// SAVE CURRENT STATUS
// =====================================================

void saveCurrentStatus() {

  previousSlot1 = slot1;
  previousSlot2 = slot2;
  previousSlot3 = slot3;
  previousSlot4 = slot4;
}

// =====================================================
// CONNECT TO WIFI + BLYNK
// =====================================================

void connectBlynk() {

  Serial.println();
  Serial.println("Connecting to WiFi...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  Serial.println();
  Serial.println("Blynk connected.");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// =====================================================
// CHECK PARKING STATUS
// =====================================================

void checkParkingStatus() {

  // Read sensors

  readSensors();

  // Check if any slot changed

  if (statusChanged()) {

    Serial.println();
    Serial.println("================================");
    Serial.println("PARKING STATUS CHANGED");
    Serial.println("================================");

    // Save new status

    saveCurrentStatus();

    // -------------------------------------------------
    // OLED: Show welcome
    // -------------------------------------------------

    showWelcome();

    // -------------------------------------------------
    // OLED: Show updated parking status
    // -------------------------------------------------

    showParkingStatus();

    // -------------------------------------------------
    // BLYNK: Send updated status
    // -------------------------------------------------

    updateBlynk();
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ===================================================
  // IR SENSOR SETUP
  // ===================================================

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);

  // ===================================================
  // OLED SETUP
  // ===================================================

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED NOT FOUND");

    while (true) {
      delay(100);
    }
  }

  Serial.println("OLED READY");

  // ===================================================
  // INITIAL SENSOR STATUS
  // ===================================================

  readSensors();

  saveCurrentStatus();

  // ===================================================
  // WELCOME SCREEN
  // ===================================================

  showWelcome();

  // ===================================================
  // SHOW INITIAL PARKING STATUS
  // ===================================================

  showParkingStatus();

  // ===================================================
  // CONNECT TO BLYNK
  // ===================================================

  connectBlynk();

  // ===================================================
  // SEND INITIAL STATUS TO BLYNK
  // ===================================================

  updateBlynk();

  // ===================================================
  // CHECK SENSORS EVERY 500ms
  // ===================================================

  timer.setInterval(
    500L,
    checkParkingStatus
  );

  Serial.println();
  Serial.println("================================");
  Serial.println("SMART PARKING SYSTEM STARTED");
  Serial.println("================================");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Keep Blynk connected

  Blynk.run();

  // Run timer

  timer.run();
}