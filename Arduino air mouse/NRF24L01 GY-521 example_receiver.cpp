#include <SPI.h>
#include <Mouse.h>
#include "RF24.h"

int msg[2];
int msg1, msg2;

byte address[6] = "1Node";
RF24 radio(7,8);  // CE, CSN

void setup(void) {
  Serial.begin(9600);
 
  radio.begin();
  Mouse.begin();

  radio.openReadingPipe(1, address);
  radio.startListening();
}

void loop(void) {
  if(radio.available()) {
    radio.read(&msg, sizeof(msg)); 
   
    Serial.print("Meassage (RX) = ");
 
    Serial.print(msg[0]);
    Serial.println(msg[1]);
 
    Mouse.move(msg[1], -msg[0]);
  }
}