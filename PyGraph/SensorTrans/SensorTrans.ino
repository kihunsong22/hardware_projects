int value, ti;

void setup(){
    pinMode(A6, INPUT);

    Serial.begin(115200);
}

void loop(){
    value = analogRead(A0);
    ti = (millis()%10)*2 + (millis()%5)*5;
    value += ti;
    Serial.println(value);
    delay(245);
}