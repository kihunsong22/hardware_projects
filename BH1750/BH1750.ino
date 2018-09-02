// 0x23(default), 0x5C(ADDR HIGH)
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

int vpin = 13;

void setup(){
  Serial.begin(115200);
  pinMode(vpin, OUTPUT);
  Wire.begin();
}


void loop() {
    digitalWrite(vpin, HIGH);
    delay(50);
    lightMeter.begin();
    uint16_t lux = lightMeter.readLightLevel();
    Serial.println(lux);
    delay(500);
    digitalWrite(vpin, LOW);

}