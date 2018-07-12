const int ledPin =  13;
int ledState = LOW;
unsigned long previousMillis = 0, currentMillis = 0;
int interval = 250;

void setup() {
  pinMode(ledPin, OUTPUT);      
}
 
void loop(){
  currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    
    digitalWrite(ledPin, ledState);
  }
}
