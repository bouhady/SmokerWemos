/*********************************************************************
  This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

  This example is for a 64x48 size display using I2C to communicate
  3 pins are required to interface (2 I2C and one reset)

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2



void setup()   {
  Serial.begin(115200);
  Serial.println("Hello");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done

  display.display();
  delay(2000);
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
  display.print("ch1 :");
  display.println(adc0);
  display.print("t1 :");
  display.println(calcTemperture(adc0));

  display.print("ch2 :");
  display.println(adc1);
  display.print("t2 :");
  display.println(calcTemperture(adc1));

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

//void testdrawchar(void) {
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.setCursor(0, 0);
//
//  for (uint8_t i = 0; i < 168; i++) {
//    if (i == '\n') continue;
//    display.print(String(i) + " : "  + i + " ,");
//    if ((i > 0) && (i % 5 == 0))
//      display.println();
//  }
//  display.display();
//
//}

