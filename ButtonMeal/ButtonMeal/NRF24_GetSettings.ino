#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//#include <LowPower.h>  //AVR Low Power library that will basically turn this arduino off
RF24 radio(7, 8);

int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}
FILE serial_stdout;

void setup(){
  Serial.begin(115200);
  radio.begin();
  fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &serial_stdout;

  Serial.print("Payload Size : ");
  Serial.println(radio.getPayloadSize());  //The number of bytes in the payload
  Serial.print("RF Channel : ");
  Serial.println(radio.getChannel());  //The currently configured RF Channel
  Serial.print("PA Level : ");
  Serial.println(radio.getPALevel());  //NRF24L01: -18dBm, -12dBm, -6dBm and 0dBm
  Serial.print("Datarate : ");
  Serial.println(radio.getDataRate());  //The value is one of 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS
  Serial.println();Serial.println();
  Serial.println("RF24 Detailed Info : ");
  radio.printDetails();

  delay(500);
}

void loop(){
//  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF,TIMER1_OFF, TIMER0_OFF,SPI_OFF, USART0_OFF,TWI_OFF );
}
