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
