//http://server_ip/gpio/0 will set LED OFF
//http://server_ip/gpio/1 will set LED ON

#include <ESP8266WiFi.h>

const char* ssid = "Network10";
const char* password = "12345678";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  server.begin();
  Serial.print("Server started: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println();
  Serial.println("New Client");
  while(!client.available()){
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.print("CLIENT REQUEST: ");
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  if (req.indexOf("/LED_0") != -1){ //LED_0
    val = 0;
  }
  else if (req.indexOf("/LED_1") != -1){ //LED_1
    val = 1;
  }
  else {
    Serial.println("INVALID REQUEST");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  digitalWrite(2, !val);
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\LED is now ";
  s += (val) ? 1 : 0;
  s += "</html>\n";
  s += "<br/><br/><button class='button  LED_ON'><a href='./LED_1'>LED ON</a></button><br/><br/>\r\n<button class='button  LED_OFF'><a href='./LED_0'>LED OFF</a></button>";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}