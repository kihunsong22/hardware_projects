#include <SPI.h>
// #include <LoRa.h>
// #include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

SSD1306 display(0x3c, 21, 22);

const uint8_t GP2Y10 = 8;

void setup() {
  Serial.begin(115200);
  pinMode(GP2Y10, INPUT);
  pinMode(16, OUTPUT);  // OLED reset pin
	pinMode(2, OUTPUT);  //LED
	digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
	delay(50);
	digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high


	display.init();
	display.flipScreenVertically();
  delay(50);
}

void loop() {
  uint16_t pulse = pulseIn(GP2Y10,LOW,20000);
  float ugm3 = pulse2ugm3(pulse);
  Serial.print(ugm3,4);
  Serial.println(" ug/m3");

  display.clear();
  display.setFont(Open_Sans_Hebrew_Condensed_18);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 0, "PM2.5: " + String(ugm3));

  delay(100);
}

float pulse2ugm3(uint16_t pulse){
  float value = (pulse-1400)/14.0;
  if(value > 300){
    value = 0;
  }
  return value;
}
