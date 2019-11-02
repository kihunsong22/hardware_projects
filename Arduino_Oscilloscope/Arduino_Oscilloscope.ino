void setup(){
  Serial.begin(115200);
  Serial.println("Device: 3: Analyzer");

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
}

void loop(){
  Serial.print("0 1023 ");  // fix min/max value
  Serial.print(analogRead(A0));
  Serial.print(" "); Serial.print(analogRead(A1));
  Serial.print(" "); Serial.print(analogRead(A2));
  Serial.print(" "); Serial.print(analogRead(A3));
  Serial.print(" "); Serial.print(analogRead(A4));
  Serial.print(" "); Serial.print(analogRead(A5));
  Serial.print(" "); Serial.print(analogRead(A6));
  
  delay(1);

  Serial.println();
  Serial.flush();
}