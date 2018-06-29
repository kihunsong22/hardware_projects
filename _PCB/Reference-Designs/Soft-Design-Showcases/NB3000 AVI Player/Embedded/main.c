/*****************************************************************************\
|*
|*  IN PACKAGE:         Software Platform Builder
|*
|*  COPYRIGHT:          Copyright (c) 2010, Altium
|*
|*  DESCRIPTION:        Video player demo: plays RAW RGB16 AVI files from SD card
|*
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "swplatform.h"
#include "form1.h"
#include "aviparse.h"

/* configuration */
#define STORAGEDEV            "/dev/SDCARD"
#define MOUNTPOINT            "/sdcard"

#define VGA_XRES    320
#define VGA_YRES    240
#define VGA_PIXELS (VGA_XRES * VGA_YRES)

// file descriptor for AVI file
static int avi_fd = -1;

// AVI video & audio specs
static AVIFORMAT avi_format;

// current location in audio and video stream
static int32_t avi_framepos = 0;
static int32_t avi_framesdropped = 0;
static int64_t avi_videomicrosec = 0;
static int32_t avi_samples_buffered = 0;
static int32_t avi_samples_syncmargin = 0;

// two canvasses for double buffering while playing video
graphics_t *graphics;
canvas_t   *canvas0;
canvas_t   *canvas1;

// avi decoding buffer (holds 1 AVI frame full VGA 24 bit)
// buffer in external memory so the ASP can reach it
#pragma section SRAM
uint32_t avibuf[VGA_PIXELS * 3 / sizeof(int32_t)];
#pragma endsection

static void init(void);

int avi_setup(void);
int avi_read(uint32_t *buf, int size);
int avi_videoprocess(uint32_t **bufp, int size);
void videocopy(uint32_t *buf, uint32_t *vbuf, int width, int height, int mode);
int avi_audioprocess(uint32_t **bufp, int size);

bool avi_testsupported(void);
void avi_playfile(char *filename);

clock_t sync_nextframe;
clock_t sync_frametime;

/*
 * Main program
 */
int main( void )
{
    printf( "\n*** SDCARD AVI Player ***\n\n" );

    init();

    dirlisting(MOUNTPOINT);

    while (1)
    {
        agui_service(agui);

        graphics_set_visible_canvas(graphics, canvas0);
        if (gui_playfile)
        {
            printf( "Playing file \"%s\"\n", gui_playfile );
            cursor_hide(agui);
            agui_show_form(AGUI_HANDLE(form2));

            graphics_fill_canvas(canvas1, GRAY20);

            avi_playfile(gui_playfile);
            gui_playfile = NULL;

            agui_hide_form(agui);
            cursor_show(agui);
            graphics_set_visible_canvas(graphics, canvas0);
        }
    }
}

static void init_sdhc(void)
{
    sdhc_t *sdcard = sdhc_open(SDHC_DRIVER);
    int err;
    const char *str;
    int x, y;

    for (;;)
    {
        graphics_fill_canvas(canvas0, BLACK);
        x = 40;
        y = 40;

        str = "Please insert SD card...";
        graphics_draw_string(canvas0, x, y, str, NULL, WHITE, FS_NONE);
        puts( str );

        while( !sdhc_card_detect( sdcard ) )   // Wait for card to be inserted
            ;

        str = "Card detected...";
        y += 16;
        graphics_draw_string(canvas0, x, y, str, NULL, WHITE, FS_NONE);
        puts( str );

        for ( int i = 0; i < 10; i++ )
        {
            err = sdhc_card_init( sdcard, SDHC_INIT_POWERON );
            if ( sdhc_is_memcard(err) ) break;
        }

        if ( !sdhc_is_memcard(err) )
        {
            puts( "Card init failed" );
            continue;
        }
        else
        {
            str = "Card init OK";
            y += 16;
            graphics_draw_string(canvas0, x, y, str, NULL, WHITE, FS_NONE);
            puts( str );
        }

        /* Try to mount partition #1 */
        err = mount(STORAGEDEV, MOUNTPOINT, "fatfs", 1, MOUNT_FLAG_RDONLY);
        if (err != 0)
        {
            /* ... and if that fails try the entire disk */
            err = mount(STORAGEDEV, MOUNTPOINT, "fatfs", 0, MOUNT_FLAG_RDONLY);
        }
        if ( err != 0 )
        {
            puts( "Mount failed, please remove card!" );
        }
        else
        {
            // Success!
            puts( "Filesystem mounted at \"" MOUNTPOINT "\"" );
            break;
        }

        while( sdhc_card_detect( sdcard ) )
            ;
        puts( "Card removed" );
    }
}

static void init(void)
{
    // generated initialization code
    swplatform_init_stacks();

    // low-level stack access
    graphics = graphics_open(GRAPHICS);
    canvas0 = graphics_get_canvas(graphics, 0);
    canvas1 = graphics_get_canvas(graphics, 1);

    // mount sdcard
    init_sdhc();

//    graphics_set_visible_canvas(canvas0);

    agui_show_form(AGUI_HANDLE(form1));
    cursor_show(agui);
}


/**********************************************************************
|*
|*  FUNCTION    : avi_parseheader
|*
|*  PARAMETERS  : filename = name of file from current directory to parse
|*                buf = buffer to put human description of AVI details
|*
|*  RETURNS     : true if supported
|*
|*  DESCRIPTION : parse header for AVI info and show it in the gui
 */
bool avi_parseheader(char *filename, char *info)
{
    int retcode;
    int fps;

    *info = 0;

    avi_fd = open(filename, O_RDONLY);

    retcode = avi_parse(&avi_format, NULL, 0, &avi_read, NULL, NULL, NULL);

    close(avi_fd);

    if (retcode) return false;

    // put video/audio specs details in string buffer
    fps = 10000000 / avi_format.video.microsecperframe;

    sprintf(info, "%ix%i %i.%i %i/%i/%ik",
            avi_format.video.width, avi_format.video.height, fps / 10, fps % 10,
            avi_format.audio.channels, avi_format.audio.bitspersample, avi_format.audio.samplespersec / 1000);

    return avi_testsupported();
}


/**********************************************************************
|*
|*  FUNCTION    : avi_playfile
|*
|*  PARAMETERS  : filename = name of file from current directory to play
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : play given file from current directory on TFT screen and audio output
 */
void avi_playfile(char *filename)
{
    clock_t starttime;
    int duration;
    int retcode;

    printf("playing %s\n", filename);

    avi_fd = open(filename, O_RDONLY);

    starttime = (int) (clock() / (CLOCKS_PER_SEC / 1000));

    retcode = avi_parse(&avi_format, avibuf, sizeof(avibuf), &avi_read, &avi_setup, &avi_videoprocess, &avi_audioprocess);

    duration = ((clock() / (CLOCKS_PER_SEC / 1000L)) - starttime) & 0x7FFFFFFF;

    printf("finished playing AVI file: %i ms, %i frames (%i dropped), %i ms/frame, %i.%1i frame/sec\n\n",
          duration, avi_framepos, avi_framesdropped, duration / avi_framepos,
          1000 * avi_framepos / duration, 10000 * avi_framepos / duration % 10);

    if (retcode < 0)
    {
        printf("error while parsing AVI file - aborted\n");
    }
    else if (retcode > 0)
    {
        printf("aborted by user\n");
    }
    printf("ready\n\n");

    close(avi_fd);
}



// callback function for avi_parse to read data
int avi_read(uint32_t *buf, int size)
{
    if (buf)
    { read(avi_fd, buf, size); }
    else
    { lseek(avi_fd, size, SEEK_CUR); }

    return 0;
}

/**********************************************************************
|*
|*  FUNCTION    : avi_testsupported
|*
|*  PARAMETERS  : none
|*
|*  RETURNS     : true if supported
|*
|*  DESCRIPTION : check audio/video formats if we support it
 */
bool avi_testsupported(void)
{
    if ((avi_format.video.width > 320) || (avi_format.video.height > 240))
    {
        printf("video size too big for this player\n");
        return false;
    }

    if (avi_format.video.mode == VIDEOMODE_RGB888)
    {
        printf("video bits per pixel too high for this player\n");
        return false;
    }

    if (avi_format.audio.samplespersec > 22050)
    {
        printf("audio samplerate too high for this player\n");
        return false;
    }

    if ((avi_format.audio.channels != 1) && (avi_format.audio.channels != 2))
    {
        printf("number of audio channels unsupported by this player\n");
        return false;
    }

    if ((avi_format.audio.bitspersample != 16) && (avi_format.audio.bitspersample != 8))
    {
        printf("unsupported audio format unsupported by this player\n");
        return false;
    }

    return true;
}


/**********************************************************************
|*
|*  FUNCTION    : avi_setup
|*
|*  PARAMETERS  : none
|*
|*  RETURNS     : 0 if succes, <> 0 if error
|*
|*  DESCRIPTION : callback function from avi_parse, sets up audio/video formats
 */
int avi_setup(void)
{
    if (!avi_testsupported())
    { return -1; }

    avi_framepos = 0;
    avi_framesdropped = 0;
    avi_videomicrosec = 0;
    avi_samples_buffered = 0;

    // set up A/V sync
    sync_nextframe = 0;
    sync_frametime = avi_format.video.microsecperframe * (CLOCKS_PER_SEC / 1000000L);
    clock_t starttime = clock();

    avi_samples_syncmargin = avi_format.audio.samplespersec * avi_format.video.microsecperframe / 1000000L / 4;

    audio_set_format(audio, avi_format.audio.samplespersec, avi_format.audio.channels, avi_format.audio.bitspersample);

    return 0;
}


/**********************************************************************
|*
|*  FUNCTION    : avi_videoprocess
|*
|*  PARAMETERS  : buf = buffer with video data
|*                size = size of data in bytes
|*
|*  RETURNS     : 0 if succes, <> 0 if error
|*
|*  DESCRIPTION : callback function from avi_parse, processes a single video frame
|*
|*                this supports variable size (max 320x240), bottom-up , in the formats:
|*                1) little-endian 16-bit RGB-5:5:5 encoding (uncompressed 32K color)
|*                2) 24 bit RGB encoding (uncompressed true color)
|*                (our own VGA core is 240x320 top-down big-endian 16-bit RGB-5:5:5)
 */
int avi_videoprocess(uint32_t **bufp, int size)
{
    uint32_t *buf = *bufp;
    canvas_t *canvas;

    // check for user abort
    agui_service(agui);
    if (!gui_playfile) return 1;

    ++avi_framepos;

    if (sync_nextframe == 0)
    {
        sync_nextframe = clock() + sync_frametime;
    }
    else if (clock() > sync_nextframe)
    {
//        printf("frame dropped\n");
        ++avi_framesdropped;
        sync_nextframe = clock() + sync_frametime;
        return 0;
    }
    else
    {
        while (clock() < sync_nextframe) /* empty loop */;
        sync_nextframe += sync_frametime;
    }

    // we don't verify the size, if too small harmless garbage will be shown

    while (!graphics_visible_canvas_is_set(graphics)) /* empty loop */;

    canvas = (graphics_get_visible_canvas(graphics) == canvas0) ? canvas1 : canvas0;
    videocopy(buf, (uint32_t*) canvas_get_buffer(canvas), avi_format.video.width, avi_format.video.height, avi_format.video.mode);
    graphics_set_visible_canvas(graphics, canvas);

    return 0;
}

// this function is accelerated by pushing it into hardware
void videocopy(uint32_t *buf, uint32_t *vbuf, int width, int height, int mode)
{
    int width_words = width / 2;
    uint32_t *pfromrow = buf;
    uint32_t *pcolto   = vbuf;
    const int vwidth = VGA_XRES / 2;
    const int vheight = VGA_YRES;
    width  /= 2;

    for (int yfrom = height; yfrom; yfrom--)
    {
        for (int xfrom = 0; xfrom < width; xfrom++)
        {
            uint32_t word = buf[yfrom*vwidth+xfrom];

            // select pixel and correct endianness
#ifdef __LITTLE_ENDIAN__
            uint32_t pixel_11 = word >> 16;
            uint32_t pixel_12 = word & 0x0000FFFF;
#else
            uint32_t pixel_11 = ((word & 0xFF000000) >> 24) | ((word & 0x00FF0000) >> 8);
            uint32_t pixel_12 = ((word & 0x0000FF00) >> 8) | ((word & 0x000000FF) << 8);
#endif
            if (mode == VIDEOMODE_RGB555)
            {
                pixel_11 = ((pixel_11 & 0x7FE0) << 1) | (pixel_11 & 0x001F);
                pixel_12 = ((pixel_12 & 0x7FE0) << 1) | (pixel_12 & 0x001F);
            }

            word = (pixel_11 << 16) | pixel_12;
            vbuf[(vheight - yfrom)*vwidth+xfrom] = word;
        }
    }
    return;
}


/**********************************************************************
|*
|*  FUNCTION    : avi_audioprocess
|*
|*  PARAMETERS  : buf = buffer with audio data
|*                size = size of data in bytes
|*
|*  RETURNS     : 0 if succes, <> 0 if error
|*
|*  DESCRIPTION : callback function from avi_parse, processes a block audio
|*
|*                this supports 16 & 8 bits, mono and stereo
 */
int avi_audioprocess(uint32_t **bufp, int size)
{
    uint32_t *buf = *bufp;
//printf("audio %i\n", size);
//return 0;

    switch (avi_format.audio.bitspersample)
    {
    case 8:
        {
            uint8_t *buf8 = (uint8_t *) buf;

            for (int i = 0; i < size; ++i)
            {
                buf8[i] = (((int)buf8[i]) - 0x80) & 0xFF;
            }

            while (size)
            {
                int done = audio_play(audio, buf8, size);
                buf8 += done;
                size -= done;
            }
        }

        break;

    case 16:
        size /= 2;

        {
            uint16_t *buf16 = (uint16_t *) buf;

#ifdef __BIG_ENDIAN__
            for (int i = 0; i < size; ++i)
            {
                uint32_t val = buf16[i];
                ((int16_t*) buf16)[i] = ((val << 8) | (val >> 8));
            }
#endif

            while (size)
            {
                int done = audio_play(audio, buf16, size);
                buf16 += done;
                size -= done;
                if (size) printf("%i bytes did not fit in audio buffer\n", size);
            }
        }

        break;

    }

    return 0;
}

