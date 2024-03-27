#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <DHT.h>

const char *ssid = "Anandan";
const char *password = "12345678";

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4); // Adjust the I2C address if necessary
#define DOUT  23
#define CLK  19
#define BUZZER 25
HX711 scale;
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);
  Serial.println("Remove all weight from scale");
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare();
  long zero_factor = scale.read_average();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Print IP address on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  measureTemperature();
  measureWeight();
}

void handleRoot() {
  float temperature = dht.readTemperature();
  // Ensure positive value for temperature
  temperature = abs(temperature);

  float weight = scale.get_units(5);
  // Ensure positive value for weight
  weight = abs(weight);

  int liter = weight * 470;
  int val = map(liter, 0, 505, 0, 100);

  String page = "<html>\
    <head>\
      <meta http-equiv='refresh' content='5'/>\
      <meta name='viewport' content='width=device-width, initial-scale=1'>\
      <title>ESP32 IV Bag Monitor</title>\
      <style>\
        body { font-family: Arial, sans-serif; background-color: #f0f0f0; }\
        h1 { text-align: center; }\
        .reading { margin: 20px auto; max-width: 400px; background-color: #fff; border-radius: 10px; padding: 20px; box-shadow: 0px 0px 10px rgba(0,0,0,0.1); }\
        .gauge { position: relative; width: 100px; height: 100px; margin: 0 auto; }\
        .gauge .mask { position: absolute; top: 0; left: 0; width: 100%; height: 100%; border-radius: 50%; overflow: hidden; }\
      </style>\
      <script>\
        function updateGauge(percent) {\
          var fill = document.querySelector('.fill');\
          fill.style.transform = 'rotate(' + (percent * 1.8 - 45) + 'deg)';\
        }\
      </script>\
    </head>\
    <body>\
      <h1>ESP32 IV Bag Monitor</h1>\
      <div class='reading'>\
        <h2>Temperature</h2>\
        <p>Temperature: " + String(temperature) + " &deg;C</p>\
      </div>\
      <div class='reading'>\
        <h2>IV Bag</h2>\
        <p>Liters: " + String(liter) + " mL</p>\
        <div class='gauge'>\
          <div class='mask'></div>\
        </div>\
        <p>Percent: " + String(val) + " %</p>\
      </div>\
    </body>\
  </html>";

  server.send(200, "text/html", page);

  // Execute JavaScript to update gauge
  server.sendContent("<script>updateGauge(" + String(val) + ");</script>");

  // Display the temperature, liters, and percentage values on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Liters: ");
  lcd.print(liter);
  lcd.print("mL");
  lcd.setCursor(0, 2);
  lcd.print("Percent: ");
  lcd.print(val);
  lcd.print("%");
}

void measureWeight() {
  float calibration_factor = 102500;
  scale.set_scale(calibration_factor);
}

void measureTemperature() {
  float tempC = dht.readTemperature();
  if (isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}
