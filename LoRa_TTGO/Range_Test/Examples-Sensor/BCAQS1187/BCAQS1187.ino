#include "BCAQS1187.h"
#include <Wire.h>

#define BCAQS1187_ADDR 0x5A
#define ALERTPIN 4

void sensorInitialize(void);
void readSensorData(void);

BCAQS1187 bcaqs1187 (BCAQS1187_ADDR); //CSS811 Indoor Air Quality Sensor

void setup() {
  Serial.begin (115200);
  Wire.begin();
  sensorInitialize();
}

void loop() {
  readSensorData();
  Serial.println();
  delay(1000);
}


void sensorInitialize(){
  Serial.println("[BCAQS1187] Indoor Air Quarity Sensor Enabled");
  bcaqs1187.SWReset();
  delay(100);
  bcaqs1187.appStart();
  bcaqs1187.setThresholds((uint16_t) 1500, (uint16_t) 2500, (uint8_t) 50);
  bcaqs1187.setDriveMode(BCAQS1187_DRIVE_MODE_1SEC);

  Serial.println("Initialized BC Sensors Series\n\n");
}

void readSensorData(){  
  bcaqs1187.readResultData();
  Serial.print("[BCAQS1187] TVOC : ");
  Serial.print(bcaqs1187.getTVOC());
  Serial.print(" ppb   eCO2 : ");
  Serial.print(bcaqs1187.geteCO2());  
  Serial.println(" ppm");
}
