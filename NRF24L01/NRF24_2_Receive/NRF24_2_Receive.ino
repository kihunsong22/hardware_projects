// Receiver
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
String text = "";

void setup(){
  Serial.begin(115200);
  
  radio.begin();
  radio.openReadingPipe(0, rxAddr);
  
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  
  radio.setChannel(108);
  radio.startListening();

  Serial.println();
  Serial.println("Receiving");
}

void loop(){
  if (radio.available()){
    text = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
}
