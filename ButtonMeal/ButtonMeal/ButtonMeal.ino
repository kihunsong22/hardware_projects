#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

void setup() {
}

void loop() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_unifont);
    u8g.setPrintPos(0, 10);
    u8g.print("Hello, world");
    u8g.drawStr(0, 30, "Hello, world!");
  } while (u8g.nextPage());
}


// String data;
// void setup(){
//     Serial.begin(115200);
//     pinMode(LED_BUILTIN, OUTPUT);

//     digitalWrite(LED_BUILTIN, LOW);
// }

// void loop(){
//     if(Serial.available()){
//         data = Serial.read();

//         Serial.println(data);
//         Serial.println();
//     }
// }