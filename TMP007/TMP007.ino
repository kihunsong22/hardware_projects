#include <Wire.h>
#include "Adafruit_TMP007.h"

// 3.3V 사용이 더 안정적일수도 있다고 하는데 모르겠다
// SCL(A5), SDA(A4)

Adafruit_TMP007 tmp007;
//Adafruit_TMP007 tmp007(0x41);  // start with a diferent i2c address!

void setup() { 
  Serial.begin(115200);
  Serial.println("Adafruit TMP007");

  // 1 sample takes 250milsec
  // tmp007.begin(TMP007_CFG_1SAMPLE) /2/4/8 - 16 by default
  if (! tmp007.begin(TMP007_CFG_2SAMPLE)) {
    Serial.println("No sensor found");
    continue;
  }
}

void loop() {
   float objt = tmp007.readObjTempC();
   Serial.print("TARGET temp: "); Serial.print(objt); Serial.println("*C");
   float diet = tmp007.readDieTempC();
   Serial.print("   DIE temp: "); Serial.print(diet); Serial.println("*C");
   Serial.println();
   
   delay(500); // 250mil for every sampling rate
}
