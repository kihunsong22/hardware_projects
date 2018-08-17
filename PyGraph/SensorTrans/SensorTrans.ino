int value = 0;
int i = 0;

void setup(){
    pinMode(A0, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(115200);
    Serial.println("Serial connected");
}

void loop(){
    value = analogRead(A0);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Value: ");
    Serial.println(value);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
}