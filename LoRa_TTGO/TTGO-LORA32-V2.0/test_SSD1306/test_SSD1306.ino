#include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24


const uint8_t blue = 23;
const uint8_t vbatPin = 35;
float VBAT;  // battery voltage from ESP32 ADC read
char string[25];

SSD1306  display(0x3c, 21, 22);

/*
The ADC value is a 12-bit number, so the maximum value is 4095 (counting from 0)
To convert the ADC integer value to a real voltage you’ll need to divide it by the maximum value of 4095,
then double it (note above that Adafruit halves the voltage), then multiply that by the reference voltage of the ESP32 which 
is 3.3V and then vinally, multiply that again by the ADC Reference Voltage of 1100mV.
*/

void setup(){
	Serial.begin(115200);
	pinMode(blue, OUTPUT);
	pinMode(vbatPin, INPUT);
	display.init();

	display.flipScreenVertically();
}

void loop(){
	digitalWrite(blue, 1);
	delay(100);
	digitalWrite(blue, 0);
	delay(100);
	digitalWrite(blue, 1);
	delay(100);

	VBAT = (float)(analogRead(vbatPin)) / 4095*2*3.3*1.1;
	Serial.println("Vbat = "); Serial.print(VBAT); Serial.println(" Volts");

	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	display.drawString(0, 0, "Battery");
	display.setFont(ArialMT_Plain_16);
	display.drawString(0, 10, "Monitoring");
	display.setFont(ArialMT_Plain_24);
	itoa(Vbat,string,10);
	sprintf(string,"%7.5f",Vbat);
	display.drawString(0, 26, string);
	display.display();

	digitalWrite(blue, 0);
	delay(700);
}
