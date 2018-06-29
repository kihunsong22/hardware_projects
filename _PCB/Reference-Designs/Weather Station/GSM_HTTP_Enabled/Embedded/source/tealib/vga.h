#ifndef VGA_H
#define VGA_H

enum {  black = 0,
        darkred,
        darkgreen,
        darkyellow,
        darkblue,
        darkmagenta,
        darkcyan,
        darkgrey,
        grey,
        red,
        green,
        yellow,
        blue,
        magenta,
        cyan,
        white
     };
        
void vga_init( void );
void vga_plot( unsigned short x, unsigned short y, unsigned char color );
void vga_line( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color );
void vga_circle( unsigned short x0, unsigned short y0, unsigned short r, unsigned char color );
unsigned char vga_getbmwidth( __rom const unsigned char *bm );
unsigned char vga_getbmheight( __rom const unsigned char *bm );
unsigned char vga_bitmap_2c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm, const unsigned char fg_color, const unsigned char bg_color );
unsigned char vga_bitmap_16c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm );
__rom unsigned char *vga_charset( const char c );
__rom unsigned char *vga_charsets( const char c );
void vga_fill( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color );

#endif
