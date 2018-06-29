/*****************************************************************************\
|*
|*  IN PACKAGE:         Mandelbrot
|*
|*  COPYRIGHT:          Copyright (c) 2008, Altium
|*
|*  DESCRIPTION:        Draw Mandelbrot on TFT
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

// Include the embedded application stack software
#include <timing.h>
#include <devices.h>
#include <drv_vga_tft.h>
#include <touchscreen.h>

#define XRES    240             // Screen width in pixels
#define YRES    320             // Screen height in pixels
#define MAXITER 128             // Maximum number of iterations
#define LIMIT   4.0             // Divergence check value

static void init(void);
static void drawbrot( float x_center, float y_center, float zoom_scale );
static void draw_pixel(uint16_t *screen, int x, int y, uint16_t color);

uint16_t      video[XRES * YRES];
vga_tft_t     * tft;
touchscreen_t * ts;

/**********************************************************************
|*
|*  FUNCTION    : main
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Draw the bot
 */

void main( void )
{
    float cx = -0.5;            // Horizontal center of brot
    float cy = 0.0;             // Vertical center of brot
    float scale = 0.012;      // Zoom factor
    int x = 0;
    int y = 0;

    unsigned state = 0;

    init();


    drawbrot( cx, cy, scale );

    for (;;)
    {
        touchscreen_data_t pos;
        if ( touchscreen_get_pos( ts, &pos ) )
        {
            switch( state )
            {
            case 0 :
                if ( pos.pendown )  // Left button make event
                {
                    x = pos.x;
                    y = pos.y;
                    state = 1;
                }
                break;
            case 1 :

                if ( !pos.pendown ) // Left button release event
                {
                    float x_zoom, y_zoom;

                    x_zoom = fabsf( (x - pos.x) / (float)XRES );
                    y_zoom = fabsf( (y - pos.y) / (float)YRES );

                    // Note: the bot is rotated by 90 degrees => X/Y are swapped!

                    x = (x + pos.x - XRES) / 2;
                    cy += x * scale;

                    y = (y + pos.y - YRES) / 2;
                    cx += y * scale;


                    scale *= ( x_zoom > y_zoom ) ? x_zoom : y_zoom;
                    drawbrot( cx, cy, scale );
                    state = 0;
                }
                else
                {
                    int x0, x1, y0, y1;
                    if ( x > pos.x )
                    {
                        x0 = pos.x;
                        x1 = x;
                    }
                    else
                    {
                        x0 = x;
                        x1 = pos.x;
                    }
                    if ( y > pos.y )
                    {
                        y0 = pos.y;
                        y1 = y;
                    }
                    else
                    {
                        y0 = y;
                        y1 = pos.y;
                    }
                    for ( int ypos = y0; ypos <= y1; ypos++ )
                    {
                        int offs = ypos * 240;
                        video[offs + x0] = ~video[offs + x0];
                        video[offs + x1] = ~video[offs + x1];
                    }

                    {
                        int offs0 = y0 * 240;
                        int offs1 = y1 * 240;
                        for ( int xpos = x0; xpos <= x1; xpos++ )
                        {
                            video[offs0 + xpos] = ~video[offs0 + xpos];
                            video[offs1 + xpos] = ~video[offs1 + xpos];
                        }
                    }
                    for ( clock_t timeout = clock() + CLOCKS_PER_SEC / 30; clock() < timeout; ) ;// Do nothing ;
                    for ( int ypos = y0; ypos <= y1; ypos++ )
                    {
                        int offs = ypos * XRES;
                        video[offs + x0] = ~video[offs + x0];
                        video[offs + x1] = ~video[offs + x1];
                    }

                    {
                        int offs0 = y0 * XRES;
                        int offs1 = y1 * XRES;
                        for ( int xpos = x0; xpos <= x1; xpos++ )
                        {
                            video[offs0 + xpos] = ~video[offs0 + xpos];
                            video[offs1 + xpos] = ~video[offs1 + xpos];
                        }
                    }
                }
                break;
            default : // Should never come here
                state = 0;
            }
        }
    }
}

/********************************************************************
|*
|*  FUNCTION    : init
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Initialize hardware
 */

static void init(void)
{
    // Initialize VGA
    tft = vga_tft_open( DRV_VGA_TFT_0 );
    vga_tft_set_screen(tft, (uintptr_t)video);
    ts = touchscreen_open( TOUCHSCREEN_0 );
}

/********************************************************************
|*
|*  FUNCTION    : drawbrot
|*
|*  PARAMETERS  : x_center, y_center = part of mandelbrot to center on
|*                scale = convertion scale from coordinates to display
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Draw the mandelbrot
 */

static void drawbrot( float x_center, float y_center, float zoom_scale )
{
    int lp;                             // Convergence check value.
    float xcur,ycur,xnext,ynext;        // For calculating the iterations.
    float ax,ay;                        // The actual position of (x,y) in relation to the Mandelbrot set.
    int x,y;                            // The pixel we are drawing.

    for ( x = -YRES / 2; x <  YRES / 2; x++ )
    {
        for( y = -XRES / 2; y < XRES / 2; y++ )
        {
            // Scale and shift display position to mathematical position
            ax = x_center + x * zoom_scale;
            ay = y_center + y * zoom_scale;

            // Mandelbrot formula
            xcur = ax;
            ycur = ay;
            for (lp = MAXITER; lp; lp-- )
            {
                xnext = xcur * xcur - ycur * ycur + ax;
                ynext = xcur * 2.0 * ycur + ay;

                xcur = xnext;
                ycur = ynext;
                if ( (xcur * xcur + ycur * ycur) > LIMIT ) break;
            }

            // Define colour and draw pixel.
            draw_pixel( video, y+XRES/2, x + YRES / 2, lp | (lp << 5) | (lp << 11) );
        }
    }
}


static void draw_pixel(uint16_t *screen, int x, int y, uint16_t color)
{
    uint16_t *pixel;

    pixel = screen + x + y * XRES;
    *pixel = color;
}



