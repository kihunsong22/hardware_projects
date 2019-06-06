#include <AWS_IOT.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "aws_iot_config.h"
#include <SPI.h>
#include <PubSubClient.h>

// #define HOST_ADDRESS "arn:aws:iot:ap-northeast-2:252527415385:cert/7b40b6256cde143aa9746795f1dbf2c4b290386426c88a034f2fef37810e8a8e"
// #define CLIENT_ID "ESP32"
// #define TOPIC_NAME ""

WiFiMulti wifiMulti;
PubSubClient client(ethClient);

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress server(172, 16, 0, 2);

int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];


void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void mySubCallBackHandler(char *topicName, int payloadLen, char *payLoad);

void setup(){
  Serial.begin(115200);
  Serial.println("\n\n");

  wifiMulti.addAP("DimiFi 2G1", "newdimigo");
  wifiMulti.addAP("DimiFi 2G2", "newdimigo");
  wifiMulti.addAP("DimiFi_2G", "newdimigo");

  Serial.print("Attempting to connect to SSID: ");
  if (wifiMulti.run() == WL_CONNECTED){
    Serial.println("Connected to wifi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  
  client.setServer(server, 1883);
  client.setCallback(callback);

  delay(250);
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
