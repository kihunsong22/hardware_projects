#include <SoftwareSerial.h>
#define MODE 1 //Operating Mode: 0~3
SoftwareSerial lora(2, 3); //TX, RX

int prevauxstate = 0, auxstate = 0, auxcount = 0;
int modeArr[4][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

int M0 = 5;
int M1 = 4;
int AUX = 7;

void setup() {
  Serial.begin(9600);
  lora.begin(9600);

  Serial.println("Power on");

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(AUX, INPUT);

  digitalWrite(M0, modeArr[MODE][0]); //M0, M1을 통한 전송 모드 설정
  digitalWrite(M1, modeArr[MODE][1]);
  Serial.print("M0: ");
  Serial.println(modeArr[MODE][0]);
  Serial.print("M1: ");
  Serial.println(modeArr[MODE][1]);
  Serial.println();
}

void loop() {
  prevauxstate = auxstate;
  auxstate = digitalRead(AUX);
  if(prevauxstate != auxstate){
    auxcount++;
  }
  else if(auxcount>0){
    Serial.print("auxSTATE : ");
    Serial.print(auxstate);
    Serial.print("        auxcount : ");
    Serial.println(auxcount);
    auxcount = 0;
  }

  if(Serial.available() > 0){//사용자 입력을 LoRa모듈에 전송
    String input = Serial.readString();
    lora.println(input);
  }
 
  if(lora.available() > 1){//LoRa모듈의 출력을 시리얼 모니터에 출력
    String input = lora.readString();
    Serial.println(input);
  }

  if (lora.available()) {
    Serial.write(lora.read());
  }
  if (Serial.available()) {
    lora.write(Serial.read());
  }
}