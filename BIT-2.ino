#define BLYNK_PRINT Serial
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3VhzV9TKu"
#define BLYNK_TEMPLATE_NAME "IoT based IV bag monitoring system"
#include <WiFi.h>
#include <WiFiClient.h>
#include <Blynk.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <DHT.h> // Include the DHT library
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define DOUT  23
#define CLK  19
#define BUZZER 25
HX711 scale;
#define BLYNK_PRINT Serial

char auth[] = "UlA9Ze_mj9kG4-LJw6rao5HCJPNC2wqg";
char ssid[] = "Anandan";
char pass[] = "12345678";

int liter;
int val;
float weight; 
float calibration_factor = 102500; // change this value for your Load cell sensor 

#define DHTPIN 18  // Pin where the DHT sensor is connected
#define DHTTYPE DHT11 // Type of DHT sensor, change to DHT22 or DHT21 if you're using a different sensor
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor

void setup() {
  // Set up serial monitor
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER ,OUTPUT);
  Serial.println("Remove all weight from scale");
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); 
  Serial.println(zero_factor);
  Blynk.begin(auth, ssid, pass); 

  dht.begin(); // Initialize the DHT sensor
}
 void loop() {
 Blynk.run();
 measureweight();
 measureTemperature(); // Call the function to measure temperature
}
 
void measureweight(){
 scale.set_scale(calibration_factor); //Adjust to this calibration factor
  weight = scale.get_units(5); 
    if(weight<0)
  {
    weight=0.00;
    }
  liter = weight*470;
  val = liter;
  val = map(val, 0, 505, 0, 100);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Tech Titans-IV Bag");
  lcd.setCursor(2, 1);
  lcd.print("Monitering System");
  Serial.print("Kilogram: ");
  Serial.print(weight); 
  Serial.println(" Kg");
  lcd.setCursor(1, 2);
  lcd.print("IV liters = ");
  lcd.print(liter);
  lcd.print(" mL");
  Serial.print("IV BOTTLE: ");
  Serial.print(liter);
  Serial.println("mL");
  lcd.setCursor(1, 3);
  lcd.print("IV percent = ");
  lcd.print(val);
  lcd.print("%");
  Serial.print("IV Bag Percent: ");
  Serial.print(val);
  Serial.println("%");
  Serial.println();
  delay(500);
  if (val <= 50 && val >= 40){
    Blynk.logEvent("iv_alert","IV Bottle is 50%");
  }
  else if (val <= 20 && val >10){
    Blynk.logEvent("iv_alert","IV Bottle is 20%");

  }
  else if (val <= 10 && val >1){
    Blynk.logEvent("iv_alert","IV Bottle is too Low");
    digitalWrite(BUZZER, HIGH);
    delay(0);
  }
  else{
    digitalWrite(BUZZER, LOW);
  }
  Blynk.virtualWrite(V0,liter);
  Blynk.virtualWrite(V1,val);
}

void measureTemperature() {
  float tempC = dht.readTemperature(); // Read temperature in Celsius
  if (isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.println(" °C");

  Blynk.virtualWrite(V2, tempC); // Send temperature data to Blynk
}
 