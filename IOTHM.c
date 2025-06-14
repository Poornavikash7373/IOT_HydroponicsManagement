#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi & Blynk credentials
char auth[] = "Your_Blynk_Auth_Token";
char ssid[] = "Your_WiFi_SSID";
char pass[] = "Your_WiFi_Password";

// Pin definitions
#define DHTPIN 4          // DHT11 data pin
#define DHTTYPE DHT11
#define TDS_PIN 34        // Analog pin for TDS sensor
#define PH_PIN 35         // Optional: Analog pin for pH sensor

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  dht.begin();
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  Blynk.run();

  // Read temperature
  float tempC = dht.readTemperature();
  if (isnan(tempC)) {
    Serial.println("Failed to read temperature!");
    return;
  }

  // Read raw EC value from analog pin
  int rawTDS = analogRead(TDS_PIN);
  float voltage = (rawTDS / 4095.0) * 3.3;       // ESP32 is 3.3V ADC
  float ec = voltage * 1000.0;                   // Scale factor for your TDS sensor

  // Temperature compensation
  const float tempCoefficient = 0.019;           // For nutrient solutions
  float ec_25 = ec / (1 + tempCoefficient * (tempC - 25.0));  // EC @ 25°C

  // Convert to TDS (ppm)
  float tds = ec_25 * 0.64;  // Depends on your meter (0.5 - 0.7 range)

  // Read pH sensor if connected
  int rawPH = analogRead(PH_PIN);
  float phVoltage = (rawPH / 4095.0) * 3.3;
  float pH = 3.5 * phVoltage;   // Adjust based on your sensor calibration

  // Logging to serial
  Serial.print("Temp: "); Serial.print(tempC); Serial.print(" °C | ");
  Serial.print("EC: "); Serial.print(ec_25, 2); Serial.print(" µS/cm | ");
  Serial.print("TDS: "); Serial.print(tds, 2); Serial.print(" ppm | ");
  Serial.print("pH: "); Serial.println(pH, 2);

  // Sending to Blynk (use your virtual pins)
  Blynk.virtualWrite(V0, tempC);
  Blynk.virtualWrite(V1, tds);
  Blynk.virtualWrite(V2, pH);

  delay(3000);  // Read every 3 seconds
}
