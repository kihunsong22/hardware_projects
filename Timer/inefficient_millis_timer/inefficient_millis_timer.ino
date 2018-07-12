int led = 13;
int prevState = 1;
unsigned int t = 0, t1 = 0;
void setup() {
  pinMode(led, OUTPUT);
  t = millis();
  digitalWrite(led, HIGH);
}

void loop() {
  while((millis()-t)<=250){
    continue;
  };
  t = millis();
  if(prevState){
    digitalWrite(led, LOW);
    prevState = 0;
  }else{
    digitalWrite(led, HIGH);
    prevState = 1;
  }
}
