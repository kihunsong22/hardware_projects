// 수정해주세요
#define DEVNUM 1  // 디바이스 번호
const uint8_t relayPin = LED_BUILTIN;  // 릴레이 핀번호
const char ssid[] = "SSID";  // 와이파이 이름
const char pass[] = "PASS";  // 와이파이 비밀번호
// 수정해주세요

#include <SPI.h>
#include <WiFiNINA.h>
WiFiClient client;
int status = WL_IDLE_STATUS;

const String webLink = "GET /upload.php?pass=pass1406002454";

uint16_t delayTime = 10000;
uint8_t pinStatus = 0;
uint32_t lastConnectionTime = 0;
String payload = "";

void setup(){
  Serial.begin(115200);
  Serial.println();

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(relayPin, OUTPUT);
	
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }
	
  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

	while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    delay(10000);
  }
  printWifiStatus();

	Serial.println();
}



void loop(){
  if (client.available()) {
		payload.concat(client.read());
		if(payload.indexOf("@1#") > 0){
			pinStatus = 1;
			// digitalWrite(relayPin, HIGH);
			digitalWrite(LED_BUILTIN, HIGH);
			payload = "";
		}else if(payload.indexOf("@0#") > 0){
			pinStatus = 0;
			// digitalWrite(relayPin, LOW);
			digitalWrite(LED_BUILTIN, LOW);
			payload = "";
		}
  }else if (millis() - lastConnectionTime >= delayTime) {
		Serial.println(payload);
		Serial.println("\n========================================");
		Serial.print("Status: ");  Serial.println(pinStatus);
		Serial.println("========================================\n");

    httpRequest();
  }
}


void httpRequest() {
  client.stop();

	String link1 = webLink + "&devnum=" + (String)DEVNUM + "&status=" + (String)status + " HTTP/1.1";
	
	if (client.connect("iotsv.cafe24.com", 80) == 1) {
    Serial.print("\nconnected to ");
		Serial.println(link1);
		Serial.flush();

		client.println(link1);
		client.println("Host: iotsv.cafe24.com");
    client.println("User-Agent: ArduinoWiFi/1.1");
		client.println("Connection: close");
		client.println();
	} else {
		Serial.println("Ethernet HTTP connection failed");
	}
	lastConnectionTime = millis();
}


void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
