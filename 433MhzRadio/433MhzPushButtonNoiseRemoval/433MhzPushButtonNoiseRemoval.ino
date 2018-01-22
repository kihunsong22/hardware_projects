int count = 0;
float voltage = 0;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("Connection Established");
}

void loop() {
    voltage = analogRead(A6) * (5.0 / 1023.0);
    if(voltage>2.5){
       count++;
      }
    else{
      count = 0;
      Serial.println("No Signal");
      }
      
    if(count>=7){
      Serial.println("Receiving Signal");
      }
    delay(15);
}
