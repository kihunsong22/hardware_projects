String data;

void setup(){
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);
}

void loop(){
    if(Serial.available()){
        data = Serial.read();

        Serial.println(data);
        Serial.println();

        // if( data.toInt()==1 ){
        //     digitalWrite(LED_BUILTIN, HIGH);
        // }else{
        //     digitalWrite(LED_BUILTIN, LOW);
        // }
    }
}