#include <Wire.h>
#include "Adafruit_TMP007.h"

// Connect VCC to +3V (its a quieter supply than the 5V supply on an Arduino
// Gnd -> Gnd
// SCL connects to the I2C clock pin. On newer boards this is labeled with SCL
// otherwise, on the Uno, this is A5 on the Mega it is 21 and on the Leonardo/Micro digital 3
// SDA connects to the I2C data pin. On newer boards this is labeled with SDA
// otherwise, on the Uno, this is A4 on the Mega it is 20 and on the Leonardo/Micro digital 2

Adafruit_TMP007 tmp007;
//Adafruit_TMP007 tmp007(0x41);  // start with a diferent i2c address!

void setup() { 
  Serial.begin(115200);
  Serial.println("Adafruit TMP007 example");

  // you can also use tmp007.begin(TMP007_CFG_1SAMPLE) or 2SAMPLE/4SAMPLE/8SAMPLE
  if (! tmp007.begin(TMP007_CFG_2SAMPLE)) {
    Serial.println("No sensor found");
    while (1);
  }
}

void loop() {
   float objt = tmp007.readObjTempC();
   Serial.print("TARGET temp: "); Serial.print(objt); Serial.println("*C");
   float diet = tmp007.readDieTempC();
   Serial.print("   DIE temp: "); Serial.print(diet); Serial.println("*C");
   Serial.println();
   
   delay(500); // 250mil for 1 sample
}
