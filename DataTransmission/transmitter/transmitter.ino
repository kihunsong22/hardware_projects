#define TX_DATA 4
#define TX_RATE 20 // custom baud rate

const char *message = "transmission message";

void setup(){
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TX_DATA, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    for(int rep=0; rep<3; rep++){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
    }
    digitalWrite(TX_DATA, HIGH);
    digitalWrite(TX_DATA, LOW);

    for (int byte_idx = 0; byte_idx < strlen(message); byte_idx++){
        char tx_byte = message[byte_idx];

        for(int bit_idx = 0; bit_idx < 8; bit_idx++){
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