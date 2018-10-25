int GPdataPin = A2; //Connect dust sensor to Arduino A0 pin
int GPLEDPIN = 4; //GP2Y10 LED pin (pin 3 on GP2Y10)
float GPval = 0, dustDensity = 0;

void setup(){
  Serial.begin(115200);
  pinMode(GPLEDPIN, OUTPUT);
}

void loop(){
  digitalWrite(GPLEDPIN,LOW); //LED ON
  delayMicroseconds(280);
  GPval = analogRead(GPdataPin);
  delayMicroseconds(40);
  digitalWrite(GPLEDPIN,HIGH); //LED OFF
  delayMicroseconds(9680);

  GPval = GPval * (5.0 / 1024.0);
  dustDensity = 0.17 * GPval - 0.1;
//  if(dustDensity<0){
//    dustDensity=0;
//  }

  Serial.print("Voltage: ");
  Serial.print(GPval);
  Serial.print("   // Dust Density: ");
  Serial.print(dustDensity);
  Serial.println("mg/m^3");
  Serial.println();

  delay(1000);
}
