// 수정해주세요
#define DEVNUM 3  // 디바이스 번호
const uint8_t relayPin = 12;  // 릴레이 핀번호
const uint8_t DHTpin  = 2;  // DHT11 데이터 핀번호
const char ssid[] = "mickey-office";  // 와이파이 이름
const char pass[] = "01052913901~~";  // 와이파이 비밀번호
unsigned long myChannelNumber = 803609;  // 채널 넘버
const char * myWriteAPIKey = "AI5CGWHS71R74C7K";  // API 키
// 수정해주세요

#include <SPI.h>
#include <WiFiNINA.h>
#include <pm2008_i2c.h>
#include "DHT.h"
#include "ThingSpeak.h"

int status = WL_IDLE_STATUS;

const String webLink = "GET /upload.php?pass=pass1406002454";
const uint16_t delayTime = 10000;
uint8_t pinStatus = 0;
uint32_t lastConnectionTime = 0;
String payload = "";

int pm10 = 0;
int pm25 = 0;
int pm1 = 0;
int dht_temp = 0;
int dht_humi = 0;

PM2008_I2C pm2008_i2c;
WiFiClient client;
DHT dht(DHTpin, DHT11);

void setup(){
  Serial.begin(115200);
  Serial.println();

	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(relayPin, OUTPUT);
  
  pm2008_i2c.begin();
  pm2008_i2c.command();
  dht.begin();
	
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

  ThingSpeak.begin(client);

	Serial.println();
}


void loop(){
  if (client.available() > 0) {
    char t = client.read();
		payload.concat(t);
		if(payload.indexOf("@1#") >= 0){
      Serial.println("========================================");
      Serial.println(payload);
      Serial.println("========================================");
			pinStatus = 1;
			digitalWrite(relayPin, HIGH);
			digitalWrite(LED_BUILTIN, HIGH);
			payload = "";
		}else if(payload.indexOf("@0#") >= 0){
      Serial.println("========================================");
      Serial.println(payload);
      Serial.println("========================================");
			pinStatus = 0;
			digitalWrite(relayPin, LOW);
			digitalWrite(LED_BUILTIN, LOW);
			payload = "";
		}
  }else if (millis() - lastConnectionTime >= delayTime) {
		Serial.println("\n========================================");
		Serial.print("Status: ");  Serial.println(pinStatus);
		Serial.println("========================================\n");
    
    uint8_t ret = pm2008_i2c.read();
    if(ret == 0){
      Serial.println("PM2008 readings okay");
      pm1 = pm2008_i2c.pm1p0_grimm;
      pm25 = pm2008_i2c.pm2p5_grimm;
      pm10 = pm2008_i2c.pm10_grimm;
    }else{
      Serial.println("PM2008 error");
    }

    dht_temp = dht.readTemperature();
    dht_humi = dht.readHumidity();

    if (isnan(dht_humi) || isnan(dht_temp)) {
      Serial.println("DHT11 error");
    }

    ThingSpeak.setField(1, pm10);
    ThingSpeak.setField(2, pm25);
    ThingSpeak.setField(3, pm1);
    ThingSpeak.setField(4, dht_temp);
    ThingSpeak.setField(5, dht_humi);

    ThingSpeak.setStatus("Good");
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x==200){
      Serial.println("ThingSpeak upload success");
    }else{
      Serial.println("ThingSpeak error");
    }

    httpRequest();
  }

  if(millis() > 7200000‬){  // 2시간마다 리셋
    resetFunc();
  }
}


void httpRequest() {
  client.stop();

	String link1 = webLink + "&devnum=" + (String)DEVNUM + "&status=" + (String)pinStatus + " HTTP/1.1";
	
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

void(* resetFunc) (void) = 0;
