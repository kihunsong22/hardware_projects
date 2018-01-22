#include <SoftwareSerial.h>
SoftwareSerial HM10(3, 2); // 아두이노의 RX, TX핀을 설정

String beaconMAC = "";  //비콘의 맥주소
String tempString = "";
String returnString = "";  //AT+DISI? 명령의 반환값을 저장

int rssi = 0;  //전달받은 RSSI값
int EOS = 0;
int i = 0, count = 0, count1 = 0;
char returnArray[160];

void setup() {
  Serial.begin(9600);
//  Serial.println("Serial connected");
  HM10.begin(9600);
}

void loop() {
  if(HM10.available()){
    returnArray[i] = HM10.read();
    Serial.print(returnArray[i++]);
    count = 0;
    EOS = 0;
  }
  else
    count++;

  if(count>10000&&EOS == 0){
      if(count1>4000){
        Serial.println();
        EOS = 1;
        count1 = 0;
      }
      count1++;
  }

//  if(String(HM10.read()).length()==2){
//    Serial.print(returnString);
//    tempString = returnString;
//    returnString = "";
//    EOS = 1;
//    Serial.println();
//  }

  if (Serial.available()) {
    byte data = Serial.read();
    HM10.write(data);
  }
}
