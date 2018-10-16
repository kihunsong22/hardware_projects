#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// WiFi
#define SSID1 "DimiFi 2G1"
#define PASS1 "newdimigo"
#define SSID2 "DimiFi 2G2"
#define PASS2 "newdimigo"

// http://tracker.iwinv.net/upload?gps_long=-1&gps_lati=-1&rssi=404&passcode=4660
const String link = "http://tracker.iwinv.net/upload?passcode=4660";
String link1 = "";

uint16_t gps_lati=0, gps_long=0, rssi=0;

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

void setup(){
	Serial.begin(115200);
	Serial.println("\r\n\r\n\r\n");
	Serial.println("Initializing");

	WiFi.mode(WIFI_STA);
	WiFiMulti.addAP(SSID1, PASS1);
	WiFiMulti.addAP(SSID2, PASS2);
	while(WiFiMulti.run() != WL_CONNECTED){
		Serial.print(".");
		delay(250);
	}
	Serial.println("\r\n");

	Serial.print("Connected: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

void loop(){
	gps_lati=10;
	gps_long=10;
	rssi=404;

	link1 = link+"&gps_lati="+String(gps_lati)+"&gps_long="+String(gps_long)+"&rssi="+String(rssi);
	Serial.print("LINK: ");
	Serial.println(link1);

	http.begin(link1);
	if (http.GET() == HTTP_CODE_OK) {
		String payload = http.getString();
		Serial.print("Response: ");
		Serial.println(payload);
		Serial.println("HTTP upload success");
	}else{
		Serial.println("HTTP Connection Error");
		Serial.println();
	}
	http.end();

	while(1){
		ESP.wdtFeed();
	}
}