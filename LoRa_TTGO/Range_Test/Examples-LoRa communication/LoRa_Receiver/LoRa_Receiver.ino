#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306.h"
#include "fonts.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define BAND  868E6  // 433E6, 868E6, 915E6
#define TXPOW 10 // 2~20 -> PA_OUTPUT_RF0_PIN, 0~14-> PA_OUTPUT_BOOST_PIN
#define SF 7 // Spreading Factor: 6~12, default 7
#define SBW 125E3 // Signal Bandwidth: 7.83E, 10.4E3, 15.6E3, 20.8E3, 41.7E3, 62.5E3, 125.E3, 250E3, default 125E3
#define CR 5 // Coding Rate: 5~8, default 5

SSD1306 display(0x3c, 21, 22);

String packet, prevpacket;
int packetSize = 0;

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
	LoRa.setSignalBandwidth(SBW);
	LoRa.setCodingRate4(CR);
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
	if (packetSize){  // if(non-zero) -> true
		packet ="";
		for (int i = 0; i < packetSize; i++){
			packet += (char) LoRa.read();
		}
		packet = packet.substring(7);

		display.clear();
		display.setFont(Open_Sans_Hebrew_Condensed_18);
		display.setTextAlignment(TEXT_ALIGN_RIGHT);
		display.drawString(128, 0, "RSSI:" + String(LoRa.packetRssi(), DEC));
		display.setTextAlignment(TEXT_ALIGN_LEFT);
		display.drawStringMaxWidth(0, 0, 64, packet);
		// display.drawString(64, 19, String(packet.toInt()));
		// display.drawString(0, 38, String(packet.toInt()));

		if( (packet.toInt()-prevpacket.toInt()) != 1 ){
			display.setFont(Open_Sans_Hebrew_Condensed_18);
			delay(5);
			display.drawString(0, 19, " Lost: " + (packet.toInt()-prevpacket.toInt()));
		}
		printInfo();
		Serial.println(packet + "   RSSI:" + String(LoRa.packetRssi(), DEC));

		prevpacket = packet;
	}
	
	delay(50);
}

void printInfo(){
	display.setFont(Open_Sans_Hebrew_Condensed_14);
	display.setTextAlignment(TEXT_ALIGN_RIGHT);

	switch( String(BAND).substring(0, 3).toInt() ) {
		case 433:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 433Mhz, SF " + String(SF));
			break;

		case 868:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 868Mhz, SF " + String(SF));
			break;

		case 915:
			display.drawString(128, 48, "TXPOW: " + String(TXPOW) + ", 915Mhz, SF " + String(SF));
			break;
		
		default:
			break;
	}

	// display.drawString(128, 33, "Battery: " + String(VBAT));
	display.display();
}