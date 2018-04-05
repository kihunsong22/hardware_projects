#include <SoftwareSerial.h>
SoftwareSerial ble(3, 2); // 아두이노의 RX, TX핀을 설정

void setup() {
  Serial.begin(9600);
  Serial.println("Serial connected");
  Serial.println();
  ble.begin(9600);
  ble.print("AT");
}

void loop() {
    if (ble.available()) {
    Serial.write(ble.read());
  }
  if (Serial.available()) {
    ble.write(Serial.read());
  }
}
