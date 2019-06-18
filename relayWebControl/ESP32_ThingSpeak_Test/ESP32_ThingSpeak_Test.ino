#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "ThingSpeak.h"
WiFiMulti wifiMulti;
HTTPClient http;
WiFiClient  client;

const char * myWriteAPIKey = "QH4U4JO6HEF7PEGH";
const uint32_t myChannelNumber = 803603;

void setup(){
  Serial.begin(115200);
  Serial.println();

	pinMode(LED_BUILTIN, OUTPUT);
	wifiMulti.addAP("DimiFi_2G", "newdimigo");

	Serial.println("Connecting to WiFi");
	while (wifiMulti.run() != WL_CONNECTED){
		delay(250);
		Serial.print(".");
	}
	Serial.print("Connected to wifi: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client);
  
	Serial.println();
}



void loop(){

	int number1 = random(20, 80);
	int number2 = random(0,100);
	int number3 = random(0,100);
	int number4 = random(0,100);
	int number5 = random(0,100);

	
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);
  ThingSpeak.setField(4, number4);
  ThingSpeak.setField(5, number5);
  ThingSpeak.setStatus("status message aaaaaargh");

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

	delay(15000);
}
