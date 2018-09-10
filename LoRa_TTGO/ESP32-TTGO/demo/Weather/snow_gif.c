#define snow_width 50
#define snow_height 50
const char snow_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0xB0, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x1F, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xF0, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC3, 
  0x87, 0x01, 0x00, 0x00, 0x00, 0x20, 0x83, 0x83, 0x09, 0x00, 0x00, 0x00, 
  0xF0, 0x83, 0xC1, 0x1F, 0x00, 0x00, 0x00, 0xF0, 0x87, 0xC3, 0x0F, 0x00, 
  0x00, 0x00, 0xC0, 0x87, 0xE3, 0x07, 0x00, 0x00, 0x00, 0xE0, 0xDF, 0xF7, 
  0x0F, 0x00, 0x00, 0x00, 0xF0, 0xFF, 0x7F, 0x1F, 0x00, 0x00, 0x00, 0x20, 
  0x70, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x1C, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x70, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x70, 0xF8, 0x3F, 0x0E, 
  0x00, 0x00, 0x00, 0xF0, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0xC0, 0x8F, 
  0xF3, 0x07, 0x00, 0x00, 0x00, 0xE0, 0x87, 0xC3, 0x07, 0x40, 0x00, 0x00, 
  0xF0, 0x83, 0xC1, 0x1F, 0x60, 0x00, 0x00, 0x70, 0x83, 0xC3, 0x1D, 0x50, 
  0x01, 0x00, 0x00, 0xC3, 0x87, 0x01, 0xF8, 0x03, 0x00, 0x00, 0xE3, 0x8F, 
  0x01, 0xF3, 0x09, 0x00, 0x00, 0xF0, 0x1F, 0xC0, 0xE3, 0x58, 0x00, 0x00, 
  0xB0, 0x1F, 0xC0, 0x63, 0x7C, 0x00, 0x00, 0x80, 0x03, 0x80, 0x47, 0x3C, 
  0x00, 0x00, 0x80, 0x01, 0xC0, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xC0, 
  0xF8, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0xC0, 0xF8, 0x23, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x7F, 0x00, 
  0x00, 0x00, 0x00, 0x80, 0x67, 0x3C, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x43, 
  0x7C, 0x00, 0x00, 0x00, 0x00, 0x40, 0xE3, 0x68, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xF3, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x03, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, };