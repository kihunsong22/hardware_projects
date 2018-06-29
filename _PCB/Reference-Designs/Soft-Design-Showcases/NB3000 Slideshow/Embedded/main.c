/*****************************************************************************
|*  Slideshow Example
|*
|* Devices:
|*  - LEDs and switches (PRTIO)
|*  - TFT screen (TFT)
|*  - JPEG decoder (JPGDEC)
|*
|* Services used:
|*  - VGA
|*  - PAL (processor abstraction layer)
|*
|* Description:
|*  This example shows images from memory on the TFT screen.
|*
\*****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include <drv_jpgdec.h>
#include <per_ioport.h>
#include <drv_vga_ili9320.h>
#include <timing.h>
#include <drv_pwmx.h>

#include "devices.h" // device IDs
#include "fade.h"

#define MIN( A, B )         ((A)<(B)?(A):(B))
#define MODE_FADE           0x01
#define MODE_WAIT           0x02
#define MODE_NEXT           0x04

#define XRES_TFT            320
#define YRES_TFT            240
#define TFT_BUFSIZE         XRES_TFT * YRES_TFT

#define MAX_CLUSTER_SIZE    4096

#define BRIGHT_MAX 0xFF
#define BRIGHT_MIN 0x10

/*
 * The following storage must be located in external memory. This is achieved using the linker settings.
 */
#pragma section EXT_RAM
static uint16_t  image0[TFT_BUFSIZE] __align(4);
static uint16_t  image1[TFT_BUFSIZE] __align(4);
#pragma endsection

size_t image0_size;

#pragma section EXT_RAM2
static uint16_t  tft_buf0[TFT_BUFSIZE] __align(4);
static uint16_t  tft_buf1[TFT_BUFSIZE] __align(4);
#pragma endsection

static uint8_t  modeswitches( uint8_t mode );
static void init( void );

volatile uint8_t    *diskstatus;
volatile uint8_t    *switches;
volatile uint8_t    *leds;
jpgdec_t            *jpgdec;
vga_ili9320_t       *tft;
uint16_t            *tft_buf;
pwmx_t              *pwm;

#define IMAGES_COUNT 20

extern uint8_t _lc_ub_img1_jpg[];
extern uint8_t _lc_ue_img1_jpg[];

extern uint8_t _lc_ub_img2_jpg[];
extern uint8_t _lc_ue_img2_jpg[];

extern uint8_t _lc_ub_img3_jpg[];
extern uint8_t _lc_ue_img3_jpg[];

extern uint8_t _lc_ub_img4_jpg[];
extern uint8_t _lc_ue_img4_jpg[];

extern uint8_t _lc_ub_img5_jpg[];
extern uint8_t _lc_ue_img5_jpg[];

extern uint8_t _lc_ub_img6_jpg[];
extern uint8_t _lc_ue_img6_jpg[];

extern uint8_t _lc_ub_img7_jpg[];
extern uint8_t _lc_ue_img7_jpg[];

extern uint8_t _lc_ub_img8_jpg[];
extern uint8_t _lc_ue_img8_jpg[];

extern uint8_t _lc_ub_img9_jpg[];
extern uint8_t _lc_ue_img9_jpg[];

extern uint8_t _lc_ub_img10_jpg[];
extern uint8_t _lc_ue_img10_jpg[];

extern uint8_t _lc_ub_img11_jpg[];
extern uint8_t _lc_ue_img11_jpg[];

extern uint8_t _lc_ub_img12_jpg[];
extern uint8_t _lc_ue_img12_jpg[];

extern uint8_t _lc_ub_img13_jpg[];
extern uint8_t _lc_ue_img13_jpg[];

extern uint8_t _lc_ub_img14_jpg[];
extern uint8_t _lc_ue_img14_jpg[];

extern uint8_t _lc_ub_img15_jpg[];
extern uint8_t _lc_ue_img15_jpg[];

extern uint8_t _lc_ub_img16_jpg[];
extern uint8_t _lc_ue_img16_jpg[];

extern uint8_t _lc_ub_img17_jpg[];
extern uint8_t _lc_ue_img17_jpg[];

extern uint8_t _lc_ub_img18_jpg[];
extern uint8_t _lc_ue_img18_jpg[];

extern uint8_t _lc_ub_img19_jpg[];
extern uint8_t _lc_ue_img19_jpg[];

uint8_t* images_lc_ub[IMAGES_COUNT] = { _lc_ub_img1_jpg,
                                        _lc_ub_img2_jpg,
                                        _lc_ub_img3_jpg,
                                        _lc_ub_img4_jpg,
                                        _lc_ub_img5_jpg,
                                        _lc_ub_img6_jpg,
                                        _lc_ub_img7_jpg,
                                        _lc_ub_img8_jpg,
                                        _lc_ub_img9_jpg,
                                        _lc_ub_img10_jpg,
                                        _lc_ub_img11_jpg,
                                        _lc_ub_img12_jpg,
                                        _lc_ub_img13_jpg,
                                        _lc_ub_img14_jpg,
                                        _lc_ub_img15_jpg,
                                        _lc_ub_img16_jpg,
                                        _lc_ub_img17_jpg,
                                        _lc_ub_img18_jpg,
                                        _lc_ub_img19_jpg
                                      };

uint8_t* images_lc_ue[IMAGES_COUNT] = { _lc_ue_img1_jpg,
                                        _lc_ue_img2_jpg,
                                        _lc_ue_img3_jpg,
                                        _lc_ue_img4_jpg,
                                        _lc_ue_img5_jpg,
                                        _lc_ue_img6_jpg,
                                        _lc_ue_img7_jpg,
                                        _lc_ue_img8_jpg,
                                        _lc_ue_img9_jpg,
                                        _lc_ue_img10_jpg,
                                        _lc_ue_img11_jpg,
                                        _lc_ue_img12_jpg,
                                        _lc_ue_img13_jpg,
                                        _lc_ue_img14_jpg,
                                        _lc_ue_img15_jpg,
                                        _lc_ue_img16_jpg,
                                        _lc_ue_img17_jpg,
                                        _lc_ue_img18_jpg,
                                        _lc_ue_img19_jpg
                                      };

void main( void )
{
    uint16_t            * curimg;            // Points to the current image
    uint16_t            * nextimg;           // Points to the next image
    uint8_t             mode;
    uint32_t            i;

    mode         = MODE_FADE | MODE_WAIT;

    init();
    curimg       = image0;
    nextimg      = image1;
    vga_ili9320_set_screen(tft, (uintptr_t)tft_buf0);
    tft_buf = tft_buf1;

    while( 1 )
    {
        for ( i = 0 ; i < IMAGES_COUNT ; i++ )
        {
            image0_size = (images_lc_ue[i] - images_lc_ub[i] + 3) & ~3;

            jpgdec_set_area(jpgdec, 0, 0, XRES_TFT, YRES_TFT );
            jpgdec_set_outputbuffer( jpgdec, (uintptr_t)nextimg,
                                     vga_ili9320_get_width( tft ) * vga_ili9320_get_height( tft ) * sizeof(uint16_t),
                                     vga_ili9320_get_width( tft ) );

            jpgdec_decode( jpgdec, images_lc_ub[i], image0_size, 0 );

            for (;;)
            {
                uint32_t status = jpgdec_get_status(jpgdec);

                if (status & JPGDEC_STATUS_READY)
                {
                    //finished!
                    break;
                }

                if (status & JPGDEC_STATUS_READEMPTY)
                {
                    if (image0_size > 0)
                    {
                        jpgdec_set_inputdata(jpgdec, images_lc_ub[i], image0_size);
                        jpgdec_decode_continue(jpgdec, 0);
                        continue;
                    }
                    else
                    {
                        //print_status("JPG truncated, abort\n");
                        break;
                    }
                }

                if ( status != 0 )
                {
                    // unexpected
                    break;
                }
            }

            mode = modeswitches( mode );

            // switch display or crossfade
            for ( uint16_t step = (mode & MODE_FADE) ? 0 : 256; step <= 256; step+=8 )
            {
                mode = modeswitches( mode );
                if ((mode & MODE_FADE) == 0)
                {
                    step = 256;
                }
                *leds = mode | (step & 0xf8);

                fade( curimg, nextimg, tft_buf, step, TFT_BUFSIZE );

                vga_ili9320_set_screen( tft, (uintptr_t)tft_buf );
                tft_buf = tft_buf == tft_buf0 ? tft_buf1 : tft_buf0;
            }

            if (mode & MODE_WAIT)
            {

                clock_t tick = clock() + 3 * freq_hz();

                while ( tick > clock() && (mode & MODE_WAIT) )
                {
                    mode = modeswitches( mode );
                }
            }

            // wait for keypress when in pause mode
            while (mode == 0)
            {
                mode = modeswitches( mode );
            }

            curimg = (curimg == image1) ? image0 : image1;
            nextimg = (nextimg == image1) ? image0 : image1;
        }
    }
}

/**********************************************************************
|*
|*  FUNCTION    : init
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Initialize the hardware and drivers
 */

static void init( void )
{
    // Set clock speed
    __clocks_per_sec = freq_hz();

    // Initialize device drivers
    if ( tft = vga_ili9320_open( DRV_VGA_ILI9320_1 ), !tft )
    {
        abort();
    }

    if ( jpgdec = jpgdec_open( DRV_JPGDEC_1 ), !jpgdec )
    {
        abort();
    }    

    pwm = pwmx_open(DRV_PWMX_1);
    pwmx_set_resolution_mode(pwm, PWMX_MODE_8BIT);
    pwmx_set_frequency(pwm, 10000);
    pwmx_enable_controller(pwm);
    pwmx_set_pulsewidth(pwm, BRIGHT_MAX);

    if ( leds = (void *)per_ioport_get_base_address(PRTIO), !leds )
    {
        abort();
    }
    diskstatus = leds + 1;
    switches = diskstatus;
}

/**********************************************************************
|*
|*  FUNCTION    : modeswitches
|*
|*  PARAMETERS  : mode = current mode
|*
|*  RETURNS     : new mode
|*
|*  DESCRIPTION : change mode variable if user pressed any of the pushbuttons
 */
static uint8_t modeswitches( uint8_t mode )
{
    uint8_t pressed = ~ (*switches) & 0x1F;

    if (pressed & 0x01)
    {
        mode = 0;
    }
    else if (pressed & 0x02)
    {
        mode = MODE_NEXT;
    }
    else if (pressed & 0x04)
    {
        mode = MODE_WAIT;
    }
    else if (pressed & 0x08)
    {
        mode = MODE_FADE;
    }
    else if (pressed & 0x10)
    {
        mode = MODE_FADE | MODE_WAIT;
    }

    *leds = mode;

    if (pressed)
    {
        while (~ (*switches) & 0x1F)
        {
            delay_ms(100);
        }
    }

    return mode;
}

