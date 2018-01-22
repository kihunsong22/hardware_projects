/*
#include <IRremote.h>

int RECV_PIN = 11;
int led = 13;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(led, LOW);
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}
*/
#include <IRremote.h>

int RECV_PIN = 11;
int led = 13;
char recv;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(13, OUTPUT);
}

void loop() {
  digitalWrite(led, LOW);
  if (irrecv.decode(&results)) {
    Serial.println(results.value, DEC);
    digitalWrite(led, HIGH);
    irrecv.resume(); // Receive the next value
    delay(200);
    digitalWrite(led, LOW);
  }
}
