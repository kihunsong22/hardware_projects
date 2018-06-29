/*****************************************************************************
|*  SSAS Slideshow Example
|*
|* Devices:
|*  - LEDs and switches (WB_PRTIO)
|*  - TFT screen (VGA32_TFT)
|*  - JPEG decoder (WB_JPGDEC)
|*  - SD card (SPI_W + SD_CARD)
|*
|* Services used:
|*  - FAT filesystem (fs_fat)
|*  - VGA
|*  - PAL (processor abstraction layer)
|*
|* Description:
|*      This example shows images from an SD card on the TFT screen.
|*      The images must be in the root folder of the SD card.
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
#include <drv_vga_tft.h>
#include <fs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <timing.h>

#include "devices.h" // device IDs
#include "fade.h"

#define MIN( A, B )         ((A)<(B)?(A):(B))
#define MODE_FADE           0x01
#define MODE_WAIT           0x02
#define MODE_NEXT           0x04

#define XRES_TFT            240
#define YRES_TFT            320
#define TFT_BUFSIZE         XRES_TFT * YRES_TFT

#define MAX_CLUSTER_SIZE    4096

/*
 * The following storage must be located in external memory. This is achieved using the linker settings.
 */
#pragma section EXT_RAM
static uint8_t   diskbuf[MAX_CLUSTER_SIZE];
static uint16_t  image0[TFT_BUFSIZE] __align(4);
static uint16_t  image1[TFT_BUFSIZE] __align(4);
#pragma endsection

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
vga_tft_t           *tft;
uint16_t            *tft_buf;

void main( void )
{
    uint16_t            * curimg;            // Points to the current image
    uint16_t            * nextimg;           // Points to the next image
    uint8_t             mode;

    DIR                 *dir;
    struct dirent       *dirent;
    struct stat         statbuf;
    int                 jpgfile;
    size_t              filesize;

    mode         = MODE_FADE | MODE_WAIT;

    init();
    curimg       = image0;
    nextimg      = image1;
    vga_tft_set_screen(tft, (uintptr_t)tft_buf0);
    tft_buf = tft_buf1;

    // Mount filesystem
    *diskstatus = 0;
    do
    {
        // Try to mount first partition, if that does not succeed, try to mount the whole disk
        if ( mount("/dev/BLOCKIO_1", "/sdcard", "fatfs", 1, MOUNT_FLAG_RDONLY) == 0 ) break;
    } while( mount("/dev/BLOCKIO_1", "/sdcard", "fatfs", 0, MOUNT_FLAG_RDONLY) != 0 );

    *diskstatus = 1;

    if ( chdir("/sdcard") != 0)
    {
        exit(1);
    }

    /* main loop */
    while( 1 )
    {
        dir = opendir("");
        if ( dir == NULL )
        {
            exit(1);
        }

        while( dirent = readdir(dir), dirent )
        {
            mode &= ~MODE_NEXT;
            *leds = mode;

            if ( !strstr( dirent->d_name, "JPG" ) && !strstr( dirent->d_name, "jpg" )  )
            {
                continue;
            }

            if ( stat(dirent->d_name, &statbuf) != 0 )
            {
                continue;
            }
            filesize = statbuf.st_size;

            jpgfile = open(dirent->d_name, O_RDONLY);

            if ( jpgfile < 0 )
            {
                continue;
            }

            jpgdec_set_area(jpgdec, 0, 0, XRES_TFT, YRES_TFT );
            jpgdec_set_outputbuffer( jpgdec, (uintptr_t)nextimg,
                                     vga_tft_get_width( tft ) * vga_tft_get_height( tft ) * sizeof(uint16_t),
                                     vga_tft_get_width( tft ) );
            jpgdec_decode(jpgdec, 0, 0, 0);

            for (;;)
            {
                uint32_t status = jpgdec_get_status(jpgdec);

                if (status & JPGDEC_STATUS_READY)
                {
        //            finished!
                    break;
                }

                if (status & JPGDEC_STATUS_READEMPTY)
                {
                    if (filesize > 0)
                    {
                        int readsize = MIN( filesize, sizeof( diskbuf ));
                        read( jpgfile, diskbuf, readsize );
                        jpgdec_set_inputdata(jpgdec, diskbuf, readsize);
                        jpgdec_decode_continue(jpgdec, 0);
                        continue;
                    }
                    else
                    {
//                        print_status("JPG truncated, abort\n");
                        break;
                    }
                }

                if ( status != 0 )
                {
                    // unexpected
                    break;
                }
            }
            close( jpgfile );
            *diskstatus = 0;

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

                vga_tft_set_screen( tft, (uintptr_t)tft_buf );
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
        closedir(dir);
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
    // Initialize device drivers
    if ( tft = vga_tft_open(DRV_VGA_TFT_1 ), !tft )
    {
        abort();
    }

    if ( jpgdec = jpgdec_open( DRV_JPGDEC_1 ), !jpgdec )
    {
        abort();
    }

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

