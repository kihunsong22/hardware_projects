int value, ti;

void setup(){
    pinMode(A0, INPUT);

    Serial.begin(115200);
    // Serial.println("Serial connected");

    ti = millis();
}

void loop(){
    if( (millis()-ti)>10 ){
        value = analogRead(A0);
        Serial.print("Value: ");
        Serial.println(value);
        ti = millis();
    }
}