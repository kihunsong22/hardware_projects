#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
SoftwareSerial ble(3, 2); // 아두이노의 RX, TX핀을 설정
RF24 radio(7, 8);

//  필요에따라 수정하세요!
const String tarMAC = "D43639C46D6F";  //검색할 비콘의 MAC주소
//const int limRSSI = 80;  //최소 거리
//

const byte rxAddr[6] = "00001";
String inString;
int intRssi = 0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);
  radio.stopListening();
  ble.begin(9600);
  
//  ble.print("AT+MODE1");
//  Serial.println(ble.readString());
//  ble.print("AT+ROLE1");
//  Serial.println(ble.readString());
//  ble.print("AT+IMME1");
//  Serial.println(ble.readString());
//  delay(100);
  Serial.println("Now Scanning");
}

void loop() {
  intRssi = 0;
  delay(1500);
//  ble.print("AT+DISI?");
//  inString = ble.readString();
//  if (inString.startsWith("OK+DISIS")) {
//    inString.replace("OK+DISIS", "");
//  }
//  if(!(inString.indexOf(tarMAC)>0)){
//    Serial.println("NOT FOUND");
//    return;
//  }
//
//  inString = inString.substring(inString.indexOf(tarMAC)+15, inString.indexOf(tarMAC)+17);
//  inString.remove(4);
  inString = "76";
  intRssi = inString.toInt();
  Serial.println(intRssi);
  radio.write(&intRssi, sizeof(intRssi));
}
