// Transmitter
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int text = 1000;

void setup(){
  Serial.begin(115200);

  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  // radio.openReadingPipe(rxAddr);
  
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(108);
  radio.stopListening();

  Serial.println();
  Serial.println("Transmitting");
}

void loop(){
  radio.write(&text, sizeof(text));
  delay(1000);
  Serial.print("Sent: ");
  Serial.println(text);
  text++;
}
