// 🔥 ESP8266 IGNITION SYSTEM - PROFESSIONAL EDITION  
// 🧑‍💻 Authorized Developer: Biswajit ⚡
// 🚀 Non-blocking | Stable | Secure

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>

#define IGNITION_PIN 5
#define LED_PIN 2
#define EEPROM_SIZE 512

ESP8266WebServer server(80);
WiFiClientSecure TGclient;
UniversalTelegramBot* bot = nullptr;

// --------- CONFIG STRUCTURE ---------
struct Config {
  char ssid[32];
  char password[32]; 
  char token[64];
  char chatID[32];
} config;

// --------- STATE VARIABLES ---------
bool OTA_enabled = false;
bool ignition_active = false;
bool countdown_active = false;
unsigned long countdown_start = 0;
int countdown_seconds = 0;
unsigned long lastTelegramCheck = 0;
unsigned long lastLEDBlink = 0;
String lastUser = "Unknown";

// --------- EEPROM HANDLING ---------
void saveConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(0, config);
  EEPROM.commit();
  EEPROM.end();
}

void loadConfig() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, config);
  EEPROM.end();
  
  // Default config if empty
  if (strlen(config.ssid) == 0) {
    strcpy(config.ssid, "ESP-SETUP");
    strcpy(config.password, "12345678");
    strcpy(config.token, "");
    strcpy(config.chatID, "");
    saveConfig();
  }
}

// --------- NON-BLOCKING COUNTDOWN ---------
void updateCountdown() {
  if (!countdown_active) return;
  
  unsigned long currentTime = millis();
  int elapsed = (currentTime - countdown_start) / 1000;
  int remaining = countdown_seconds - elapsed;
  
  // LED blinking during countdown
  if (currentTime - lastLEDBlink > 500) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastLEDBlink = currentTime;
  }
  
  // Countdown finished - start ignition
  if (remaining <= 0) {
    countdown_active = false;
    startIgnition();
  }
}

// --------- IGNITION SEQUENCE ---------
void startIgnition() {
  if (bot) bot->sendMessage(config.chatID, "🔥 *IGNITION STARTED*", "Markdown");
  
  ignition_active = true;
  digitalWrite(IGNITION_PIN, HIGH);
  digitalWrite(LED_PIN, LOW); // Solid ON during fire
  
  // Non-blocking fire duration
  countdown_start = millis();
}

void stopIgnition() {
  ignition_active = false;
  digitalWrite(IGNITION_PIN, LOW);
  digitalWrite(LED_PIN, HIGH); // OFF
  
  if (bot) bot->sendMessage(config.chatID, "✅ Ignition Completed - System Safe", "");
}

// --------- STATUS MESSAGE ---------
void sendStatus() {
  if (!bot) return;
  
  String status = 
    "🔥 *ESP8266 Ignition System*\n"
    "👑 Developer: *Biswajit* ⚡\n\n"
    "📡 WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "\n" +
    "🟢 Ignition: " + String(ignition_active ? "ACTIVE" : "Standby") + "\n" +
    "⏳ Countdown: " + String(countdown_active ? "ACTIVE (" + String(countdown_seconds) + "s)" : "Inactive") + "\n" +
    "⚙ OTA: " + String(OTA_enabled ? "Enabled" : "Disabled") + "\n" +
    "👤 Last User: " + lastUser;
    
  bot->sendMessage(config.chatID, status, "Markdown");
}

// --------- SETUP MODE ---------
void setupMode() {
  WiFi.softAP("🔥 ESP SETUP MODE", "12345678");
  
  server.on("/", []() {
    String html = R"raw(
<!DOCTYPE html>
<html>
<head>
  <title>🔥 ESP8266 SETUP</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { background: #000; color: #0f0; font-family: monospace; text-align: center; }
    .container { max-width: 400px; margin: 50px auto; padding: 20px; }
    input { width: 90%; padding: 12px; margin: 8px 0; background: #111; color: #0f0; border: 1px solid #0f0; }
    button { background: #f00; color: #000; padding: 12px 24px; border: none; cursor: pointer; font-weight: bold; }
    button:hover { background: #0f0; }
  </style>
</head>
<body>
  <div class="container">
    <h2>🔥 ESP8266 SETUP MODE</h2>
    <form action="/save">
      <input name="ssid" placeholder="WiFi SSID" value=")raw" + String(config.ssid) + R"raw(">
      <input name="pass" placeholder="WiFi Password" type="password" value=")raw" + String(config.password) + R"raw(">
      <input name="token" placeholder="Bot Token" value=")raw" + String(config.token) + R"raw(">
      <input name="chat" placeholder="Chat ID" value=")raw" + String(config.chatID) + R"raw(">
      <br><br>
      <button>💾 SAVE & RESTART</button>
    </form>
    <p>👑 Authorized Developer: <b>Biswajit</b> ⚡</p>
  </div>
</body>
</html>
    )raw";
    server.send(200, "text/html", html);
  });

  server.on("/save", []() {
    if (server.hasArg("ssid")) strncpy(config.ssid, server.arg("ssid").c_str(), 31);
    if (server.hasArg("pass")) strncpy(config.password, server.arg("pass").c_str(), 31);
    if (server.hasArg("token")) strncpy(config.token, server.arg("token").c_str(), 63);
    if (server.hasArg("chat")) strncpy(config.chatID, server.arg("chat").c_str(), 31);
    
    saveConfig();
    server.send(200, "text/html", "<h3 style='color:#0f0'>✅ Saved! Restarting...</h3>");
    delay(2000);
    ESP.restart();
  });
  
  server.begin();
}

// --------- TELEGRAM COMMAND HANDLER ---------
void handleTelegram() {
  if (!bot) return;
  
  if (millis() - lastTelegramCheck > 1000) {
    lastTelegramCheck = millis();
    
    int msgCount = bot->getUpdates(bot->last_message_received + 1);
    for (int i = 0; i < msgCount; i++) {
      String chatID = bot->messages[i].chat_id;
      String text = bot->messages[i].text;
      String user = bot->messages[i].from_name;
      
      // Authorization check
      if (chatID != config.chatID) {
        bot->sendMessage(chatID, "❌ Unauthorized access");
        continue;
      }
      
      if (user != "") lastUser = user;
      
      // Command processing
      if (text == "/start") {
        bot->sendMessage(chatID, 
          "🔥 *Ignition Control Panel*\n\n"
          "Available commands:\n"
          "/instant - Immediate ignition\n"  
          "/timer5 - 5 second timer\n"
          "/timer10 - 10 second timer\n"
          "/timer15 - 15 second timer\n"
          "/stop - Cancel operation\n"
          "/status - System status\n"
          "/setup - Enter setup mode\n"
          "/ota_on - Enable OTA\n"
          "/ota_off - Disable OTA", "Markdown");
      }
      else if (text == "/instant" && !ignition_active && !countdown_active) {
        startIgnition();
      }
      else if (text.startsWith("/timer") && !ignition_active && !countdown_active) {
        int seconds = text.substring(6).toInt();
        if (seconds >= 1 && seconds <= 60) {
          countdown_active = true;
          countdown_seconds = seconds;
          countdown_start = millis();
          bot->sendMessage(chatID, "⏳ Countdown: " + String(seconds) + " seconds", "");
        }
      }
      else if (text == "/stop") {
        ignition_active = false;
        countdown_active = false;
        digitalWrite(IGNITION_PIN, LOW);
        digitalWrite(LED_PIN, HIGH);
        bot->sendMessage(chatID, "🛑 Operation cancelled", "");
      }
      else if (text == "/status") {
        sendStatus();
      }
      else if (text == "/setup") {
        setupMode();
        bot->sendMessage(chatID, "📡 Setup mode activated", "");
      }
      else if (text == "/ota_on") {
        OTA_enabled = true;
        bot->sendMessage(chatID, "⚙ OTA updates enabled", "");
      }
      else if (text == "/ota_off") {
        OTA_enabled = false;
        bot->sendMessage(chatID, "🔒 OTA updates disabled", "");
      }
    }
  }
}

// --------- OTA SETUP ---------
void setupOTA() {
  ArduinoOTA.setHostname("esp8266-ignition");
  ArduinoOTA.onStart([]() {
    Serial.println("OTA Update Started");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("OTA Update Finished");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
  });
  ArduinoOTA.begin();
}

// --------- SETUP ---------
void setup() {
  Serial.begin(115200);
  Serial.println("🔥 ESP8266 Ignition System Starting...");
  
  pinMode(IGNITION_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(IGNITION_PIN, LOW);
  digitalWrite(LED_PIN, HIGH); // OFF initially
  
  loadConfig();
  
  // Try to connect to WiFi
  if (strlen(config.ssid) > 0 && String(config.ssid) != "ESP-SETUP") {
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    
    Serial.print("Connecting to WiFi");
    for (int i = 0; i < 20; i++) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      Serial.print(".");
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi Connected: " + WiFi.localIP().toString());
      TGclient.setInsecure();
      bot = new UniversalTelegramBot(config.token, TGclient);
      setupOTA();
      
      // Send startup message
      if (bot) {
        bot->sendMessage(config.chatID, "🔥 System Online - IP: " + WiFi.localIP().toString(), "");
      }
    } else {
      Serial.println("WiFi failed - Starting setup mode");
      setupMode();
    }
  } else {
    Serial.println("No config - Starting setup mode");  
    setupMode();
  }
}

// --------- MAIN LOOP ---------
void loop() {
  server.handleClient();
  
  if (WiFi.status() == WL_CONNECTED) {
    handleTelegram();
    if (OTA_enabled) ArduinoOTA.handle();
  }
  
  // Non-blocking state management
  updateCountdown();
  
  // Handle ignition duration (4 seconds)
  if (ignition_active && (millis() - countdown_start >= 4000)) {
    stopIgnition();
  }
  
  // Normal LED blinking when idle
  if (!ignition_active && !countdown_active && WiFi.status() == WL_CONNECTED) {
    if (millis() - lastLEDBlink > 2000) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastLEDBlink = millis();
    }
  }
}
