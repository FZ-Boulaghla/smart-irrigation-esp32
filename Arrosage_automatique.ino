#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <time.h>

#define RELAY_PIN 4
#define SENSOR_PIN 33

const char* ssid = "";
const char* password = "";

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;          // Maroc = UTC+1
const int daylightOffset_sec = 0;

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- POST /api/pump ---
void handlePumpControl() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String body = server.arg("plain");
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error || !doc.containsKey("state") || !doc["state"].is<bool>()) {
    server.send(400, "application/json", "{\"error\":\"Invalid or missing 'state' boolean\"}");
    return;
  }

  bool turnOn = doc["state"];

  if (turnOn) {
    digitalWrite(RELAY_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Pump: ON       ");
    server.send(200, "application/json", "{\"status\": \"Pump ON\"}");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Pump: OFF      ");
    server.send(200, "application/json", "{\"status\": \"Pump OFF\"}");
  }
}

// --- GET /api/soilMoisture ---
void handleSoilMoisture() {
  int value = analogRead(SENSOR_PIN);
  value = map(value, 0, 4095, 0, 100);
  value = (value - 100) * -1;

  String json = "{\"moisture\": " + String(value) + "}";
  server.send(200, "application/json", json);
}

// --- Initialisation NTP ---
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP...");
    delay(1000);
  }
  Serial.println("Time synchronized");
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pompe OFF

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi...");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  Serial.println(WiFi.localIP());

  setupTime();

  server.on("/api/pump", HTTP_POST, handlePumpControl);
  server.on("/api/soilMoisture", HTTP_GET, handleSoilMoisture);
  server.begin();
}

void loop() {
  server.handleClient();

  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate > 5000) {
    // Affichage humidité
    int value = analogRead(SENSOR_PIN);
    value = map(value, 0, 4095, 0, 100);
    value = (value - 100) * -1;

    // Heure actuelle
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[17];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

      lcd.setCursor(0, 0);
      lcd.print(timeStr);          // Affiche l'heure sur ligne 0
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Time error     ");
    }

    // Affichage humidité ligne 1
    lcd.setCursor(8, 0);
    lcd.print("H:");
    lcd.print(value);
    lcd.print("% ");

    lastUpdate = now;
  }
}
