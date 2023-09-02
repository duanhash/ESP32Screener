#include <WiFi.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  Serial.println("Connecting to WiFi");
  WiFi.begin("Wokwi-GUEST", "", 6);
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.println("U.S. Ticker Symbols Only");
  Serial.setTimeout(5000);
}

void loop() {
  // put your main code here, to run repeatedly:

  String ticker = Serial.readStringUntil('\n');

  if(ticker.length() != 0) {
    WiFiClient client;
    HttpClient http(client);


    int err = http.get("api.twelvedata.com", ("/time_series?symbol=" + ticker + "&interval=1day&outputsize=2&apikey=5acbed4e811542899eda33eea90dbdb2").c_str()); // Don't steal API key C:

    if (err) {
      Serial.println("Error making request");
    }

    http.skipResponseHeaders();
    while (http.available()) {
      DynamicJsonDocument doc(2048);
      auto error = deserializeJson(doc, http);

      if (error) {
        Serial.print("Parsing error: ");
        Serial.println(error.c_str());
        continue;
      }

      serializeJsonPretty(doc, Serial);
      Serial.println();
      Serial.println("---------------------------");
      float todayPrice = doc["values"][0]["close"];
      float yesterdayPrice = doc["values"][1]["close"];
      String name = doc["meta"]["symbol"];
      float priceChange = todayPrice - yesterdayPrice;
      float percentChange = ((todayPrice - yesterdayPrice) / yesterdayPrice) * 100;
      Serial.println(priceChange);
      Serial.println(percentChange);
      lcd.setCursor(0, 0);
      lcd.print("Today:");
      lcd.setCursor(0, 1);
      lcd.print(name + " $" + String(todayPrice));
      lcd.setCursor(0, 2);
      if (priceChange > 0) {
        lcd.print("+" + String(priceChange) + " (" + String(percentChange) + "%)");
      } else {
        lcd.print(String(priceChange) + " (" + String(percentChange) + "%)");
      }
    }
    http.stop();
    delay(100); // this speeds up the simulation
  }
}
