#include <Wire.h>
#include "U8glib.h"
// #include "fonts_kor.h"  //kfont
// #include "fonts_eng.h"  //Open_Sans_Hebrew_Condensed_14, Open_Sans_Hebrew_Condensed_18, Open_Sans_Hebrew_Condensed_24

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

void setup() {
	Serial.begin(115200);
}

void loop() {
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_unifont);
    // u8g.setPrintPos(0, 10);
    // u8g.print("안녕하세요");
    u8g.drawStr(0, 0, "Hello World");
    // u8g.setPrintPos(20, 10);
    u8g.drawStr(30, 30, "헬로우 월드");
  } while (u8g.nextPage());  // pause until update has finished
}
