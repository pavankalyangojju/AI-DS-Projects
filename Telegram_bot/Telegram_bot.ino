#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// ================= WIFI =================
const char* ssid = "G";
const char* password = "123456789";

// ================= TELEGRAM =================
#define BOT_TOKEN "8941366457:AAEmLqu74wW7vewbHupUil_zjCd1bms-Gfo"
#define CHAT_ID "1367693706"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// ================= LED =================
#define LED_PIN 2

bool ledState = false;

unsigned long lastTime = 0;
const unsigned long checkInterval = 1000;

// =====================================================
// HANDLE TELEGRAM MESSAGES
// =====================================================
void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    // Security: accept commands only from your Telegram ID
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user!", "");
      continue;
    }

    // ================= LED ON =================
    if (text == "/on")
    {
      digitalWrite(LED_PIN, HIGH);
      ledState = true;

      bot.sendMessage(chat_id, "💡 LED is ON", "");
    }

    // ================= LED OFF =================
    else if (text == "/off")
    {
      digitalWrite(LED_PIN, LOW);
      ledState = false;

      bot.sendMessage(chat_id, "🔴 LED is OFF", "");
    }

    // ================= STATUS =================
    else if (text == "/status")
    {
      if (ledState)
      {
        bot.sendMessage(chat_id, "💡 LED Status: ON", "");
      }
      else
      {
        bot.sendMessage(chat_id, "🔴 LED Status: OFF", "");
      }
    }

    // ================= START =================
    else if (text == "/start")
    {
      String message = "🤖 ESP32 LED Control\n\n";
      message += "/on - Turn LED ON\n";
      message += "/off - Turn LED OFF\n";
      message += "/status - Check LED status";

      bot.sendMessage(chat_id, message, "");
    }
  }
}

// =====================================================
// SETUP
// =====================================================
void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Connect WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Telegram HTTPS
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  Serial.println("Telegram Bot Ready");

  bot.sendMessage(CHAT_ID,
                  "🤖 ESP32 LED Control Started!\n\n"
                  "Use /on, /off or /status",
                  "");
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
  if (millis() - lastTime > checkInterval)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("New Telegram message received");

      handleNewMessages(numNewMessages);

      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTime = millis();
  }
}
