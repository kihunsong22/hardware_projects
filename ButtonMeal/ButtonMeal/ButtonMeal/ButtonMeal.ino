unsigned int i = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
}

void loop() {
    Serial.print("TRA: ");
    Serial.println(i);
    i++;
    delay(500);
}