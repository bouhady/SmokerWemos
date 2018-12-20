
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>

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
}


void loop() {
  int16_t adc0, adc1;

  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();
  
  Serial.println("ch1 :" + adc0);
  display.println("T1 :" + String(calcTemperture(adc0),1));

  Serial.println("ch2 :" + adc1);
  display.println("T2 :" + String(calcTemperture(adc1),1));

  display.display();
  delay(500);
}

float calcTemperture(int16_t adc) {
  float lanVal = (((float)26150 / (float)adc) - 1) / 10 ;
  Serial.println("lanVal : " + String(lanVal) );
  float lanItem = 0.000253 * log(lanVal) ;
  Serial.println("lanItem : " + String(lanItem) );
  float temp = 1 / (0.00335 + lanItem) - 273 ;
  return temp;
}

