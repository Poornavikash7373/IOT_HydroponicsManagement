#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ==== CONFIGURATION ====
// WiFi
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// MQTT
const char* mqtt_server = "broker.hivemq.com";
const char* topic = "hydroponics/data";

// Pins
#define DHTPIN 4       // DHT sensor pin
#define DHTTYPE DHT22
#define PH_PIN 34      // Analog pin for pH
#define EC_PIN 35      // Analog pin for EC

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

// ==== WiFi Connection ====
void setup_wifi() {
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// ==== MAIN LOOP ====
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int ph_raw = analogRead(PH_PIN);
  int ec_raw = analogRead(EC_PIN);

  float ph_val = (ph_raw / 4095.0) * 14.0;       // Calibrate as needed
  float ec_val = (ec_raw / 4095.0) * 5.0;        // Calibrate as needed

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" | Hum: "); Serial.print(hum);
  Serial.print(" | pH: "); Serial.print(ph_val);
  Serial.print(" | EC: "); Serial.println(ec_val);

  String payload = "{\"temp\":" + String(temp, 1) +
                   ",\"hum\":" + String(hum, 1) +
                   ",\"ph\":" + String(ph_val, 2) +
                   ",\"ec\":" + String(ec_val, 2) + "}";

  client.publish(topic, payload.c_str());

  delay(10000); // Log every 10 seconds
}

// ==== MQTT Reconnect ====
void reconnect() {
  while (!client.connected()) {
    if (client.connect("hydroponicsClient")) break;
    delay(1000);
  }
}
