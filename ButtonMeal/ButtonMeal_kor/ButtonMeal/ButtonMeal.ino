#include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //kfont, Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

SSD1306 display(0x3c, 21, 22);

void setup() {
	Serial.begin(115200);

	pinMode(16, OUTPUT);  // OLED reset pin
	digitalWrite(16, LOW);  // set GPIO16 LOW to reset OLED
	digitalWrite(16, HIGH);  // while OLED is running, must set GPIO16 in HIGH

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
