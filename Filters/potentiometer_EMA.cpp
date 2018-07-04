// https://www.norwegiancreations.com/2015/10/tutorial-potentiometers-with-arduino-and-filtering/

    int sensorPin = 0;

    int sensorValue = 0; //EMA current data
    float EMA_a = 0.6; //EMA alpha, can be set between 0<a<1
    int EMA = 0; //EMA output
    // as EMA_a is set closer to 1, the more quicker the EMA output will change
 
void setup(){
    Serial.begin(115200);
    EMA = analogRead(sensorPin); //EMA -< t=1
}
 
void loop(){
    sensorValue = analogRead(sensorPin);
    EMA = (EMA_a*sensorValue) + ((1-EMA_a)*EMA); //EMA formula
    Serial.println(EMA);
    delay(50);
}