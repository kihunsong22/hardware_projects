#define TX_DATA 3
#define TX_RATE 5 // custom baud rate

const char *message = "transmission message";

void setup(){
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TX_DATA, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    for (int byte_idx = 0; byte_idx < strlen(message); byte_idx++){
        char tx_byte = message(byte_idx);

        for(int bit_idx = 0; bit idx < 8; bit_idx++){
            bool tx_bit = tx_byte & (0x80 >> bit_idx);

            digitalWrite(TX_DATA, tx_bit);
            delay(1000/TX_RATE);
        }
    }

    digitalWrite(TX_DATA, LOW);
}

void loop(){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}