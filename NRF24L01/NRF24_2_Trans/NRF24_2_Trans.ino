#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int text1 = 1000;

void setup(){
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  
  radio.stopListening();
  Serial.println("Starting");
}

void loop(){
  radio.write(&text1, sizeof(text1));
  delay(1000);
  Serial.println("transmitted");
}
