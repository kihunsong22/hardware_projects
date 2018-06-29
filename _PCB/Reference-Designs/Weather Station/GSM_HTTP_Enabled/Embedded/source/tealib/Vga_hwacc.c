#include "vga.h"
#include "char.h"
#include "timer0.h"
#include "..\char\garamond.h"


#include "..\main.h"
#include "lcd0.h"

#define BANKSW  (*( volatile __sfr unsigned char *) 0xD1 )

#define VCMD    (*( volatile __sfr unsigned char *) 0xD9 )
#define VXPOS   (*( volatile __sfr unsigned int  *) 0xDA )
#define VXPOSH  (*( volatile __sfr unsigned char *) 0xDA )
#define VXPOSL  (*( volatile __sfr unsigned char *) 0xDB )
#define VYPOS   (*( volatile __sfr unsigned int  *) 0xDC )
#define VYPOSH  (*( volatile __sfr unsigned char *) 0xDC )
#define VYPOSL  (*( volatile __sfr unsigned char *) 0xDD )
#define VCOLOR  (*( volatile __sfr unsigned char *) 0xDE )

static __rom const unsigned char mask[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };


static void vga_plot2( unsigned short x, unsigned short y, unsigned char color1, unsigned char color2 );


/**********************************************************************
|*
|*  FUNCTION    : vga_init
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Clear the VGA memory
 */

void vga_init( void )
{
    /* wait until VGA-controller is ready to accept new command */
    while ( *( volatile __sfr unsigned char *) 0xD9 != 0 );
    
    /* fill whole screen with color black */
    VCOLOR = black;
    VCMD = 1;
}


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

void vga_plot( unsigned short x, unsigned short y, unsigned char color )
{
    /* wait until VGA-controller is ready to accept new command */
    while ( *( volatile __sfr unsigned char *) 0xD9 != 0 );

    /* Set x-pos, y-pos, and color */
    VXPOS = x;
    VYPOS = y;
    VCOLOR = color;
    /* write one pixel */
    VCMD = 2;
}


/**********************************************************************
|*
|*  FUNCTION    : vga_plot2
|*
|*  INPUT       : x, y = coordinates of the first pixel
|*                colors  = color(s) of the pixels
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Draw two pixels with given color(s), given x-coordinate must be an even pixel address
 */

static void vga_plot2( unsigned short x, unsigned short y, unsigned char color1, unsigned char color2 )
{
    /* wait until VGA-controller is ready to accept new command */
    while ( *( volatile __sfr unsigned char *) 0xD9 != 0 );

    /* Set x-pos, y-pos, and color */
    VXPOS = x;
    VYPOS = y;
    VCOLOR = ( color1 << 4 ) | color2;
    /* write two pixels */
    VCMD = 3;
}


/**********************************************************************
|*
|*  FUNCTION    : vga_line
|*
|*  INPUT       : x1, y1 = coordinates of startpoint
|*                x2, y2 = coordinates of endpoint
|*                color  = color to be used for drawing
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Draw a straight line between two points using Bresenham's
|*                integer-only algorithm
 */

void vga_line( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color )
{
    char    steep = 0;
    char    sx, sy;
    char    swap_char;
    int     swap_int;
    int     dx, dy;
    int     d, i;
    
    if ( x2 > x1 )
    {
        sx = 1;
        dx = x2 - x1;
    }
    else
    {
        sx = -1;
        dx = x1 - x2;
    }

    if ( y2 > y1 )
    {
        sy = 1;
        dy = y2 - y1;
    }
    else
    {
        sy = -1;
        dy = y1 - y2;
    }
        
    if ( dy > dx )
    {
        steep = 1;
        swap_int = x1; x1 = y1; y1 = swap_int;
        swap_int = dx; dx = dy; dy = swap_int;
        swap_char = sx; sx = sy; sy = swap_char;
    }
    
    d = (2 * dy) - dx;
    for ( i = 0; i < dx; i++ )
    {
        if ( steep )
        {
            vga_plot( y1, x1, color );
        }
        else
        {
            vga_plot( x1, y1, color );
        }
        while ( d >= 0 )
        {
            y1 = y1 + sy;
            d = d - (2 * dx);
        }
        x1 = x1 + sx;
        d = d + (2 * dy);
    }
    vga_plot( x2, y2, color );
}


/**********************************************************************
|*
|*  FUNCTION    : vga_cicrle
|*
|*  INPUT       : x0, y0 = coordinates of center point
|*                r      = radius
|*                color  = color to be used for drawing
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Draw a circle using Bresenham's integer-only algorithm
 */

void vga_circle( unsigned short x0, unsigned short y0, unsigned short r, unsigned char color )
{
    short   eps;
    short   x;
    short   y;
    x = 0;
    y = r;
    eps = 3 - (r << 1);
    while( x <= y )
    {
        vga_plot( (unsigned short) (x0 + x), (unsigned short) (y0 + y), color );
        vga_plot( (unsigned short) (x0 - x), (unsigned short) (y0 + y), color );
        vga_plot( (unsigned short) (x0 + x), (unsigned short) (y0 - y), color );
        vga_plot( (unsigned short) (x0 - x), (unsigned short) (y0 - y), color );

        vga_plot( (unsigned short) (x0 + y), (unsigned short) (y0 + x), color );
        vga_plot( (unsigned short) (x0 - y), (unsigned short) (y0 + x), color );
        vga_plot( (unsigned short) (x0 + y), (unsigned short) (y0 - x), color );
        vga_plot( (unsigned short) (x0 - y), (unsigned short) (y0 - x), color );

        if ( eps < 0 )
        {
            eps += (x << 2) + 6;
        }
        else
        {
            eps += ((x - y) << 2) + 10;
            y--;
        }
        x++;
    }

}


/**********************************************************************
|*
|*  FUNCTION    : vga_bitmap_2c
|*
|*  INPUT       : x, y     = coordinates of upper left corner
|*                bm       = pointer to bitmap
|*                fg_color = foreground color
|*                bg_color = background color
|*
|*  OUTPUT      : Width of the bitmap
|*
|*  DESCRIPTION : Draw a bitmap, 1 bit per pixel.
|*                Bit = 1: foreground; bit = 0: background.
 */

unsigned char vga_bitmap_2c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm, const unsigned char fg_color, const unsigned char bg_color )
{
    unsigned char i;
    unsigned char j;
    unsigned char width;
    unsigned char color1, color2;
    
    width = ( bm[0] & 0x07 ) ? ( bm[0] >> 3 ) + 1 : bm[0] >> 3;

    j = 0;
    
    if ( x % 2 )            /* odd x-pos? -> plot first vert line of bitmap */
    {
        for ( i = 0; i < bm[1]; i++ )   /* height */
        {
            color1 = ( bm[2 + ( i * width ) + ( j >> 3 )] & ( mask[j & 0x07])) ? fg_color : bg_color;
            vga_plot( x + j, y + i, color1 );    
        }
        j++;
    }
    
    while ( j < bm[0] - 1 ) /* plot bitmap-body */
    {
        for ( i = 0; i < bm[1]; i++ )   /* height */
        {
            color1 = ( bm[2 + ( i * width ) + ( j >> 3 )] & ( mask[j & 0x07])) ? fg_color : bg_color;
            color2 = ( bm[2 + ( i * width ) + (( j+1 ) >> 3 )] & ( mask[(j+1) & 0x07 ])) ? fg_color : bg_color;
            vga_plot2( x + j, y + i, color2, color1);
        }
        j += 2;
    }

    if ( j < bm[0] )        /* last x-pos not done? -> plot last vert line of bitmap*/
    {
        for ( i = 0; i < bm[1]; i++ )   /* height */
        {
            color1 = ( bm[2 + ( i * width ) + ( j >> 3 )] & ( mask[j & 0x07])) ? fg_color : bg_color;
            vga_plot( x + j, y + i, color1 );    
        }
    }
    
    return bm[0];
}


/**********************************************************************
|*
|*  FUNCTION    : vga_bitmap_16c
|*
|*  INPUT       : x, y     = coordinates of upper left corner
|*                bm       = pointer to bitmap
|*
|*  OUTPUT      : Width of the bitmap
|*
|*  DESCRIPTION : Draw a bitmap, 4 bit per pixel.
 */

unsigned char vga_bitmap_16c( const unsigned short x, const unsigned short y, __rom const unsigned char *bm )
{
    unsigned char i, j;
    unsigned char width;
    unsigned char color1, color2;
    
    width = ( bm[0] >> 1 ) + ( bm[0] & 0x01 );

    i = 0;
    
    if ( x % 2 )            /* odd x-pos? -> plot first vert line of bitmap */
    {
        for ( j = 0; j < bm[1]; j++ )   /* height */
        {
            color1 = bm[2 + ( j * width )] >> 4; 
            vga_plot( x, y + j, color1 );
        }
        i++;
    }
    
    while ( i < bm[0] - 1 ) /* plot bitmap-body */
    {
        for ( j = 0; j < bm[1]; j++ )   /* height */
        {
            if ( i % 2 )
            {
                color1 = bm[2 + ( j * width ) + (( i+1 ) >> 1 )] >> 4;
                color2 = bm[2 + ( j * width ) + ( i >> 1 )] & 0x0F;
            }
            else
            {
                color2 = bm[2 + ( j * width ) + ( i >> 1 )] >> 4;
                color1 = bm[2 + ( j * width ) + ( i >> 1 )] & 0x0F;
            }
            vga_plot2( x + i , y + j, color1, color2 );
        }
        i += 2;
    }

    if ( i < bm[0] )        /* last x-pos not done? -> plot last vert line of bitmap*/
    {
        for ( j = 0; j < bm[1]; j++ )   /* height */
        {
            vga_plot( x + i, y + j, bm[2 + ( j * width ) + ( i >> 1 )] >> 4 );
        }
    }

    return bm[0];
}


/**********************************************************************
|*
|*  FUNCTION    : vga_getbmwidth
|*
|*  INPUT       : bm = pointer to bitmap
|*
|*  OUTPUT      : Width of the bitmap
|*
|*  DESCRIPTION : Get the width of a bitmap
 */

unsigned char vga_getbmwidth( __rom const unsigned char *bm )
{
    return bm[0]; 
}


/**********************************************************************
|*
|*  FUNCTION    : vga_getbmheight
|*
|*  INPUT       : bm = pointer to bitmap
|*
|*  OUTPUT      : Height of the bitmap
|*
|*  DESCRIPTION : Get the height of a bitmap
 */

unsigned char vga_getbmheight( __rom const unsigned char *bm )
{
    return bm[1]; 
}


/**********************************************************************
|*
|*  FUNCTION    : vga_fill
|*
|*  INPUT       : x1, y1 = coordinates of first corner of block
|*              : x2, y2 = coordinates of opposite corner of block
|*                color  = color to be used for filling
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Draw a circle using Bresenham's integer-only algorithm
 */

void vga_fill( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, unsigned char color )
{
    short x, xstop, ystart, ystop, y;
    
    if ( x1 > x2 )
    {
        x = x2;
        xstop = x1;
    }
    else
    {
        x = x1;
        xstop = x2;
    }

    if ( y1 > y2 )
    {
        ystart = y2;
        ystop = y1 + 1;
    }
    else
    {
        ystart = y1;
        ystop = y2 + 1;
    }

    if ( x % 2 )            /* first pixel on odd x-pos? -> plot first line */
    {
        for ( y = ystart; y < ystop; y++ )
        {
            vga_plot( x, y, color );
        }
        x++;
    }
    
    while ( x < xstop )        /* plot block, two pixels a time */
    {
        for ( y = ystart; y < ystop; y++ )
        {
            vga_plot2( x, y, color, color );
        }
        x += 2;
    }
    
    if ( x != xstop )          /* all x done, else plot last line */
    {
        for ( y = ystart; y < ystop; y++ )
        {
            vga_plot( x, y, color );
        }
    }
}


__rom unsigned char *vga_charset( const char c )
{
    __rom unsigned char *ret_val = 0;
    
    if ( c >= '0' && c <= '9' )
    {
        ret_val = garamond[c-'0'];
    }
    else if ( c == '-' )
    {
        ret_val = garamond[10];
    }
    else if ( c == '.' )
    {
        ret_val = garamond[11];
    }
    else if ( c == 'N' )
    {
        ret_val = garamond[12];
    }
    else if ( c == 'E' )
    {
        ret_val = garamond[13];
    }
    else if ( c == 'S' )
    {
        ret_val = garamond[14];
    }
    else if ( c == 'W' )
    {
        ret_val = garamond[15];
    }

    return ret_val;
}

