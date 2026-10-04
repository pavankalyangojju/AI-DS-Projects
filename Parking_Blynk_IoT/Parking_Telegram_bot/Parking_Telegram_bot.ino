#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// WIFI SETTINGS
// =====================================================

const char* WIFI_SSID = "NTRguna";
const char* WIFI_PASSWORD = "Guna@8688";

// =====================================================
// TELEGRAM BOT SETTINGS
// =====================================================

// Get this from @BotFather
#define BOT_TOKEN "8642993016:AAGf9Tfq6ZNQL-eM4B1Gcrxmq_7z6ql_-HY"

// Your Telegram Chat ID
#define CHAT_ID "8753067838"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

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
// CREATE PARKING MESSAGE
// SAME INFORMATION AS OLED
// =====================================================

String getParkingMessage() {

  int freeSlots = getFreeSlots();

  String message = "";

  message += "SMART PARKING\n\n";

  message += "S1: ";
  message += (slot1 ? "FILLED" : "FREE");

  message += "       S3: ";
  message += (slot3 ? "FILLED" : "FREE");

  message += "\n";

  message += "S2: ";
  message += (slot2 ? "FILLED" : "FREE");

  message += "       S4: ";
  message += (slot4 ? "FILLED" : "FREE");

  message += "\n\n";

  message += "FREE SLOTS: ";
  message += String(freeSlots);

  return message;
}

// =====================================================
// SEND PARKING STATUS TO TELEGRAM
// =====================================================

void sendParkingStatusToTelegram() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi not connected.");
    return;
  }

  String message = getParkingMessage();

  Serial.println("Sending parking status to Telegram...");

  bool result = bot.sendMessage(
    CHAT_ID,
    message,
    ""
  );

  if (result) {
    Serial.println("Telegram message sent successfully.");
  } 
  else {
    Serial.println("Failed to send Telegram message.");
  }
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
// CONNECT TO WIFI
// =====================================================

void connectWiFi() {

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
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

  Wire.begin(OLED_SDA, OLED_SCL);

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
  // WIFI SETUP
  // ===================================================

  connectWiFi();

  // ===================================================
  // TELEGRAM SSL
  // ===================================================

  // Allows the ESP32 to connect to Telegram
  client.setInsecure();

  Serial.println("Telegram client ready.");

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
  // SEND INITIAL STATUS TO TELEGRAM
  // ===================================================

  sendParkingStatusToTelegram();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // Read sensors
  readSensors();

  // Check whether parking status changed
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
    // TELEGRAM: Send SAME parking status
    // -------------------------------------------------

    sendParkingStatusToTelegram();
  }

  delay(100);
}