//Woon Jun Shen
//UM402 (433 MHz UART)
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); //TX, RX
// gnd SET_A and SET_B for Normal Mode (Send and Receive)

int i = 0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  i++;
  mySerial.print("TEST");
  mySerial.println(i);
  delay(1000);
  Serial.print("TEST");
  Serial.println(i);
  Serial.println();
}
