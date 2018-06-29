#ifndef VGA_H
#define VGA_H

/* defines of VGA controller */

#define VCMD    (*( volatile __sfr unsigned char *) 0xD9 )
#define VXPOS   (*( volatile __sfr unsigned int  *) 0xDA )
#define VXPOSH  (*( volatile __sfr unsigned char *) 0xDA )
#define VXPOSL  (*( volatile __sfr unsigned char *) 0xDB )
#define VYPOS   (*( volatile __sfr unsigned int  *) 0xDC )
#define VYPOSH  (*( volatile __sfr unsigned char *) 0xDC )
#define VYPOSL  (*( volatile __sfr unsigned char *) 0xDD )
#define VCOLOR  (*( volatile __sfr unsigned char *) 0xDE )


/* enums */

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


/* inline functions */

/**********************************************************************
|*
|*  FUNCTION    : vga_plot
|*
|*  INPUT       : x, y = coordinates of pixel
|*                color  = color of the pixel
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Draw one pixel with given color
 */

inline void vga_plot( unsigned short x, unsigned short y, unsigned char color )
{
    /* wait until VGA-controller is ready to accept new command */
    while ( VCMD != 0 );

    /* Set x-pos, y-pos, and color */
    VXPOS = x;
    VYPOS = y;
    VCOLOR = color;
    /* write one pixel */
    VCMD = 2;
}


/* function prototypes */

void vga_init( void );
void vga_line( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color );
void vga_circle( unsigned short x0, unsigned short y0, unsigned short r, unsigned char color );
unsigned char vga_getbmwidth( __rom const unsigned char *bm );
unsigned char vga_getbmheight( __rom const unsigned char *bm );
unsigned char vga_bitmap_2c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm, const unsigned char fg_color, const unsigned char bg_color );
unsigned char vga_bitmap_16c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm );
void vga_fill( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color );

#endif
