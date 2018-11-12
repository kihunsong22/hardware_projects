#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define MAX_TX_SIZE 57
#define BC_ADDH 0xFF
#define BC_ADDL 0xFF
#define BC_CHAN 0x0E

// Wemos D1 Mini
// const uint8_t M0_PIN = D0;
// const uint8_t M1_PIN = D1;
// const uint8_t AUX_PIN = D2;
// const uint8_t SOFT_RX = D6;
// const uint8_t SOFT_TX = D7;

// Wemos D1 Mini Pro
const uint8_t M0_PIN = 16;
const uint8_t M1_PIN = 14;
const uint8_t AUX_PIN = 4;
const uint8_t SOFT_RX = 12;
const uint8_t SOFT_TX = 13;

// ATMega328p
// const uint8_t M0_PIN = 7;
// const uint8_t M1_PIN = 8;
// const uint8_t AUX_PIN = A0;
// const uint8_t SOFT_RX = 10;
// const uint8_t SOFT_TX = 11;

const uint8_t GPS_RX = 5;
const uint8_t GPS_TX = 0;  // useless

struct CFGstruct {  // settings parameter -> E32 pdf p.28
  // uint8_t HEAD = 0xC0;  // do not save parameters when power-down
  uint8_t HEAD = 0xC2;  // save parameters when power-down
  uint8_t ADDH = 0x05;
  uint8_t ADDL = 0x01;
  // uint8_t SPED = 0x18;  // 8N1, 9600bps, 0.3k air rate
  // uint8_t SPED = 0x19;  // 8N1, 9600bps, 1.2k air rate 
  uint8_t SPED = 0x1A;  // 8N1, 9600bps, 2.4k air rate
  // uint8_t CHAN = 0x10;  // 424Mhz
  uint8_t CHAN = 0x18;  // 434Mhz
  uint8_t OPTION_bits = 0xC4;  // 1, 1, 000, 1, 00
};
struct CFGstruct CFG;

SoftwareSerial E32(SOFT_RX, SOFT_TX);
SoftwareSerial gps_s(GPS_RX, GPS_TX);
TinyGPSPlus gps;

int8_t WaitAUX_H();  // wait for AUX clear
void SwitchMode(uint8_t mode);  // change mode to mode
void blinkLED();
void triple_cmd(uint8_t Tcmd);  // send 3x Tcmd
void ReceiveMsg();
int8_t SendMsg(String msg);

void setup(){
  Serial.begin(115200);
  E32.begin(9600);
  gps_s.begin(9600);

  Serial.println("\nInitializing...");
  Serial.println("Device: 1: Node/TX");

  pinMode(M0_PIN, OUTPUT);
  pinMode(M1_PIN, OUTPUT);
  pinMode(AUX_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  triple_cmd(0xC4);  // 0xC4: reset
  delay(1000);

  SwitchMode(3);  // sleep mode/parameter setting
  E32.write((const uint8_t *)&CFG, 6);  // 6 for 6 variables in CFG
  delay(1200);

  SwitchMode(2);  // power-saving mode

  Serial.println("Init complete");
}

// Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum
//            (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail
// ----------------------------------------------------------------------------------------------------------------------------------------
// **** ***** ********** *********** **** ********** ******** **** ****** ****** ***** ***   ******** ****** ***   0     0         0        
// 8    1.9   37.393063  126.954201  211  11/03/2018 14:02:04 211  72.30  0.00   0.00  N     8875     329.70 NNW   648   2         0        
// 8    1.9   37.393070  126.954208  207  11/03/2018 14:02:05 208  72.80  0.00   0.33  N     8875     329.70 NNW   1296  4         0        
// 8    1.9   37.393070  126.954208  224  11/03/2018 14:02:06 224  72.70  0.00   0.00  N     8875     329.70 NNW   1944  6         0        

String dataStr = "", data1="";
uint8_t gps_sat=0;
double gps_lati=0.0, gps_long=0.0;

void loop(){
  while (gps_s.available() > 0){
    if (gps.encode(gps_s.read())){
      gps_lati = gps.location.lat()*10000;
      gps_long = gps.location.lng()*10000;
      gps_sat = gps.satellites.value();

      dataStr = "";
      dataStr.concat("$");
      dataStr.concat(String(gps.location.lat(), 6));
      // dataStr.concat(gps_lati/10000);
      dataStr.concat("#");
      dataStr.concat(String(gps.location.lng(), 6));
      // dataStr.concat(gps_long/10000);
      dataStr.concat("$");
      Serial.print("DataSTR: "); Serial.println(dataStr);
      Serial.print("Satellites: "); Serial.println(gps_sat);

      if(gps_sat>4){
        if(SendMsg(dataStr) == 0){
          blinkLED();
          delay(2500);
        }
      }
      SwitchMode(2); // Power-Saving mode
    }
  }
}



void blinkLED(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(75);
  digitalWrite(LED_BUILTIN, LOW);
  delay(75);
}

int8_t WaitAUX_H(){
  uint8_t cnt = 0;

  while((digitalRead(AUX_PIN)==LOW) && (cnt++<15)){
    Serial.print(".");
    delay(100);
  }

  if(cnt>=100){
    Serial.println("  AUX-TimeOut");
    return -1;
  }
  return 0;
}

void SwitchMode(uint8_t mode){
  WaitAUX_H();

  switch (mode){
    case 0:
      // Mode 0 | normal operation
      digitalWrite(M0_PIN, LOW);
      digitalWrite(M1_PIN, LOW);
      break;
    case 1:
      // Mode 1 | wake-up
      digitalWrite(M0_PIN, HIGH);
      digitalWrite(M1_PIN, LOW);
      break;
    case 2:
      // Mode 2 | power save
      digitalWrite(M0_PIN, LOW);
      digitalWrite(M1_PIN, HIGH);
      break;
    case 3:
      // Mode 3 | sleep mode/parammeter setting
      digitalWrite(M0_PIN, HIGH);
      digitalWrite(M1_PIN, HIGH);
      break;
    default:
      return;
  }

  WaitAUX_H();
  delay(10);
}


void triple_cmd(uint8_t Tcmd){
  WaitAUX_H();
  uint8_t CMD[3] = {Tcmd, Tcmd, Tcmd};
  E32.write(CMD, 3);
  Serial.print("Command: ");
  Serial.print(Tcmd, HEX);
  Serial.print(Tcmd, HEX);
  Serial.print(Tcmd, HEX);
  Serial.println();
  delay(15);
}

void ReceiveMsg(){
  if(E32.available()==0){
    return;
  }
  uint8_t data_len = E32.available();
  uint8_t idx;
  blinkLED();

  Serial.print("LoRa Received: [");
  Serial.print(String(data_len));
  Serial.println("] bytes.");

  char RX_buf[data_len+1];
  for(idx=0;idx<data_len;idx++){
    RX_buf[idx] = E32.read();
  }
  // RX_buf[data_len] = 0;  // NULL terminate array

  Serial.print("data: [");
  Serial.print(RX_buf);
  Serial.println("]");
  Serial.println();
  Serial.flush();

  return;
}

int8_t SendMsg(String msg){
  msg.concat("  ");
  Serial.print("LoRa transmitting [");
  Serial.print(String(msg.length()));
  Serial.println("] bytes");
  // Serial.print("data: ["); Serial.print(msg); Serial.println("]");

  char text[MAX_TX_SIZE+3];
  msg = msg.substring(0, MAX_TX_SIZE-1);
  if(msg.length() > MAX_TX_SIZE){
    msg.toCharArray(text, MAX_TX_SIZE);
  }else{
    msg.toCharArray(text, msg.length());
  }

  if(CFG.CHAN != 0x0E){  // fixed transmission mode: set ADDH, ADDL, CHAN for first 3 bits
    for(uint8_t q=60; q>=3; q--){
      text[q] = text[q-3];
    }
    text[0] = CFG.ADDH;
    text[1] = CFG.ADDL;
    text[2] = CFG.CHAN;
  }

  SwitchMode(0);  // Normal mode

  E32.write(text, msg.length()+3);
  WaitAUX_H();
  delay(10);

  return 0;
}
