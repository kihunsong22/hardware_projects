/*****************************************************************************\
|*
|*  IN PACKAGE:         Software Platform Builder
|*
|*  COPYRIGHT:          Copyright (c) 2009, Altium
|*
|*  DESCRIPTION:        Video player demo: plays RAW RGB16 AVI files from CF-card
|*
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include <fs.h>

#include "devices.h"
#include <audio.h>
#include <agui.h>
#include <graphics.h>

#include "form1.h"
#include "aviparse.h"

/* configuration */
#define SRC_DIR             "/cfcard"

#define VGA_XRES    240
#define VGA_YRES    320
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

// handles for Software platform driver instances
audio_t *audio;
graphics_t *graphics;
agui_t *agui;

// two canvasses for double buffering while playing video
canvas_t *canvas0;
canvas_t *canvas1;

// avi decoding buffer (holds 1 AVI frame full VGA 24 bit)
// buffer in external memory so the ASP can reach it
#pragma section xram
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
    printf( "\n*** IDE AVI Player ***\n\n" );

    init();

    dirlisting("/cfcard");

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


static void init(void)
{
    if (mount("/dev/BLOCKIO_1", "/cfcard", "fatfs", 1, MOUNT_FLAG_RDONLY))
    {
        printf("ERROR - mounting CF card failed\n");
    }

    audio = audio_open(AUDIO_1);

    graphics = graphics_open(GRAPHICS_1);

    canvas0 = graphics_get_canvas(graphics, 0);
    canvas1 = graphics_get_canvas(graphics, 1);
//    graphics_set_visible_canvas(canvas0);

    agui = agui_open(AGUI_1);

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
    uint32_t *pcolto = vbuf;

    // center movie
    pcolto += (VGA_XRES - height) / 4;
    //pcolto += (VGA_XRES * (VGA_YRES - avi_format.video.width))

    // convert two lines into two colums at the same time so we can always read/write 32-bits
    for (int yfrom = height / 2; yfrom; --yfrom)
    {
        uint32_t* pto = pcolto++;
        uint32_t *pfromrow1 = pfromrow;
        pfromrow += width_words;
        uint32_t *pfromrow2 = pfromrow;
        pfromrow += width_words;

        for (int xfrom = width_words; xfrom; --xfrom)
        {
            // get 2x2 pixels from the AVI frame
            uint32_t readset1 = *pfromrow1++;
            uint32_t readset2 = *pfromrow2++;

            // select pixel and correct endianness
#ifdef __LITTLE_ENDIAN__
            uint32_t pixel_11 = (readset1 & 0xFFFF0000) >> 16;
            uint32_t pixel_12 = readset1 & 0x0000FFFF;
            uint32_t pixel_21 = (readset2 & 0xFFFF0000) >> 16;
            uint32_t pixel_22 = readset2 & 0x0000FFFF;
#else
            uint32_t pixel_11 = ((readset1 & 0x0000FF00) >> 8) | ((readset1 & 0x000000FF) << 8);
            uint32_t pixel_12 = ((readset1 & 0xFF000000) >> 24) | ((readset1 & 0x00FF0000) >> 8);
            uint32_t pixel_21 = ((readset2 & 0x0000FF00) >> 8) | ((readset2 & 0x000000FF) << 8);
            uint32_t pixel_22 = ((readset2 & 0xFF000000) >> 24) | ((readset2 & 0x00FF0000) >> 8);
#endif
            if (mode == VIDEOMODE_RGB555)
            {
                pixel_11 = ((pixel_11 & 0x7FE0) << 1) | (pixel_11 & 0x001F);
                pixel_12 = ((pixel_12 & 0x7FE0) << 1) | (pixel_12 & 0x001F);
                pixel_21 = ((pixel_21 & 0x7FE0) << 1) | (pixel_21 & 0x001F);
                pixel_22 = ((pixel_22 & 0x7FE0) << 1) | (pixel_22 & 0x001F);
            }

            // mirror the 2x2 pixels over the diagonal (\) line
            uint32_t writeset1 = (pixel_12 << 16) | pixel_22;
            uint32_t writeset2 = (pixel_11 << 16) | pixel_21;

            // store in VGA memory
            *pto = writeset1;
            pto += (VGA_XRES / 2);
            *pto = writeset2;
            pto += (VGA_XRES / 2);
        }

        pfromrow1 += width_words;
        pfromrow2 += width_words;
    }
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



