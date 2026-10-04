#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <MFRC522.h>
#include <UniversalTelegramBot.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// Wi-Fi
// =====================================================

#define WIFI_SSID "Shyam"
#define WIFI_PASSWORD "123456789"

// =====================================================
// Telegram
// =====================================================

#define BOT_TOKEN "8878281794:AAHhPGRZP-SjcCnR7O9RkFRwF24MX0C5FRI"
#define CHAT_ID "8753067838"

WiFiClientSecure client;

UniversalTelegramBot bot(
  BOT_TOKEN,
  client
);

// =====================================================
// OLED DISPLAY
// =====================================================

#define OLED_SDA 21
#define OLED_SCL 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1

#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// RC522 RFID
// =====================================================

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(
  SS_PIN,
  RST_PIN
);

// =====================================================
// STUDENT RFID UIDs
// =====================================================

// Student 1 - Bheeru Gowdu
byte student1UID[] = {
  0x1D,
  0x93,
  0xD9,
  0x05
};

// Student 2 - Rahul
byte student2UID[] = {
  0xF4,
  0xD1,
  0xD9,
  0x05
};

// Student 3 - Priya
// Replace with Priya's real RFID UID
byte student3UID[] = {
  0x11,
  0x22,
  0x33,
  0x44
};

// =====================================================
// ATTENDANCE RESTRICTION
// =====================================================

// 1 hour
const unsigned long ATTENDANCE_INTERVAL =
  60UL * 60UL * 1000UL;

// Last attendance time
unsigned long student1LastAttendance = 0;
unsigned long student2LastAttendance = 0;
unsigned long student3LastAttendance = 0;

// =====================================================
// OLED DEFAULT SCREEN
// =====================================================

void showDefaultScreen() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setCursor(0, 10);

  display.println(
    "Welcome"
  );

  display.setCursor(0, 30);

  display.println(
    "Scan RFID Card"
  );

  display.display();
}

// =====================================================
// COMPARE RFID UID
// =====================================================

bool compareUID(
  byte *cardUID,
  byte *studentUID,
  byte length
) {

  for (
    byte i = 0;
    i < length;
    i++
  ) {

    if (
      cardUID[i] != studentUID[i]
    ) {

      return false;
    }
  }

  return true;
}

// =====================================================
// OLED DISPLAY MESSAGE
// =====================================================

void displayMessage(
  String line1,
  String line2
) {

  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setCursor(0, 10);

  display.println(line1);

  display.setCursor(0, 30);

  display.println(line2);

  display.display();
}

// =====================================================
// SEND TELEGRAM + OLED
// =====================================================

void sendAttendanceMessage(
  String oledLine1,
  String oledLine2,
  String telegramMessage
) {

  // Display attendance message
  displayMessage(
    oledLine1,
    oledLine2
  );

  // Send Telegram message
  bot.sendMessage(
    CHAT_ID,
    telegramMessage,
    ""
  );

  // Serial Monitor
  Serial.println(
    "--------------------------------"
  );

  Serial.println(
    telegramMessage
  );

  Serial.println(
    "--------------------------------"
  );

  // ===================================================
  // KEEP MESSAGE ON OLED FOR 5 SECONDS
  // ===================================================

  delay(5000);

  // ===================================================
  // RETURN TO DEFAULT SCREEN
  // ===================================================

  showDefaultScreen();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  // ===================================================
  // SERIAL
  // ===================================================

  Serial.begin(115200);

  // ===================================================
  // OLED I2C
  // ===================================================

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  // ===================================================
  // START OLED
  // ===================================================

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    )
  ) {

    Serial.println(
      "OLED not found!"
    );

    while (1);
  }

  // ===================================================
  // STARTUP SCREEN
  // ===================================================

  display.clearDisplay();

  display.setTextSize(1);

  display.setTextColor(
    SSD1306_WHITE
  );

  display.setCursor(0, 10);

  display.println(
    "RFID ATTENDANCE"
  );

  display.setCursor(0, 30);

  display.println(
    "Starting..."
  );

  display.display();

  delay(2000);

  // ===================================================
  // RFID
  // ===================================================

  SPI.begin();

  rfid.PCD_Init();

  Serial.println(
    "RFID initialized."
  );

  // ===================================================
  // WIFI
  // ===================================================

  displayMessage(
    "Connecting WiFi",
    "Please wait..."
  );

  Serial.print(
    "Connecting to Wi-Fi"
  );

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println(
    "Wi-Fi Connected"
  );

  Serial.print(
    "IP Address: "
  );

  Serial.println(
    WiFi.localIP()
  );

  // ===================================================
  // TELEGRAM HTTPS
  // ===================================================

  client.setInsecure();

  // ===================================================
  // DEFAULT SCREEN
  // ===================================================

  showDefaultScreen();

  // ===================================================
  // SERIAL INFORMATION
  // ===================================================

  Serial.println();

  Serial.println(
    "================================="
  );

  Serial.println(
    "     RFID ATTENDANCE SYSTEM"
  );

  Serial.println(
    "================================="
  );

  Serial.println(
    "Wi-Fi: Connected"
  );

  Serial.println(
    "Telegram: Ready"
  );

  Serial.println(
    "RFID: Ready"
  );

  Serial.println(
    "OLED: Ready"
  );

  Serial.println(
    "Attendance Limit: 1 Hour"
  );

  Serial.println(
    "================================="
  );

  Serial.println(
    "Scan RFID Card..."
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // CHECK FOR NEW RFID CARD
  // ===================================================

  if (
    !rfid.PICC_IsNewCardPresent()
  ) {

    return;
  }

  // ===================================================
  // READ RFID CARD
  // ===================================================

  if (
    !rfid.PICC_ReadCardSerial()
  ) {

    return;
  }

  // ===================================================
  // PRINT CARD UID
  // ===================================================

  Serial.print(
    "Card UID:"
  );

  for (
    byte i = 0;
    i < rfid.uid.size;
    i++
  ) {

    Serial.print(
      rfid.uid.uidByte[i] < 0x10
      ? " 0"
      : " "
    );

    Serial.print(
      rfid.uid.uidByte[i],
      HEX
    );
  }

  Serial.println();

  // Current time
  unsigned long currentTime =
    millis();

  // ===================================================
  // STUDENT 1 - BHEERU GOWDU
  // ===================================================

  if (
    rfid.uid.size == 4 &&
    compareUID(
      rfid.uid.uidByte,
      student1UID,
      4
    )
  ) {

    Serial.println(
      "Student: Bheeru Gowdu"
    );

    // Check 1 hour
    if (
      student1LastAttendance == 0 ||
      currentTime -
      student1LastAttendance >=
      ATTENDANCE_INTERVAL
    ) {

      student1LastAttendance =
        currentTime;

      Serial.println(
        "Attendance: MARKED"
      );

      sendAttendanceMessage(
        "Bheeru: Present",
        "Attendance Marked",
        "Bheeru: Present\nAttendance Marked"
      );

    } else {

      Serial.println(
        "Attendance already marked."
      );

      sendAttendanceMessage(
        "Bheeru : Already Marked",
        "Wait for next class",
        "Bheeru: Already Marked\nWait for next class"
      );
    }
  }

  // ===================================================
  // STUDENT 2 - RAHUL
  // ===================================================

  else if (
    rfid.uid.size == 4 &&
    compareUID(
      rfid.uid.uidByte,
      student2UID,
      4
    )
  ) {

    Serial.println(
      "Student: Rahul"
    );

    // Check 1 hour
    if (
      student2LastAttendance == 0 ||
      currentTime -
      student2LastAttendance >=
      ATTENDANCE_INTERVAL
    ) {

      student2LastAttendance =
        currentTime;

      Serial.println(
        "Attendance: MARKED"
      );

      sendAttendanceMessage(
        "Rahul: Present",
        "Attendance Marked",
        "Rahul: Present\nAttendance Marked"
      );

    } else {

      Serial.println(
        "Attendance already marked."
      );

      sendAttendanceMessage(
        "Rahul : Already Marked",
        "Wait for next class",
        "Rahul: Already Marked\nWait for next class"
      );
    }
  }

  // ===================================================
  // STUDENT 3 - PRIYA
  // ===================================================

  else if (
    rfid.uid.size == 4 &&
    compareUID(
      rfid.uid.uidByte,
      student3UID,
      4
    )
  ) {

    Serial.println(
      "Student: Priya"
    );

    // Check 1 hour
    if (
      student3LastAttendance == 0 ||
      currentTime -
      student3LastAttendance >=
      ATTENDANCE_INTERVAL
    ) {

      student3LastAttendance =
        currentTime;

      Serial.println(
        "Attendance: MARKED"
      );

      sendAttendanceMessage(
        "Priya: Present",
        "Attendance Marked",
        "Priya: Present\nAttendance Marked"
      );

    } else {

      Serial.println(
        "Attendance already marked."
      );

      sendAttendanceMessage(
        "Already Marked",
        "Wait for next class",
        "Priya: Already Marked\nWait for next class"
      );
    }
  }

  // ===================================================
  // UNKNOWN RFID CARD
  // ===================================================

  else {

    Serial.println(
      "Unknown RFID Card!"
    );

    sendAttendanceMessage(
      "Unknown RFID",
      "Please try again",
      "Unknown RFID\nPlease try again"
    );
  }

  // ===================================================
  // STOP RFID COMMUNICATION
  // ===================================================

  rfid.PICC_HaltA();

  rfid.PCD_StopCrypto1();

  delay(2000);
}