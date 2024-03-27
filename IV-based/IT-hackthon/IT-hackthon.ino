#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3VhzV9TKu"
#define BLYNK_TEMPLATE_NAME "IoT based IV bag monitoring system"

#include <WiFi.h>
#include <WiFiClient.h>
#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
HX711 scale;
DHT dht_sensor(21, DHT11);

char auth[] = "7o2wTXUgn-gXNZqc6XTurFKeuZzJ0KO9"; // Blynk authentication token
char ssid[] = "Anandan";
char pass[] = "2257anand";

#define DOUT  23
#define CLK  19
#define BUZZER 25

int liter;
int val;
float weight;
float calibration_factor = 102500; // Change this value for your Load cell sensor

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);
  Serial.println("Remove all weight from scale");
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); // Reset the scale to 0
  long zero_factor = scale.read_average(); // Get a baseline reading
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht_sensor.begin();
}

void loop() {
  Blynk.run();
  measureWeight();
  readTemperature();
}

void measureWeight() {
  scale.set_scale(calibration_factor); // Adjust to this calibration factor
  weight = scale.get_units(5);
  if (weight < 0) {
    weight = 0.00;
  }
  liter = weight * 474;
  val = liter;
  val = map(val, 0, 505, 0, 100);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IOT Based IV Bag");
  lcd.setCursor(2, 1);
  lcd.print("Monitoring System");
  Serial.print("Kilogram: ");
  Serial.print(weight);
  Serial.println(" Kg");
  lcd.setCursor(1, 2);
  lcd.print("IV Bottle = ");
  lcd.print(liter);
  lcd.print(" mL");
  Serial.print("IV BOTTLE: ");
  Serial.print(liter);
  Serial.println("mL");
  lcd.setCursor(1, 3);
  lcd.print("IV Bag Percent=");
  lcd.print(val);
  lcd.print("%");
  Serial.print("IV Bag Percent: ");
  Serial.print(val);
  Serial.println("%");
  Serial.println();
  delay(500);
  if (val <= 50 && val >= 40) {
    Blynk.logEvent("iv_alert", "IV Bottle is 50%");
    digitalWrite(BUZZER, HIGH);
    delay(50);
    digitalWrite(BUZZER, LOW);
    delay(50);
  }
  else if (val <= 20) {
    Blynk.logEvent("iv_alert", "IV Bottle is too LOW");
    digitalWrite(BUZZER, HIGH);
  }
  else {
    digitalWrite(BUZZER, LOW);
  }
  Blynk.virtualWrite(V0, liter);
  Blynk.virtualWrite(V1, val);
}

void readTemperature() {
  float tempC = dht_sensor.readTemperature();
  if (isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
    Blynk.virtualWrite(V2, tempC);
  }
  delay(2000);
}
