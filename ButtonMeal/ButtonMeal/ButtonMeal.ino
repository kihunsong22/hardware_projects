#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

String data;

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {
    Serial.begin(115200);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    display.display();

    display.setTextSize(2);
    display.setCursor(10, 0);
    display.clearDisplay();
    display.println("INPUT YOUR TEXT HERE: ");

    display.invertDisplay(true);
    display.invertDisplay(false);
    display.display();
    display.clearDisplay();
}


void loop() {
    while (1) {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 0);
    display.clearDisplay();
    printText();
    display.display();

    }
}

void printText(void) {
    if (Serial.available()){
        data = Serial.readString();
        if( data.length()>1 ){
            display.println(data);
            display.display();
            display.println("\n");
        }
    }

    while( !Serial.available()){
        delay(1);
    }
}

// #include "U8glib.h"

// U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

// void setup() {
// }

// void loop() {
//   u8g.firstPage();
//   do {
//     u8g.setFont(u8g_font_unifont);
//     u8g.setPrintPos(0, 10);
//     u8g.print("Hello, world");
//     u8g.drawStr(0, 30, "Hello, world!");
//   } while (u8g.nextPage());
// }
