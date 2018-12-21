
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>
#include <ESP8266WiFi.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>






ESP8266WiFiMulti wifiMulti;
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

void setup()   {
  Serial.begin(115200);
  Serial.println("Hello");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  delay(1000);
  display.clearDisplay();
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  WiFiManager wifiManager;

  display.println("Connecting...");
  display.display();
  wifiManager.autoConnect("AutoConnectAP");
  display.println("Connected!");
  display.display();
  delay(1000);
  display.clearDisplay();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

}

int16_t counter = 0;

void loop() {
  int16_t adc0, adc1;
  float t1, t2;

  adc0 = ads.readADC_SingleEnded(0);
  Serial.println("ch1 :" + String(adc0));
  adc1 = ads.readADC_SingleEnded(1);
  Serial.println("ch2 :" + String(adc1));

  display.setCursor(0, 0);
  display.clearDisplay();

  t1 = calcTemperture(adc0);
  display.setTextSize(1);
  display.println("T1 :");
  display.setTextSize(2);
  display.println(String(t1, 1));

  t2 = calcTemperture(adc1);
  display.setTextSize(1);
  display.println("T2 :" );
  display.setTextSize(2);
  display.println(String(t2, 1));

  display.display();

  if (counter > 6) {
    Serial.println("updating to cloud");
    updateDataToCloud(t1, t2);
    counter = 0;
  }
  counter++;
  delay(500);
}

float calcTemperture(int16_t adc) {
  float lanVal = (((float)26150 / (float)adc) - 1) / 10 ;
  float lanItem = 0.000253 * log(lanVal) ;
  float temp = 1 / (0.00335 + lanItem) - 273 ;
  return temp;
}

void updateDataToCloud(float temperture1, float temperture2) {

  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin("http://us-central1-smokingcontroller.cloudfunctions.net/multiTempUpdate?t1=" + String(temperture1) + "&t2=" + String(temperture2)); //HTTP
    int httpCode = http.GET();
    Serial.println("httpCode:" +  String(httpCode));
    http.end();
  }
}

