
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include <drv_jpgdec.h>
#include <drv_vga_tft.h>
#include <fs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <timing.h>

#include "devices.h" // device IDs
#include "fade.h"

#define XRES_TFT            240
#define YRES_TFT            320
#define TFT_BUFSIZE         XRES_TFT * YRES_TFT

#define MAX_CLUSTER_SIZE    4096
#define MIN( A, B )         ((A)<(B)?(A):(B))

/*
 * The following storage must be located in external memory. This is achieved using the linker settings.
 */
#pragma section EXT_RAM
static uint8_t   diskbuf[MAX_CLUSTER_SIZE];
static uint16_t  image0[TFT_BUFSIZE];
static uint16_t  image1[TFT_BUFSIZE];
static uint16_t  tft_buf0[TFT_BUFSIZE];
static uint16_t  tft_buf1[TFT_BUFSIZE];
#pragma endsection

void * slideshow_thread( void* argc )
{
    uint16_t            *tft_buf;           // Points to the current video page
    uint16_t            *curimg;            // Points to the current image
    uint16_t            *nextimg;           // Points to the next image
    jpgdec_t            *jpgdec;
    vga_tft_t           *tft;

    DIR                 *dir;
    struct dirent       *dirent;
    struct stat         statbuf;
    int                 jpgfile;
    size_t              filesize;

    // Initialize device drivers
    if ( tft = vga_tft_open(DRV_VGA_TFT_1 ), !tft )
    {
        return NULL;
    }

    if ( jpgdec = jpgdec_open( DRV_JPGDEC_1 ), !jpgdec )
    {
        return NULL;
    }

    curimg       = image0;
    nextimg      = image1;
    vga_tft_set_screen( tft, (uintptr_t)tft_buf0 );
    tft_buf = tft_buf1;

    /* main loop */
    while( 1 )
    {
        // Mount filesystem

        do
        {
            // Try to mount first partition, if that does not succeed, try to mount the whole disk
            if ( mount("/dev/BLOCKIO_1", "/sdcard", "fatfs", 1, MOUNT_FLAG_RDONLY) == 0 ) break;
        } while( mount("/dev/BLOCKIO_1", "/sdcard", "fatfs", 0, MOUNT_FLAG_RDONLY) != 0 );


        if ( chdir("/sdcard") != 0)
        {
            continue;
        }

        if ( dir = opendir(""), dir == NULL )
        {
            continue;
        }

        while( dirent = readdir(dir), dirent )
        {

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

            // switch display or crossfade
            for ( uint16_t step = 0 ; step <= 256; step+=8 )
            {
                fade( curimg, nextimg, tft_buf, step, TFT_BUFSIZE );

                vga_tft_set_screen( tft, (uintptr_t)tft_buf );          // show calculated buffer
                tft_buf = tft_buf == tft_buf0 ? tft_buf1 : tft_buf0;    // switch to other buffer for next calculation
            }

            curimg = (curimg == image1) ? image0 : image1;
            nextimg = (nextimg == image1) ? image0 : image1;
        }
        closedir(dir);
    }
}



