
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>
#include <ESP8266WiFi.h>
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "constants.h"
#include <Fonts/Picopixel.h>




Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
WiFiManager wifiManager;
int reconnections = 0;
String ssidNew  ;
String passNew  ;
String sessionID ;

void setup()   {
  Serial.begin(115200);
  Serial.println("Hello");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  display.clearDisplay();
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();
  display.setRotation(1);
  display.setFont(&Picopixel);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);

  display.println("Connecting...");
  display.display();
  wifiManager.autoConnect("SmokerAP");
  ssidNew = WiFi.SSID();
  passNew = WiFi.psk();

  Serial.println("connected. :)" + ssidNew + " password: " + passNew);

  display.println("Connected! ");
  display.display();
  sessionID = initSessionOnCloud();
  delay(1000);
  display.clearDisplay();
  //display.setFont(&FreeSerifItalic9pt7b);

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

}

int16_t counter = 0;
float t1sum = 0;
float t2sum = 0;
float t3sum = 0;
float t4sum = 0;

void loop() {
  int16_t adc0, adc1, adc2, adc3;
  float t1, t2, t3, t4;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);


  t1 = calcTemperture(adc0);
  t1sum += t1;

  t2 = calcTemperture(adc1);
  t2sum += t2;

  t3 = calcTemperture(adc2);
  t3sum += t3;

  t4 = calcTemperture(adc3);
  t4sum += t4;

  display.setCursor(0, 10);
  display.clearDisplay();


  printTemp(" T1:  " , t1);
  printTemp(" T2:  " , t2);
  printTemp(" T3:  " , t3);
  printTemp(" T4:  " , t4);


  Serial.println("T1 :" + String(t1));
  Serial.println("T2 :" + String(t2));

  Serial.println("T3 :" + String(t3));
  Serial.println("T4 :" + String(t4));


  display.setTextSize(1);
  display.println(String(reconnections, 1));

  display.display();

  if (counter >= 16) {
    display.fillCircle(43, 60, 2, WHITE);
    display.display();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("updating to cloud");
    updateDataToCloud(t1sum / (float)16, t2sum / (float)16, t3sum / (float)16, t4sum / (float)16);
    digitalWrite(LED_BUILTIN, HIGH);
    counter = 0;
    t1sum = 0;
    t2sum = 0;
    t3sum = 0;
    t4sum = 0;
  }
  counter++;
  delay(500);
}

void printTemp(String label, float t) {
  display.setTextSize(1);
  display.print(label);
  display.setTextSize(2);
  display.println(String(t, 1));

}

float calcTemperture(int16_t adc) {
  float lanVal = (((float)26150 / (float)adc) - 1) / 10 ;
  float lanItem = 0.000253 * log(lanVal) ;
  float temp = 1 / (0.00335 + lanItem) - 273 ;
  if (temp < -1)
    temp = -1;
  if (isnan(temp))
    temp = -2;
  return temp;
}
String initSessionOnCloud() {
  Serial.println("initSessionOnCloud");
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String httpAddress = "http://" + BASE_URL + "/initSession";
    http.begin(httpAddress); //HTTP
    int httpCode = http.GET();
    Serial.println("httpCode:" +  String(httpCode));

    String payload = http.getString();
    Serial.println("payload:");
    Serial.println(payload);
    http.end();
    return payload;
  } else {
    WiFi.begin((const char*)ssidNew.c_str(), (const char*)passNew.c_str() );
    reconnections++;
    return "";
  }
}
void updateDataToCloud(float temperture1, float temperture2, float temperture3, float temperture4) {

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    String httpAddress = "http://" + BASE_URL + "/multiTempUpdate?t1=" + String(temperture1) + "&t2=" + String(temperture2)+ "&t3=" + String(temperture3) + "&t4=" + String(temperture4)+ "&sessionID=" + sessionID;
    http.begin(httpAddress); //HTTP
    int httpCode = http.GET();
    Serial.println("httpCode:" +  String(httpCode));
    http.end();
  } else {
    WiFi.begin((const char*)ssidNew.c_str(), (const char*)passNew.c_str() );
    reconnections++;
  }
}
