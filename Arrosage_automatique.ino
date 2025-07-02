#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#define RELAY_PIN 4
#define SENSOR_PIN 33

const char* ssid = "";
const char* password = "";

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

  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  bool turnOn = doc["state"];

  if (turnOn) {
    digitalWrite(RELAY_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Motor is ON ");
    server.send(200, "application/json", "{\"status\": \"Pump ON\"}");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Motor is OFF");
    server.send(200, "application/json", "{\"status\": \"Pump OFF\"}");
  }
}

// --- GET /api/soilMoisture ---
void handleSoilMoisture() {
  int value = analogRead(SENSOR_PIN);
  value = map(value, 0, 4095, 0, 100);
  value = (value - 100) * -1;  // Inverser pour afficher "humide" = 100%

  String json = "{\"moisture\": " + String(value) + "}";
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(value);
  lcd.print("%   ");
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Pompe éteinte au début

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi...");
  }

  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  // Routes HTTP REST
  server.on("/api/pump", HTTP_POST, handlePumpControl);
  server.on("/api/soilMoisture", HTTP_GET, handleSoilMoisture);

  server.begin();
}

void loop() {
  server.handleClient();

  // Affichage régulier de l’humidité sur le LCD (toutes les 5 secondes)
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate > 5000) {
    int value = analogRead(SENSOR_PIN);
    value = map(value, 0, 4095, 0, 100);
    value = (value - 100) * -1;

    lcd.setCursor(0, 0);
    lcd.print("Moisture: ");
    lcd.print(value);
    lcd.print("%   "); // Nettoie les vieux chiffres

    lastUpdate = now;
  }
}
