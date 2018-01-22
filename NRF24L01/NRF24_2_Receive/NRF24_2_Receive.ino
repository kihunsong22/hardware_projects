#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int text = 0;

void setup(){
  Serial.begin(9600);
  
  radio.begin();
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();
}

void loop(){
  if (radio.available()){
    text = 0;
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
  delay(100);
}
