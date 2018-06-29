// Project 18 - Oscilloscope

#define CHANNEL_A_PIN 0

void setup()                 
{
 Serial.begin(115200);
//Serial.begin(9600);

}

void loop()                   
{
  int value = analogRead(CHANNEL_A_PIN);
  value = (value >> 2) & 0xFF;
  Serial.print(value, BYTE);
  delayMicroseconds(100);
}
