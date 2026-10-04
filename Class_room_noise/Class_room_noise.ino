#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// =====================================================
// Wi-Fi
// =====================================================
const char* ssid = "G";
const char* password = "123456789";

// =====================================================
// Telegram
// =====================================================
#define BOT_TOKEN "8609914042:AAF911Y9urEmpPHsXSL-yVSNmEDqRR_f-HU"
#define CHAT_ID "1367693706"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// =====================================================
// Pins
// =====================================================
#define SOUND_PIN 15

#define GREEN_LED 25
#define RED_LED   26
#define BUZZER    27

// =====================================================
// Settings
// =====================================================

// Telegram alert every 2 seconds while noise continues
const unsigned long messageInterval = 2000;

unsigned long lastMessageTime = 0;

// Keeps track of current condition
bool noiseActive = false;


void setup() {

  Serial.begin(115200);

  // Pins
  pinMode(SOUND_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initial condition = Silent
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // ===================================================
  // Wi-Fi
  // ===================================================

  Serial.println();
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Telegram HTTPS
  client.setInsecure();

  Serial.println("================================");
  Serial.println("CLASSROOM NOISE ALARM READY");
  Serial.println("================================");
}


void loop() {

  // ===================================================
  // Read HW-484 Digital Output
  // ===================================================

  int soundState = digitalRead(SOUND_PIN);


  // ===================================================
  // NOISE DETECTED
  // ===================================================

  if (soundState == HIGH) {

    Serial.println("🔊 NOISE DETECTED");

    // LED control
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    // Buzzer ON
    digitalWrite(BUZZER, HIGH);


    // =================================================
    // First noise detection
    // =================================================

    if (noiseActive == false) {

      noiseActive = true;

      // Send first alert immediately
      bot.sendMessage(
        CHAT_ID,
        "⚠️ CLASSROOM NOISE ALERT!\n\n🔊 Noise detected!\n🔴 Please maintain silence.",
        ""
      );

      Serial.println("📱 Noise alert sent");

      lastMessageTime = millis();
    }


    // =================================================
    // Continue sending while noise exists
    // =================================================

    else {

      if (millis() - lastMessageTime >= messageInterval) {

        bot.sendMessage(
          CHAT_ID,
          "⚠️ CLASSROOM NOISE ALERT!\n\n🔊 Noise is still being detected!",
          ""
        );

        Serial.println("📱 Noise continues - message sent");

        lastMessageTime = millis();
      }
    }
  }


  // ===================================================
  // SILENT / NO NOISE
  // ===================================================

  else {

    Serial.println("🤫 CLASS IS SILENT");

    // LED control
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    // Buzzer OFF
    digitalWrite(BUZZER, LOW);


    // =================================================
    // Noise just stopped
    // =================================================

    if (noiseActive == true) {

      noiseActive = false;

      // Send ONE silent message
      bot.sendMessage(
        CHAT_ID,
        "✅ CLASS IS SILENT\n\n🟢 Noise level is back to normal.",
        ""
      );

      Serial.println("📱 Class is silent message sent");

      // Reset timer
      lastMessageTime = 0;
    }
  }


  delay(200);
}