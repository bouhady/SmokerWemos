
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
#include <ArduinoJson.h>


#include <WiFiClient.h>
#include "WEMOS_Motor.h"


WiFiClient wifiClient;
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
WiFiManager wifiManager;
int reconnections = 0;
String ssidNew  ;
String passNew  ;
String sessionID ;

int motorCommandId = 0;
int motorCountdown = 0;
int motorValue = 0;


StaticJsonDocument<512> doc;

Motor M1(0x30, _MOTOR_A, 1000); //Motor A

void setup()   {
  Serial.begin(115200);
  Serial.println("Hello");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  //display.display();
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  display.clearDisplay();

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
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  ads.begin();

  delay(500);
  display.clearDisplay();

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
}

int16_t counter = 0;
float t1sum = 0;
float t2sum = 0;
float t3sum = 0;
float t4sum = 0;

void loop() {

  display.setCursor(0, 10);
  display.clearDisplay();
  int16_t adc0, adc1, adc2, adc3;
  float t1, t2, t3, t4;

  Serial.print("read...");

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);

  Serial.println(" .done..");
  t1 = calcTemperture(adc0);
  t1sum += t1;

  t2 = calcTemperture(adc1);
  t2sum += t2;

  t3 = calcTemperture(adc2);
  t3sum += t3;

  t4 = calcTemperture(adc3);
  t4sum += t4;


  printTemp(" T1:  " , t1);
  printTemp(" T2:  " , t2);
  printTemp(" T3:  " , t3);
  printTemp(" T4:  " , t4);


  Serial.print("T1 :" + String(t1));
  Serial.print(", T2 :" + String(t2));
  Serial.print(", T3 :" + String(t3));
  Serial.println(", T4 :" + String(t4));


  display.setTextSize(1);
  display.println(String(reconnections, 1));
  display.drawLine(0, (4 * counter), 0, 64 , WHITE);
  display.display();

  if (counter >= 16) {

    display.drawChar(2, 63, 'N', WHITE, BLACK, 1);
    display.display();
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("updating to cloud");
    String payload = updateDataToCloud(t1sum / (float)16, t2sum / (float)16, t3sum / (float)16, t4sum / (float)16);
    digitalWrite(LED_BUILTIN, HIGH);
    counter = 0;
    t1sum = 0;
    t2sum = 0;
    t3sum = 0;
    t4sum = 0;

    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    } else {
      int fanDuration = doc["fanDuration"];
      Serial.print("fanDuration : " );
      Serial.println(fanDuration);

      int fanState = doc["fanState"];
      Serial.print("fanState : ");
      Serial.println(fanState);

      int id = doc["id"];
      Serial.print("id : ");
      Serial.print(id);
      Serial.print(" motorCommandId : ");
      Serial.println(motorCommandId);

      if (id > motorCommandId) {
        Serial.print("Set Motor : ");
        Serial.println(fanState);
        motorCommandId = id;
        motorValue = fanState;
        motorCountdown = fanDuration;
      }
    }

  }

  if (motorCountdown > 0) {
    Serial.println("motorCountdown : " + String(motorCountdown));
    display.drawChar(44, 63, 'F', WHITE, BLACK, 1);
    display.display();
    motorCountdown = motorCountdown - 500;
  } else {
    motorValue = 0;
  }


  setMotorState(motorValue);
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
    http.begin(wifiClient, httpAddress); //HTTP
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
String updateDataToCloud(float temperture1, float temperture2, float temperture3, float temperture4) {

  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;
    String httpAddress = "http://" + BASE_URL + "/multiTempUpdate?t1=" + String(temperture1) + "&t2=" + String(temperture2) + "&t3=" + String(temperture3) + "&t4=" + String(temperture4) + "&sessionID=" + sessionID;
    http.begin(wifiClient, httpAddress); //HTTP
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
  }
}

void setMotorState(int state) {
  static int prevState = 101;
  if (prevState != state) {
    if (state > 0) {
      Serial.println("Fan on -> " + String(state));
      M1.setmotor(_CW, state);
    // setmotor(state);
    } else {
      Serial.println("Fan off!");
      M1.setmotor(_STANDBY);
    }
  }

  prevState = state;

}


void setmotor(float pwm_val)
{
  uint16_t _pwm_val;
  byte dataOut[4];
  dataOut[0] = (0 | (byte)0x10);
  dataOut[1] = 2;

  _pwm_val = uint16_t(pwm_val * 100);

  if (_pwm_val > 10000)
    _pwm_val = 10000;
    
  dataOut[2] = (byte)(_pwm_val >> 8);
  dataOut[3] = (byte)_pwm_val;
  Wire.beginTransmission(0x30);
  Wire.write( dataOut, 4);
  int error = Wire.endTransmission(false);     // stop transmitting
Serial.println("i2c : " + String(error));

  delay(100);
}
