/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Version$   $Date$
|*
|*  IN PACKAGE:         FFT
|*
|*  COPYRIGHT:          Copyright (c) 2006, Altium
|*
|*  DESCRIPTION:        Main module
 */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <graphics.h>
#include <audio.h>
#include <devices.h>

#include "fft.h"

/* FFT/audio defines */
#define FFT_POINTS          1024
#define AUDIO_BUF_SIZE      (2 * FFT_POINTS)

/* display defines */
#define LEVELS 255
#define LEVEL_FACTOR ((255.0/log10(LEVELS))+0.5)
#define FREQ_BANDS 100
#define HISTO_WIDTH 200
#define HISTO_BASE  270
#define FREQ_BAND_WIDTH (HISTO_WIDTH/FREQ_BANDS)

#define DECAY_COLOR GOLD
#define LEVEL_COLOR ORANGERED
#define PEAK_COLOR  WHITE
#define BACKGROUND_COLOR    GRAY20

#define VU_LEFT      80
#define VU_LEVEL1    (VU_LEFT + 100)
#define VU_LEVEL2    (VU_LEFT + 115)
#define VU_LEVEL3    (VU_LEFT + 128)


/* bufffers */
// audio
int16_t audio_buf[AUDIO_BUF_SIZE] = {0};
audio_t *audio;
//fft
float fft_in[FFT_POINTS * 2];
complex_t fft_out[FFT_POINTS];
float sqrt_table[256 * 256];
//video
graphics_t *graphics;
canvas_t *canvas;
uint16_t freq_table[FREQ_BANDS];
uint8_t readout_table[LEVELS];


/* function prototypes */
void init_processor(void);
void init_audio(void);
void init_video(void);
int get_audio(int16_t *buffer, int size);
int put_audio(int16_t *buffer, int n);
void vu_meter(int16_t *buffer, int n);
void display_fft(complex_t *fft, int n);
void fill_fft(float *fft, int16_t *audio, int n, int state);
void fftr_opt(fft_cfg cfg, float *fin, complex_t *fout, int state);
void init_tables(void);
void init_screen(void);



/**********************************************************************
|*
|*  FUNCTION    : main
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Start here
 */

void main(void)
{
    fft_cfg fft_cfg;
    int state = 0;
    int x = 0;
    int y = 0;

    /* init */
    graphics = graphics_open(GRAPHICS_1);
    canvas = graphics_get_visible_canvas(graphics);
    graphics_draw_string(canvas, 75, 120, "INITIALIZING", NULL, WHITE, FS_NONE);
    init_tables();
    audio = audio_open(AUDIO_1);
    fft_cfg = fft_alloc(FFT_POINTS, 0, 0, 0);
    init_screen();

    while (1)
    {
        get_audio(audio_buf, AUDIO_BUF_SIZE);
        put_audio(audio_buf, AUDIO_BUF_SIZE);

        /* draw vu meter */
        vu_meter(audio_buf, AUDIO_BUF_SIZE);

        /* draw fft spectrum */
        if (state == 0)
        {
            fill_fft(fft_in, audio_buf, AUDIO_BUF_SIZE, 0);
            state = 1;
        }
        else if (state == 1)
        {
            fill_fft(fft_in, audio_buf, AUDIO_BUF_SIZE, 1);
            fftr_opt(fft_cfg, fft_in, fft_out, 1);
            display_fft(fft_out, FFT_POINTS / 2);
            state = 2;
        }
        else
        {
            fill_fft(fft_in, audio_buf, AUDIO_BUF_SIZE, 0);
            fftr_opt(fft_cfg, fft_in, fft_out, 0);
            display_fft(fft_out, FFT_POINTS / 2);
            state = 1;
        }
    }
}


/**********************************************************************
|*
|*  FUNCTION    : get_audio
|*
|*  PARAMETERS  : buffer = pointer to audio buffer
|*                n = number of samples to read
|*
|*  RETURNS     : Number of samples actually received in buffer
|*
|*  DESCRIPTION : Receive buffer from audio-input
 */

int get_audio(int16_t *buffer, int n)
{
    int s;

    do
    {
        s = audio_record(audio, buffer, n);
        n -= s;
        buffer += s;
    }while (n != 0);

    return s;
}


/**********************************************************************
|*
|*  FUNCTION    : put_audio
|*
|*  PARAMETERS  : buffer = pointer to audio stream
|*                n = Number of samples to write
|*
|*  RETURNS     : Number of samples actually transmitted
|*
|*  DESCRIPTION : Send buffer to audio-output
 */

int put_audio(int16_t *buffer, int n)
{
    int s;

    do
    {
        s = audio_play(audio, buffer, n);
        n -= s;
        buffer += s;
    } while (n != 0);

    return 0;
}



/**********************************************************************
|*
|*  FUNCTION    : init_vu_meter
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Init vu-meter graphics
 */

void init_screen(void)
{
    graphics_fill_canvas(canvas, DARKSLATEGRAY);
    graphics_draw_string(canvas, 30, 277, "LEFT", NULL, WHITE, FS_NONE);
    graphics_draw_string(canvas, 30, 297, "RIGHT", NULL, WHITE, FS_NONE);
    graphics_fill_rect(canvas,  VU_LEFT, 280, 100, 11, GREEN);
    graphics_fill_rect(canvas, VU_LEVEL1, 280,  15, 11, OLIVE);
    graphics_fill_rect(canvas, VU_LEVEL2, 280,  13, 11, MAROON);
    graphics_fill_rect(canvas,  VU_LEFT, 300, 100, 11, GREEN);
    graphics_fill_rect(canvas, VU_LEVEL1, 300,  15, 11, OLIVE);
    graphics_fill_rect(canvas, VU_LEVEL2, 300,  13, 11, MAROON);

    graphics_fill_rect(canvas, 120 - HISTO_WIDTH/2, HISTO_BASE-255, HISTO_WIDTH - 1, 255, BACKGROUND_COLOR);
}

/**********************************************************************
|*
|*  FUNCTION    : vu_meter
|*
|*  PARAMETERS  : buffer = pointer to audio buffer
|*                n = number of samples in audio buffer
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Display VU meter with peak indication
 */

void vu_meter(int16_t *buffer, int n)
{
    int16_t left = 0;
    int16_t right = 0;
    static uint8_t last_left = VU_LEFT;
    static uint8_t last_right = VU_LEFT;
    static int16_t left_peak = VU_LEFT;
    static int16_t right_peak = VU_LEFT;
    int16_t last_left_peak;
    int16_t last_right_peak;
    static int16_t left_delay = VU_LEVEL3;
    static int16_t right_delay = VU_LEVEL3;
    int i;
    int x, y;
    static int vu_levels[] = {VU_LEFT, VU_LEVEL1, VU_LEVEL2, VU_LEVEL3};
    static color_t vu_colors_on[] = {LIME, YELLOW, RED};
    static color_t vu_colors_off[] = {GREEN, OLIVE, MAROON};

    /* calculate left channel */
    for (i = 0; i < n; i += 2)
    {
        left = buffer[i] > left ? buffer[i] : left;
    }
    left /= 256;
    left += VU_LEFT;     // x offset onscreen
    last_left_peak = left_peak;
    left_peak -= left_delay >> 2;
    left_delay++;
    if (left > left_peak)
    {
        left_peak = left;
        left_delay = 0;
    }
    /* calculate right channel */
    for (i = 1; i < n; i += 2)
    {
        right = buffer[i] > right ? buffer[i] : right;
    }
    right /= 256;
    right += VU_LEFT;    // x offset on screen
    last_right_peak = right_peak;
    right_peak -= right_delay >> 2;
    right_delay++;
    if (right > right_peak)
    {
        right_peak = right;
        right_delay = 0;
    }

    /* clear previous left peak, draw new left peak */
    i = 0;
    while (last_left_peak > vu_levels[i]) {i++;}
    graphics_draw_line(canvas, last_left_peak, 280, last_left_peak, 290, vu_colors_off[i - 1]);
    graphics_draw_line(canvas, left_peak, 280, left_peak, 290, WHITE);

    /* draw left vu */
    if (last_left > left)
    {
        for (i = 0; i < 3; i++)
        {
            x = left > vu_levels[i] ? left : vu_levels[i];
            y = last_left > vu_levels[i + 1] ? vu_levels[i + 1] : last_left;
            graphics_fill_rect(canvas, x, 280, y - x, 11, vu_colors_off[i]);
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            x = last_left > vu_levels[i] ? last_left : vu_levels[i];
            y = left > vu_levels[i + 1] ? vu_levels[i + 1] : left;
            graphics_fill_rect(canvas, x, 280, y - x, 11, vu_colors_on[i]);
        }
    }

    /* clear previous right peak, draw new right peak */
    i = 0;
    while (last_right_peak > vu_levels[i]) {i++;}
    graphics_draw_line(canvas, last_right_peak, 300, last_right_peak, 310, vu_colors_off[i - 1]);
    graphics_draw_line(canvas, right_peak, 300, right_peak, 310, WHITE);

    /* draw left vu */
    if (last_right > right)
    {
        for (i = 0; i < 3; i++)
        {
            x = right > vu_levels[i] ? right : vu_levels[i];
            y = last_right > vu_levels[i + 1] ? vu_levels[i + 1] : last_right;
            graphics_fill_rect(canvas, x, 300, y - x, 11, vu_colors_off[i]);
        }
    }
    else
    {
        for (i = 0; i < 3; i++)
        {
            x = last_right > vu_levels[i] ? last_right : vu_levels[i];
            y = right > vu_levels[i + 1] ? vu_levels[i + 1] : right;
            graphics_fill_rect(canvas, x, 300, y - x, 11, vu_colors_on[i]);
        }
    }

    last_left = (uint8_t)left;
    last_right = (uint8_t)right;
}


/**********************************************************************
|*
|*  FUNCTION    : fftr_opt
|*
|*  PARAMETERS  : cfg = FFT configuration data
|*                fft_in = pointer to audio-samples converted to complex notation (r-part = FFT1, i-part = FFT2)
|*                fft_out = pointer to buffer with FFT-results
|*                state = 0, calculate 2 fft's and return first fft result,
|*                        1, return second fft result
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Calculate fft
|*
 */

void fftr_opt(fft_cfg cfg, float *fft_in, complex_t *fft_out, int state)
{
    static complex_t buf[FFT_POINTS];

    if (state == 0)
    {
        fft(cfg, (complex_t*)fft_in, buf);

        fft_out[0].r = buf[0].r;
        fft_out[0].i = 0;
        for (int i = 1; i < (cfg->nfft / 2); i++)
        {
            fft_out[i].r = 0.5 * (buf[i].r + buf[cfg->nfft - i].r);
            fft_out[i].i = 0.5 * (buf[i].i - buf[cfg->nfft - i].i);
        }
    }
    else
    {
        fft_out[cfg->nfft / 2].r = buf[cfg->nfft / 2].r;
        fft_out[cfg->nfft / 2].i = 0;
        for (int i = 1; i < (cfg->nfft / 2); i++)
        {
            fft_out[i].r = 0.5 * (buf[i].i + buf[cfg->nfft - i].i);
            fft_out[i].i = 0.5 * (buf[i].r - buf[cfg->nfft - i].r);
        }
    }
}


/**********************************************************************
|*
|*  FUNCTION    : display_fft
|*
|*  PARAMETERS  : fft = pointer to buffer with FFT results
|*                n = number of FFT points
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Display the calculated FFT
 */

void display_fft(complex_t *fft, int n)
{
    int i, x;
    int k = 0, l = 0;
    int last_level, last_peak, new_level;
    static int level[FREQ_BANDS] = {0};
    static int peak[FREQ_BANDS] = {0};
    float v = 0.0;
    int sqr_sum;
    static uint8_t decay[FREQ_BANDS];

    for (i = 0; i < n; i++)
    {
        sqr_sum = (int)((1e-9/FREQ_BAND_WIDTH) * (fft->r * fft->r + fft->i * fft->i));
        if (sqr_sum < 65535)
        {
            if (sqrt_table[sqr_sum] > v)
            {
                v = sqrt_table[sqr_sum];
            }
        }
        else
        {
            v = 256.0;
        }
        l++;
        while (i > freq_table[k])
        {
            last_level = level[k];
            last_peak = peak[k];
            peak[k] -= decay[k];
            if (decay[k] < 3)
            {
                decay[k]++;
            }
            new_level = (int)v;
            if (new_level > LEVELS-1)
            {
                new_level = LEVELS-1;
            }
            new_level = readout_table[new_level];
            level[k] = new_level;
            if (peak[k] < new_level)
            {
                peak[k] = new_level;
                decay[k] = 0;
            }
            for (x = k * FREQ_BAND_WIDTH; x < (k + 1) * FREQ_BAND_WIDTH; x++)
            {
                if (last_peak > peak[k])
                {
                    /* clear last peak */
                    graphics_draw_line(canvas, x + 120 - HISTO_WIDTH/2, HISTO_BASE - last_peak, x + 120 - HISTO_WIDTH/2, HISTO_BASE - peak[k], BACKGROUND_COLOR);
                }
                if (last_level > new_level)
                {
                    /* clear from last value to new value */
                    last_level = last_level < peak[k] ? last_level : peak[k];
                    graphics_draw_line(canvas, x + 120 - HISTO_WIDTH/2, HISTO_BASE - last_level, x + 120 - HISTO_WIDTH/2, HISTO_BASE - new_level, DECAY_COLOR);
                }
                else
                {
                    /* draw from last value to new value */
                    graphics_draw_line(canvas, x + 120 - HISTO_WIDTH/2, HISTO_BASE - last_level, x + 120 - HISTO_WIDTH/2, HISTO_BASE - new_level, LEVEL_COLOR);
                }
                graphics_draw_pixel(canvas, x + 120 - HISTO_WIDTH/2, HISTO_BASE - peak[k], PEAK_COLOR);
            }
            k++;

            if (i <= freq_table[k])
            {
                v = 0;
                l = 0;
            }
        }
        fft++;
    }
}


/**********************************************************************
|*
|*  FUNCTION    : fill_fft
|*
|*  PARAMETERS  : fft = pointer to input buffer for FFT transformation
|*                audio = pointer to audio buffer
|*                state = 0, load audio in r-part, (= first FFT)
|*                        1, load audio in i-part  (= second FFT)
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Fill FFT input buffer
 */

void fill_fft(float *fft, int16_t *audio, int n, int state)
{
    int i;

    if (state == 0)
    {
        for (i = 0; i < n; i += 2)
        {
            fft[i] = (float)((int)audio[i] + (int)audio[i + 1]);                   // inputdata of first fft loaded in r-part
        }
    }
    else
    {
        for (i = 0; i < n; i += 2)
        {
            fft[i + 1] = (float)((int)audio[i] + (int)audio[i + 1]);               // inputdata of second fft loaded in i-part
        }
    }

}


/**********************************************************************
|*
|*  FUNCTION    : init_tables
|*
|*  PARAMETERS  : None
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Init the tables used by the display routines
 */

void init_tables(void)
{
    int i;

    /* fill square root table */
    for (i = 0; i < (256 * 256); i++)
    {
        sqrt_table[i] = sqrtf((float)(i));
    }
    sqrt_table[0] = 0;
    sqrt_table[1] = 0;

    /* fill freq_band table */
    for (i = 0; i < FREQ_BANDS; i++)
    {
        freq_table[i] = floor(0.5 + pow(pow(FFT_POINTS / 2, 1.0 / (FREQ_BANDS)), (float)(i + 1)));   // store end freq of each freq_band
    }

    for (i = 1; i < LEVELS; i++)
    {
        readout_table[i] = (int)(floor(log10((float)(i+1)) * LEVEL_FACTOR)) - (int)(LEVEL_FACTOR * log10(2));
    }
    readout_table[0] = 0;
}

