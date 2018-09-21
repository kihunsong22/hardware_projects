#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306.h"
#include "images.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define BAND  868E6  // 433E6, 868E6, 915E6
#define TXPOW 20 // 2~20 -> PA_OUTPUT_RF0_PIN, 0~14-> PA_OUTPUT_BOOST_PIN
#define SF 7 // Spreading Factor: 6~12, default 7
#define SBW 125E3 // Signal Bandwidth: 7.83E, 10.4E3, 15.6E3, 20.8E3, 41.7E3, 62.5E3, 125.E3, 250E3, default 125E3
#define CR 5 // Coding Rate: 5~8, default 5

String packet;
int packetSize = 0;

SSD1306 display(0x3c, 21, 22);


void setup() {
	Serial.begin(115200);
	Serial.println();
	Serial.println("LoRa Receiver");

	pinMode(16, OUTPUT);  // OLED reset pin
	pinMode(2, OUTPUT);  //LED
	digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
	delay(50);
	digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、

	SPI.begin(SCK,MISO,MOSI,SS);
	LoRa.setPins(SS,RST,DI0);
	LoRa.setTxPower(TXPOW);
	LoRa.setSpreadingFactor(SF);
	LoRa.setSignalBandWidth(SBW);
	LoRa.setCodingRate(CR);
	if (!LoRa.begin(BAND)) {
		Serial.println("LoRa failed to start");
		while (1);
	}
	LoRa.receive();

	display.init();
	display.flipScreenVertically();  

	Serial.println("Init OK");
	delay(50);
}

void loop() {
	packetSize = LoRa.parsePacket();
	if (packetSize){  // non-zero -> true
		packet ="";
		for (int i = 0; i < packetSize; i++){
			packet += (char) LoRa.read();
		}

		display.clear();
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.setFont(Open_Sans_Hebrew_Condensed_14);
		display.drawString(0 , 15 , String(packetSize,DEC) + " bytes");
		display.drawStringMaxWidth(0 , 26 , 128, packet);
		display.drawString(0, 0, "RSSI: " + String(LoRa.packetRssi(), DEC)); 
		display.display();
		Serial.println("RSSI: " + String(LoRa.packetRssi(), DEC));
	}
	
	delay(5);
}
