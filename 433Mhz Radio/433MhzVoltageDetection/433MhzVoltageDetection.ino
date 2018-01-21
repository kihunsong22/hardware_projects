//#include<time.h>
int mil = 0, sec=0, nsec=0, temp;
float voltage = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
    int sensorValue = analogRead(A6);
    voltage = sensorValue * (5.0 / 1023.0);
    Serial.print("Voltage: ");
    if(voltage<=1)
      Serial.println("=");
    else if(1<voltage<=1.5)
      Serial.println("=====");
    else if(1.5<voltage<=2)
      Serial.println("=========");
    else if(2<voltage<=2.5)
      Serial.println("=============");
    else if(2.5<voltage<3)
      Serial.println("=================");
    else
      Serial.println("======================");
    delay(5);
    
//  mil = millis();
//  nsec = mil/500;
//  if(sec!=nsec){
//    int sensorValue = analogRead(A6);
//    voltage = sensorValue;
//    voltage = sensorValue * (5.0 / 1023.0);
//    Serial.print("433 Receiver Voltage: ");
//    Serial.println(voltage);
//    }
//    sec = nsec;

}
