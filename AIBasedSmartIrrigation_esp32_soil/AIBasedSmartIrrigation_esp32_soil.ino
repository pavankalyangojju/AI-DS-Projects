#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =====================================================
// WIFI
// =====================================================

const char* ssid = "GOJJU";
const char* password = "0987654321";

// =====================================================
// TELEGRAM
// =====================================================

#define BOT_TOKEN "8906843904:AAGwwWtU0LYH_RJXx5W4jf92d8njVgUvC0g"
#define CHAT_ID   "1367693706"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// =====================================================
// PINS
// =====================================================

#define SOIL_PIN    34
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
// SOIL CALIBRATION
// =====================================================

#define SOIL_DRY_VALUE 3200
#define SOIL_WET_VALUE 1000

#define DRY_THRESHOLD 40

// =====================================================
// STATUS VARIABLES
// =====================================================

bool previousDryStatus = false;
bool firstReading = true;

unsigned long lastTelegramCheck = 0;
const unsigned long telegramInterval = 1000;


// =====================================================
// READ SOIL MOISTURE
// =====================================================

int getSoilMoisture() {

  int soilRaw = analogRead(SOIL_PIN);

  int moisture = map(
    soilRaw,
    SOIL_DRY_VALUE,
    SOIL_WET_VALUE,
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
// SEND STATUS TO TELEGRAM
// =====================================================

void sendStatus() {

  int soilRaw = analogRead(SOIL_PIN);

  int moisture = map(
    soilRaw,
    SOIL_DRY_VALUE,
    SOIL_WET_VALUE,
    0,
    100
  );

  moisture = constrain(
    moisture,
    0,
    100
  );

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  String message = "";

  message += "🌱 *SMART IRRIGATION STATUS*\n";
  message += "━━━━━━━━━━━━━━━━━━\n\n";

  // Soil
  message += "🌱 Soil Moisture: ";
  message += String(moisture);
  message += "%\n";

  message += "🔢 Raw Soil Value: ";
  message += String(soilRaw);
  message += "\n\n";

  // DHT11
  if (!isnan(temperature)) {

    message += "🌡 Temperature: ";
    message += String(temperature, 1);
    message += " °C\n";

  } else {

    message += "🌡 Temperature: Sensor Error\n";
  }

  if (!isnan(humidity)) {

    message += "💧 Humidity: ";
    message += String(humidity, 1);
    message += "%\n\n";

  } else {

    message += "💧 Humidity: Sensor Error\n\n";
  }

  // Soil condition
  if (moisture < DRY_THRESHOLD) {

    message += "⚠️ Soil Status: DRY\n";
    message += "🚰 Pump Status: ON\n";
    message += "🔴 Irrigation: ACTIVE\n";

  } else {

    message += "✅ Soil Status: MOIST\n";
    message += "🚰 Pump Status: OFF\n";
    message += "🟢 Irrigation: NOT REQUIRED\n";
  }

  message += "\n📡 Wi-Fi: ";

  if (WiFi.status() == WL_CONNECTED) {
    message += "Connected";
  } else {
    message += "Disconnected";
  }

  message += "\n\n🤖 ESP32 Smart Irrigation";

  bot.sendMessage(
    CHAT_ID,
    message,
    "Markdown"
  );

  Serial.println("Status sent to Telegram");
}


// =====================================================
// HANDLE TELEGRAM MESSAGES
// =====================================================

void handleTelegramMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    Serial.print("Telegram message: ");
    Serial.println(text);

    // -----------------------------------------------
    // STATUS COMMAND
    // -----------------------------------------------

    if (text == "/status") {

      sendStatus();
    }

    // -----------------------------------------------
    // START COMMAND
    // -----------------------------------------------

    else if (text == "/start") {

      String welcome = "";

      welcome += "🌱 *SMART IRRIGATION BOT*\n\n";

      welcome += "Available commands:\n\n";

      welcome += "📊 /status\n";
      welcome += "Get current sensor values\n\n";

      welcome += "🌱 Soil Moisture\n";
      welcome += "🌡 Temperature\n";
      welcome += "💧 Humidity\n";
      welcome += "🚰 Pump Status\n";
      welcome += "📡 Wi-Fi Status";

      bot.sendMessage(
        chat_id,
        welcome,
        "Markdown"
      );
    }
  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ===================================================
  // PINS
  // ===================================================

  pinMode(SOIL_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Pump OFF initially
  digitalWrite(RELAY_PIN, HIGH);

  digitalWrite(BUZZER_PIN, LOW);
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
  // DHT
  // ===================================================

  dht.begin();

  // ===================================================
  // WIFI
  // ===================================================

  WiFi.begin(
    ssid,
    password
  );

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ===================================================
  // TELEGRAM
  // ===================================================

  client.setInsecure();

  delay(1000);

  bot.sendMessage(
    CHAT_ID,
    "🌱 *Smart Irrigation System Started!*\n\n"
    "ESP32 is connected.\n\n"
    "Send /status to get all current sensor values.",
    "Markdown"
  );

  delay(2000);
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // READ SOIL
  // ===================================================

  int soilRaw = analogRead(SOIL_PIN);

  int moisture = map(
    soilRaw,
    SOIL_DRY_VALUE,
    SOIL_WET_VALUE,
    0,
    100
  );

  moisture = constrain(
    moisture,
    0,
    100
  );


  // ===================================================
  // READ DHT
  // ===================================================

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();


  // ===================================================
  // PUMP CONTROL
  // ===================================================

  bool soilIsDry = moisture < DRY_THRESHOLD;


  if (soilIsDry) {

    // Pump ON
    digitalWrite(RELAY_PIN, LOW);

    // Red LED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    // Buzzer
    digitalWrite(BUZZER_PIN, HIGH);

    Serial.println("SOIL DRY - PUMP ON");

  }

  else {

    // Pump OFF
    digitalWrite(RELAY_PIN, HIGH);

    // Green LED
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);

    // Buzzer OFF
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("SOIL MOIST - PUMP OFF");
  }


  // ===================================================
  // AUTOMATIC TELEGRAM ALERT
  // ===================================================

  if (firstReading || soilIsDry != previousDryStatus) {

    String message = "";

    if (soilIsDry) {

      message += "⚠️ *SOIL DRY ALERT*\n\n";
      message += "🌱 Moisture: ";
      message += String(moisture);
      message += "%\n";

      if (!isnan(temperature)) {

        message += "🌡 Temperature: ";
        message += String(temperature, 1);
        message += " °C\n";
      }

      if (!isnan(humidity)) {

        message += "💧 Humidity: ";
        message += String(humidity, 1);
        message += "%\n";
      }

      message += "\n🚰 Water Pump: ON";

    }

    else {

      message += "✅ *SOIL MOISTURE NORMAL*\n\n";
      message += "🌱 Moisture: ";
      message += String(moisture);
      message += "%\n";

      if (!isnan(temperature)) {

        message += "🌡 Temperature: ";
        message += String(temperature, 1);
        message += " °C\n";
      }

      if (!isnan(humidity)) {

        message += "💧 Humidity: ";
        message += String(humidity, 1);
        message += "%\n";
      }

      message += "\n🚰 Water Pump: OFF";
    }

    bot.sendMessage(
      CHAT_ID,
      message,
      "Markdown"
    );

    previousDryStatus = soilIsDry;
    firstReading = false;
  }


  // ===================================================
  // CHECK TELEGRAM
  // ===================================================

  if (millis() - lastTelegramCheck > telegramInterval) {

    int numNewMessages = bot.getUpdates(
      bot.last_message_received + 1
    );

    while (numNewMessages) {

      handleTelegramMessages(numNewMessages);

      numNewMessages = bot.getUpdates(
        bot.last_message_received + 1
      );
    }

    lastTelegramCheck = millis();
  }


  // ===================================================
  // OLED
  // ===================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(10, 0);
  display.println("SMART IRRIGATION");

  display.setCursor(0, 15);
  display.print("Soil: ");
  display.print(moisture);
  display.println("%");

  display.setCursor(0, 27);
  display.print("Temp: ");

  if (isnan(temperature)) {
    display.println("Error");
  }
  else {
    display.print(temperature, 1);
    display.println(" C");
  }

  display.setCursor(0, 39);
  display.print("Humidity: ");

  if (isnan(humidity)) {
    display.println("Error");
  }
  else {
    display.print(humidity, 1);
    display.println("%");
  }

  display.setCursor(0, 53);

  if (soilIsDry) {
    display.print("Pump: ON");
  }
  else {
    display.print("Pump: OFF");
  }

  display.display();


  // ===================================================
  // DELAY
  // ===================================================

  delay(2000);
}