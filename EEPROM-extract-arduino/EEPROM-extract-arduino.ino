// source: https://www.youtube.com/watch?v=a6EWIh2D1NQ&t=730s
// extracts data from EEPROM 93LC76

#define CS 2
#define CLK 3
#define DI 4
#define DO 5

void sendInstruction(byte instruction){
    for(word mask = 0b10000000000000; mask > 0; mask >>= 1){
        if(instruction & mask){
            digitalWrite(DI, HIGH);
        }else{
            digitalWrite(DI, LOW);
        }
        digitalWrite(CLK, HIGH);
        digitalWrite(CLK, LOW);
    }
}

byte readByte(){
    byte data = 0;
    for(byte bit = 0; bit < 8; bit += 1){
        digitalWrite(CLK, HIGH);
        digitalWrite(CLK, LOW);
        if(digitalRead(DO)){
            data = data << 1 | 1;
        }else{
            data = data << 1;
        }
    }

    return data;
}

void setup(){
    Serial.begin(115200);

    pinMode(CS, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(DI, OUTPUT);
    pinMode(DO, INPUT);

    byte line[16] = { 0 };
    for(word address = 0; address < 2048; address += 16){
        // compose address
        char addrPrint[6] = { 0 };
        sprintf(addrPrint, "04x  ", address);
        Serial.print(addrPrint);

        // send read instruction
        digitalWrite(CS, HIGH);
        sendInstruction(0b11000000000000 + address);
        for(int i = 0; i < 16; i += 1){
            line[i] = readByte();
        }
        digitalWrite(CS, LOW);

        // receive response data
        for(int i = 0; i < 16; i += 1){
            char linePrint[4] = { 0 };
            sprintf(linePrint, "02x", line[i]);
            Serial.print(linePrint);
        }

        // print response
        for(int i = 0; i < 16; i += 1){
            if(line[i]<32 || line[i] > 126){  // non-printable character
                Serial.print(".");
            }else{
                Serial.print((char)line[i]);
            }
        }
        Serial.println();
    }
}

void loop(){
    ;
}
