#include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //kfont, Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24
int counter = 100;

SSD1306 display(0x3c, 21, 22);

void setup() {
	Serial.begin(115200);
	display.init();
	display.flipScreenVertically();
}

void loop() {
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(Open_Sans_Hebrew_Condensed_24);
	display.drawString(0, 0, "Data: " + (String(counter)));

	delay(1000);
}
