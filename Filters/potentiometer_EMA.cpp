// https://www.norwegiancreations.com/2015/10/tutorial-potentiometers-with-arduino-and-filtering/

int sensorPin = 0;
int sensorValue = 0;    //initialization of sensor variable, equivalent to EMA Y
float EMA_a = 0.6;      //initialization of EMA alpha
int EMA_S = 0;          //initialization of EMA S
 
void setup(){
    Serial.begin(115200);           //setup of Serial module, 115200 bits/second
    EMA_S = analogRead(sensorPin);  //set EMA S for t=1
}
 
void loop(){
    sensorValue = analogRead(sensorPin);
    EMA_S = (EMA_a*sensorValue) + ((1-EMA_a)*EMA_S);
    Serial.println(EMA_S);
    delay(50);
}