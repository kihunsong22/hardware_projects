// 수정해주세요
#define DEVNUM 1  // 디바이스 번호
// uint8_t relayPin = D4;  // 릴레이 핀번호
// 수정해주세요

#define ESP32 0

#if ESP32==1  //ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;
HTTPClient http;
const String webLink = "http://iotsv.cafe24.com/upload.php?pass=pass1406002454";
#endif

#if ESP32==0  //Ethernet
#include <SPI.h>
#include <Ethernet.h>
EthernetClient client;
IPAddress ip(192, 168, 0, 125);
IPAddress myDns(8, 8, 8, 8);
const String webLink = "GET /upload.php?pass=pass1406002454";
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
void httpRequest();
#endif

uint16_t delayTime = 10000;
uint8_t status = 0;
uint32_t lastConnectionTime = 0;
String payload = "";

void setup(){
  Serial.begin(115200);
  Serial.println();

	#if ESP32==1  //ESP32
	pinMode(LED_BUILTIN, OUTPUT);
	wifiMulti.addAP("DimiFi 2G1", "newdimigo");
	wifiMulti.addAP("DimiFi_2G", "newdimigo");
	wifiMulti.addAP("KT_GiGA_2G_AF85", "4zde4fb332");

	Serial.println("Connecting to WiFi");
	while (wifiMulti.run() != WL_CONNECTED){
		delay(250);
		Serial.print(".");
	}
	Serial.print("Connected to wifi: ");
	Serial.println(WiFi.SSID());
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	#endif

	#if ESP32==0  //Ethernet
	Serial.println("Initialize Ethernet with DHCP:");
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			Serial.println("Ethernet shield was not found");
			while (true)    {    delay(1000);    }
		}
		if (Ethernet.linkStatus() == LinkOFF) {
			Serial.println("Ethernet cable not connected.");
		}
		// try to congifure using IP address instead of DHCP:
		Ethernet.begin(mac, ip, myDns);
	} else {
		Serial.print("DHCP assigned IP: ");
		Serial.println(Ethernet.localIP());
	}
	#endif

	Serial.println();
}



void loop(){
	#if ESP32==1  //ESP32
	delay(delayTime);

	String link1 = webLink + "&devnum=" + (String)DEVNUM + "&status=" + (String)status;
	Serial.print("HTTP link: ");  Serial.println(link1);
	http.begin(link1);
	if (http.GET() == HTTP_CODE_OK) {
		payload = http.getString();
		Serial.print("Response: ");  Serial.println(payload);
		status = payload.toInt();
		if(status){
			digitalWrite(relayPin, HIGH);
			digitalWrite(LED_BUILTIN, LOW);
		}else{
			digitalWrite(relayPin, LOW);
			digitalWrite(LED_BUILTIN, HIGH);
		}
	}else{
		Serial.println("HTTP Connection Error");
	}
	http.end();
	#endif


	#if ESP32==0  //Ethernet
  if (client.available()) {
		char c = client.read();
		// Serial.print(c);
		payload.concat(c);
		// if(payload.indexOf("@1#") != -1){
		// 	status = 1;
		// 	// digitalWrite(relayPin, HIGH);
		// 	digitalWrite(LED_BUILTIN, HIGH);
		// 	payload = "";
		// }else if(payload.indexOf("@0#") != -1){
		// 	status = 0;
		// 	// digitalWrite(relayPin, LOW);
		// 	digitalWrite(LED_BUILTIN, LOW);
		// 	payload = "";
		// }
  }else if (millis() - lastConnectionTime >= delayTime) {
		Serial.println(payload);
		Serial.println("\n========================================\n");
		Serial.print("Status: ");  Serial.println(status);

    httpRequest();
  }
	#endif
}


void httpRequest() {
	// while(client.available()) {
	// 	client.read();
	// }
  client.stop();

	String link1 = webLink + "&devnum=" + (String)DEVNUM + "&status=" + (String)status + " HTTP/1.1";
	if (client.connect("iotsv.cafe24.com", 80) == 1) {
		Serial.println();
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
		Serial.println(link1);
		Serial.println();
		Serial.flush();

		client.println(link1);
		client.println("Host: iotsv.cafe24.com");
    client.println("User-Agent: arduino-ethernet");
		client.println("Connection: close");
		client.println();
	} else {
		Serial.println("Ethernet HTTP connection failed");
	}
	lastConnectionTime = millis();
	
	return;
}
