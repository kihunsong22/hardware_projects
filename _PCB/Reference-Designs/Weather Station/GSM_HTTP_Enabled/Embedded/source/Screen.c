/*************************************************************************
**
**  VERSION CONTROL:    %W%	%E%
**
**  IN PACKAGE:     	Weatherstation Demo
**
**  COPYRIGHT:      	Copyright (c) 2003 Altium
**
**  DESCRIPTION:   	IP-over-RS232 enabled weather station
**                  	VGA support routines
**
**************************************************************************/


#include "tealib\timer0.h"
#include "tealib\vga.h"
#include "bmp\bitmap.h"
#include "screen.h"
#include "main.h"

static int temp_array[ARRAY_SIZE];
static int speed_array[ARRAY_SIZE];
static int dir_array[ARRAY_SIZE];

static void update_temp_hist(void);
static void update_speed_hist(void);
static void update_dir_hist(void);
static void screen_temp_hist(void);
static void screen_speed_hist(void);
static void screen_dir_hist(void);
static void screen_temp_val(void);
static void screen_speed_val(void);
static void screen_dir_val(void);
unsigned int screen_getswidth(__rom char * s);
unsigned int screen_getvalwidth(long val, char prec);

/**********************************************************************
|*
|*  FUNCTION    : screen_init
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Initialize the screen
*/

__rom unsigned char * x_axis[] =
{
    bmp_24h, bmp_22h, bmp_20h, bmp_18h, bmp_16h, bmp_14h, bmp_12h, bmp_10h, bmp_8h, bmp_6h, bmp_4h, bmp_2h, bmp_0h
}

;
__rom unsigned char * temp_y_axis[] =
{
    bmp_minus20, bmp_minus10, bmp_0, bmp_10, bmp_20, bmp_30, bmp_40, bmp_50
}

;
__rom unsigned char * speed_y_axis[] =
{
    bmp_0, bmp_10, bmp_20, bmp_30, bmp_40, bmp_50
}

;
__rom unsigned char * dir_y_axis[] =
{
    bmp_north, bmp_east, bmp_south, bmp_west, bmp_north
}

;
unsigned char chart_type = FILL;

void screen_init(void)
{
    /* clear VGA */
    vga_init();

    /* clear VGA */
    vga_init();

    /* show altium bitmap */
    vga_bitmap_16c(L_POS_X2 - vga_getbmwidth(bmp_altium16), 0, bmp_altium16);

    /* init temperature history */
    screen_temp_hist_init();

    /* draw value */
    vga_bitmap_2c(T_VAL_POS_X2 - vga_getbmwidth(bmp_temp_c), T_LBL_POS_Y, bmp_temp_c, T_FG1_COLOR, black);
    screen_temp_val();

    screen_speed_hist_init();

    /* draw value */
    vga_bitmap_2c(S_VAL_POS_X2 - vga_getbmwidth(bmp_speed), S_LBL_POS_Y, bmp_speed, S_FG1_COLOR, black);
    screen_speed_val();

    screen_dir_hist_init();

    /* draw value */
    vga_bitmap_2c(D_VAL_POS_X2 - vga_getbmwidth(bmp_direction), D_LBL_POS_Y, bmp_direction, D_FG1_COLOR, black);
    screen_dir_val();
}

void screen_temp_hist_init(void)
{
    char i;

    /* draw labels for temp-history y_axis */
    for (i = 0; i < 8; i++)
    {
        vga_bitmap_2c(T_HIST_POS_X - 18, T_HIST_POS_Y - 4 - ((i - 2) * 10 * T_HIST_MULT), temp_y_axis[i], T_BG_COLOR, black);
    }

    /* draw background for Temperature History */
    vga_fill(T_HIST_POS_X, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X + ARRAY_SIZE, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_BG_COLOR);
    vga_line(T_HIST_POS_X + ARRAY_SIZE, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X + ARRAY_SIZE, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_LINE_COLOR);
    vga_line(T_HIST_POS_X, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_LINE_COLOR);

    /* draw labels for temp-history x_axis */
    for (i = 0; i < 13; i++)
    {
        vga_bitmap_2c(T_HIST_POS_X - (vga_getbmwidth(x_axis[i]) >> 1) + (i * ARRAY_SIZE / 12), T_HIST_POS_Y + 6 - (- 20 * T_HIST_MULT), x_axis[i], T_BG_COLOR, black);
    }

    /* draw history */
    screen_temp_hist();
}

void screen_speed_hist_init(void)
{
    char i;

    /* draw labels for speed-history y_axis */
    for (i = 0; i < 6; i++)
    {
        vga_bitmap_2c(S_HIST_POS_X - 18, S_HIST_POS_Y - 4 - (i * 10 * S_HIST_MULT), speed_y_axis[i], S_BG_COLOR, black);
    }

    /* draw background for Wind Speed History */
    vga_fill(S_HIST_POS_X, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X + ARRAY_SIZE, S_HIST_POS_Y, S_BG_COLOR);
    vga_line(S_HIST_POS_X + ARRAY_SIZE, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X + ARRAY_SIZE, S_HIST_POS_Y, S_LINE_COLOR);
    vga_line(S_HIST_POS_X, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X, S_HIST_POS_Y, S_LINE_COLOR);

    /* draw labels for speed-history x_axis */
    for (i = 0; i < 13; i++)
    {
        vga_bitmap_2c(S_HIST_POS_X - (vga_getbmwidth(x_axis[i]) >> 1) + (i * ARRAY_SIZE / 12), S_HIST_POS_Y + 6, x_axis[i], S_BG_COLOR, black);
    }

    /* draw history */
    screen_speed_hist();
}

void screen_dir_hist_init(void)
{
    char i;

    /* draw labels for direction-history y_axis */
    for (i = 0; i < 5; i++)
    {
        vga_bitmap_2c(D_HIST_POS_X - 18, D_HIST_POS_Y - 4 - (i * 10 * D_HIST_MULT), dir_y_axis[i], D_BG_COLOR, black);
    }

    /* draw background for Wind Direction History */
    vga_fill(D_HIST_POS_X, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X + ARRAY_SIZE, D_HIST_POS_Y, D_BG_COLOR);
    vga_line(D_HIST_POS_X + ARRAY_SIZE, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X + ARRAY_SIZE, D_HIST_POS_Y, D_LINE_COLOR);
    vga_line(D_HIST_POS_X, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X, D_HIST_POS_Y, D_LINE_COLOR);

    /* draw labels for speed-history x_axis */
    for (i = 0; i < 13; i++)
    {
        vga_bitmap_2c(D_HIST_POS_X - (vga_getbmwidth(x_axis[i]) >> 1) + (i * ARRAY_SIZE / 12), D_HIST_POS_Y + 6, x_axis[i], D_BG_COLOR, black);
    }

    /* draw history */
    screen_dir_hist();
}

/**********************************************************************
|*
|*  FUNCTION    : screen_update
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the history and values
*/

void screen_update(void)
{
    static      TMR_TIMER update_hist = 0;
    static      TMR_TIMER update_val = 0;
    static char state_hist = 4;
    static char state_val = 4;

    if (tmr0_expired(update_hist) && (state_hist == 0))
    {
        update_hist = tmr0_settimeout(SCREEN_HIST_RATE);
        update_temp_hist();
        update_speed_hist();
        update_dir_hist();
        state_hist = 1;
    }

    switch (state_hist)
    {
        case 1: /* update the temperature history */
            screen_temp_hist();
            state_hist = 2;
            break;
        case 2: /* update the wind speed history */
            screen_speed_hist();
            state_hist = 3;
            break;
        case 3: /* update the wind direction history */
            screen_dir_hist();
            state_hist = 0;
            break;
        case 4: /* first time after reset, init the update_hist */
            update_hist = tmr0_settimeout(1);
            state_hist = 0;
        default:
            state_hist = 0;
    }

    if (tmr0_expired(update_val) && (state_val == 0))
    {
        update_val = tmr0_settimeout(SCREEN_VAL_RATE);
        state_val = 1;
    }

    switch (state_val)
    {
        case 1: /* update the temperature value */
            screen_temp_val();
            state_val = 2;
            break;
        case 2: /* update the wind speed value */
            screen_speed_val();
            state_val = 3;
            break;
        case 3: /* update the wind direction value */
            screen_dir_val();
            state_val = 0;
            break;
        case 4: /* first time after reset, init the update_val */
            update_val = tmr0_settimeout(1);
            state_val = 0;
        default:
            state_val = 0;
    }

}

/**********************************************************************
|*
|*  FUNCTION    : update_temp_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the temperature history
*/

static void update_temp_hist(void)
{
    unsigned int i;

    for (i = 1; i < ARRAY_SIZE; i++)
    {
        temp_array[i - 1] = temp_array[i];
    }

    /* clip current value */
    if (temp_celcius < - 20)
    {
        temp_array[ARRAY_SIZE - 1] = - 20;
    }
    else if (temp_celcius > 50)
    {
        temp_array[ARRAY_SIZE - 1] = 50;
    }
    else
    {
        temp_array[ARRAY_SIZE - 1] = temp_celcius;
    }
}

/**********************************************************************
|*
|*  FUNCTION    : update_speed_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind speed history
*/

static void update_speed_hist(void)
{
    int i;

    for (i = 1; i < ARRAY_SIZE; i++)
    {
        speed_array[i - 1] = speed_array[i];
    }

    /* clip value */
    if (speed_knots > 500)
    {
        speed_array[ARRAY_SIZE - 1] = 500;
    }
    else
    {
        speed_array[ARRAY_SIZE - 1] = speed_knots;
    }
}

/**********************************************************************
|*
|*  FUNCTION    : update_dir_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind direction history
*/

static void update_dir_hist(void)
{
    int i;

    for (i = 1; i < ARRAY_SIZE; i++)
    {
        dir_array[i - 1] = dir_array[i];
    }
    dir_array[ARRAY_SIZE - 1] = wind_direction;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_temp_val
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the temperature value on the screen
*/

static void screen_temp_val(void)
{
    static int last_temp = - 100;
    int        x;

    if (last_temp != temp_celcius)
    {
        /* Right aligned */
        x = T_VAL_POS_X2 - screen_getvalwidth(temp_celcius, 0);
        vga_fill(T_VAL_POS_X2 - 135, T_VAL_POS_Y, x, T_VAL_POS_Y + 35, black);
        x = screen_putval(x, T_VAL_POS_Y, temp_celcius, 0, T_FG1_COLOR, black);

        last_temp = temp_celcius;
    }
}

/**********************************************************************
|*
|*  FUNCTION    : screen_speed_val
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind speed value on the screen
*/

static void screen_speed_val(void)
{
    static int last_speed = - 1;
    int        x;

    if (last_speed != speed_knots)
    {
        /* Right aligned */
        x = S_VAL_POS_X2 - screen_getvalwidth(speed_knots, 1);
        vga_fill(S_VAL_POS_X2 - 135, S_VAL_POS_Y, x, S_VAL_POS_Y + 35, black);
        x = screen_putval(x, S_VAL_POS_Y, speed_knots, 1, S_FG1_COLOR, black);

        last_speed = speed_knots;
    }
}

/**********************************************************************
|*
|*  FUNCTION    : screen_dir_val
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind direction value on the screen
*/

static void screen_dir_val(void)
{
    static         __rom char * last_dir = 0;
    unsigned int   x;
    __rom char   * dir;

    dir = compass_card(wind_direction);
    if (last_dir != dir)
    {
        /* Right aligned */
        x = D_VAL_POS_X2 - screen_getswidth(dir);
        vga_fill(D_VAL_POS_X2 - 135, D_VAL_POS_Y, x, D_VAL_POS_Y + 35, black);
        x = screen_puts(x, D_VAL_POS_Y, dir, D_FG1_COLOR, black);

        last_dir = dir;
    }
}

/**********************************************************************
|*
|*  FUNCTION    : screen_temp_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the temperature history on the screen
*/

static void screen_temp_hist(void)
{
    int i;
    char j;
    static char old_chart_type = FILL;

    if (chart_type == LINE)
    {
        for (i = 1; i < ARRAY_SIZE - 1; i++)
        {
            if (old_chart_type != chart_type)
            {
                if (i == 1)
                {
                    vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y, T_BG_COLOR);
                }
                vga_line(T_HIST_POS_X + i + 1, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i + 1, T_HIST_POS_Y, T_BG_COLOR);
            }

            if (temp_array[i - 1] != temp_array[i])
            {
                /* clear old line */
                vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i + 1, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_BG_COLOR);
            }

            /* draw horizontal lines */
            for (j = 0; j < 80; j += 10)
            {
                vga_plot(T_HIST_POS_X + i + 1, T_HIST_POS_Y - ((j - 20) * T_HIST_MULT), T_LINE_COLOR);
            }

            if (temp_array[i] != temp_array[i + 1])
            {
                /* plot new line */
                vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i + 1, T_HIST_POS_Y - (temp_array[i + 1] * T_HIST_MULT / T_HIST_DIV), T_FG1_COLOR);
            }
            else
            {
                vga_plot(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_FG1_COLOR);
            }
        }
    }
    else
    {
        for (i = 1; i < ARRAY_SIZE; i++)
        {
            if (old_chart_type == chart_type)
            {
                if (temp_array[i - 1] > 0 && temp_array[i] > 0)
                {
                    if (temp_array[i - 1] < temp_array[i])
                    {
                        vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_FG2_COLOR);
                    }
                    else if (temp_array[i - 1] != temp_array[i])
                    {
                        vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_BG_COLOR);
                    }
                }
                else if (temp_array[i - 1] < 0 && temp_array[i] < 0)
                {
                    if (temp_array[i - 1] > temp_array[i])
                    {
                        vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_FG2_COLOR);
                    }
                    else if (temp_array[i - 1] != temp_array[i])
                    {
                        vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_BG_COLOR);
                    }
                }
                else
                {
                    vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y, T_BG_COLOR);
                    vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y, T_FG2_COLOR);
                }
            }
            else
            {
                vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y, T_BG_COLOR);
                if (i < ARRAY_SIZE - 1)
                {
                    vga_line(T_HIST_POS_X + i + 1, T_HIST_POS_Y - (temp_array[i - 1] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i + 1, T_HIST_POS_Y, T_BG_COLOR);
                }
                vga_line(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_HIST_POS_X + i, T_HIST_POS_Y, T_FG2_COLOR);
            }
            /* draw horizontal lines */
            for (j = 0; j < 80; j += 10)
            {
                vga_plot(T_HIST_POS_X + i, T_HIST_POS_Y - ((j - 20) * T_HIST_MULT), T_LINE_COLOR);
            }
            /* draw measuring point */
            vga_plot(T_HIST_POS_X + i, T_HIST_POS_Y - (temp_array[i] * T_HIST_MULT / T_HIST_DIV), T_FG1_COLOR);
        }
    }
    old_chart_type = chart_type;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_speed_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind speed history on the screen
*/

static void screen_speed_hist(void)
{
    int i;
    char j;
    static char old_chart_type = FILL;

    if (chart_type == LINE)
    {
        for (i = 1; i < ARRAY_SIZE - 1; i++)
        {
            if (old_chart_type != chart_type)
            {
                if (i == 1)
                {
                    vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i, S_HIST_POS_Y, S_BG_COLOR);
                }
                vga_line(S_HIST_POS_X + i + 1, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i + 1, S_HIST_POS_Y, S_BG_COLOR);
            }

            if (speed_array[i - 1] != speed_array[i])
            {
                /* clear old line */
                vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i - 1] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i + 1, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_BG_COLOR);
            }

            /* draw horizontal lines */
            for (j = 0; j < 60; j += 10)
            {
                vga_plot(S_HIST_POS_X + i + 1, S_HIST_POS_Y - (j * S_HIST_MULT), S_LINE_COLOR);
            }

            if (speed_array[i] != speed_array[i + 1])
            {
                /* plot new line */
                vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i + 1, S_HIST_POS_Y - (speed_array[i + 1] * S_HIST_MULT / S_HIST_DIV), S_FG1_COLOR);
            }
            else
            {
                vga_plot(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_FG1_COLOR);
            }
        }
    }
    else
    {
        for (i = 1; i < ARRAY_SIZE; i++)
        {
            if (old_chart_type == chart_type)
            {
                if (speed_array[i - 1] < speed_array[i])
                {
                    vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i - 1] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_FG2_COLOR);
                }
                else if (speed_array[i - 1] != speed_array[i])
                {
                    vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i - 1] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_BG_COLOR);
                }
            }
            else
            {
                vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i - 1] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i, S_HIST_POS_Y, S_BG_COLOR);
                if (i < ARRAY_SIZE - 1)
                {
                    vga_line(S_HIST_POS_X + i + 1, S_HIST_POS_Y - (speed_array[i - 1] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i + 1, S_HIST_POS_Y, S_BG_COLOR);
                }
                vga_line(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_HIST_POS_X + i, S_HIST_POS_Y, S_FG2_COLOR);
            }

            /* draw horizontal lines */
            for (j = 0; j < 60; j += 10)
            {
                vga_plot(S_HIST_POS_X + i, S_HIST_POS_Y - (j * S_HIST_MULT), S_LINE_COLOR);
            }

            /* draw measuring point */
            vga_plot(S_HIST_POS_X + i, S_HIST_POS_Y - (speed_array[i] * S_HIST_MULT / S_HIST_DIV), S_FG1_COLOR);
        }
    }
    old_chart_type = chart_type;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_dir_hist
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Update the wind direction history on the screen
*/

static void screen_dir_hist(void)
{
    int i;
    char j;
    static char old_chart_type = FILL;

    if (chart_type == LINE)
    {
        for (i = 1; i < ARRAY_SIZE - 1; i++)
        {
            if (old_chart_type != chart_type)
            {
                if (i == 1)
                {
                    vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i, D_HIST_POS_Y, D_BG_COLOR);
                }
                vga_line(D_HIST_POS_X + i + 1, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i + 1, D_HIST_POS_Y, D_BG_COLOR);
            }

            if (dir_array[i - 1] != dir_array[i])
            {
                /* clear old line */
                vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i - 1] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i + 1, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_BG_COLOR);
            }
            /* draw horizontal lines */
            for (j = 0; j < 50; j += 10)
            {
                vga_plot(D_HIST_POS_X + i + 1, D_HIST_POS_Y - (j * D_HIST_MULT), D_LINE_COLOR);
            }

            if (dir_array[i] != dir_array[i + 1])
            {
                /* plot new line */
                vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i + 1, D_HIST_POS_Y - (dir_array[i + 1] * D_HIST_MULT / D_HIST_DIV), D_FG1_COLOR);
            }
            else
            {
                vga_plot(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_FG1_COLOR);
            }
        }
    }
    else
    {
        for (i = 1; i < ARRAY_SIZE; i++)
        {
            if (old_chart_type == chart_type)
            {
                if (dir_array[i - 1] < dir_array[i])
                {
                    vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i - 1] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_FG2_COLOR);
                }
                else if (dir_array[i - 1] != dir_array[i])
                {
                    vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i - 1] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_BG_COLOR);
                }
            }
            else
            {
                vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i - 1] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i, D_HIST_POS_Y, D_BG_COLOR);
                if (i < ARRAY_SIZE - 1)
                {
                    vga_line(D_HIST_POS_X + i + 1, D_HIST_POS_Y - (dir_array[i - 1] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i + 1, D_HIST_POS_Y, D_BG_COLOR);
                }
                vga_line(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_HIST_POS_X + i, D_HIST_POS_Y, D_FG2_COLOR);
            }

            /* draw horizontal lines */
            for (j = 0; j < 50; j += 10)
            {
                vga_plot(D_HIST_POS_X + i, D_HIST_POS_Y - (j * D_HIST_MULT), D_LINE_COLOR);
            }

            /* draw measuring point */
            vga_plot(D_HIST_POS_X + i, D_HIST_POS_Y - (dir_array[i] * D_HIST_MULT / D_HIST_DIV), D_FG1_COLOR);
        }
    }
    old_chart_type = chart_type;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_puts
|*
|*  INPUT       : x, y               = upper-left position of the string
|*                s                  = pointer to string in ROM
|*                fg_color, bg_color = foreground and background colors
|*
|*  OUTPUT      : end x-position of the string
|*
|*  DESCRIPTION : Put a string on the screen
*/

unsigned int screen_puts(unsigned int x, unsigned int y, __rom char * s, unsigned char fg_color, unsigned char bg_color)
{
    while (* s)
    {
        x += vga_bitmap_2c(x, y, vga_charset(* s++), fg_color, bg_color);
    }

    return x;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_getswidth
|*
|*  INPUT       : s = pointer to string in ROM
|*
|*  OUTPUT      : Width of the string in pixels
|*
|*  DESCRIPTION : Get the width of a string
*/

unsigned int screen_getswidth(__rom char * s)
{
    unsigned int x = 0;
    while (* s)
    {
        x += vga_getbmwidth(vga_charset(* s++));
    }

    return x;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_getvalwidth
|*
|*  INPUT       : val  = value
|*              : prec = number of decimals
|*
|*  OUTPUT      : Width of the value in pixels
|*
|*  DESCRIPTION : Get the width of a value
*/

unsigned int screen_getvalwidth(long val, char prec)
{
    unsigned int x = 0;
    char * p;
    p = printval(val, prec);

    while (* p)
    {
        x += vga_getbmwidth(vga_charset(* p++));
    }

    return x;
}

/**********************************************************************
|*
|*  FUNCTION    : screen_putval
|*
|*  INPUT       : x, y               = upper left position of the value on the screen
|*                val                = value to put on the screen
|*                prec               = number of decimals
|*                fg_color, bg_color = foreground and background colors
|*
|*  OUTPUT      : End x-position of the value on the screen
|*
|*  DESCRIPTION : Put a value on the screen
*/

unsigned int screen_putval(unsigned int x, unsigned int y, long val, char prec, unsigned char fg_color, unsigned char bg_color)
{
    char * p;
    p = printval(val, prec);

    while (* p)
    {
        x += vga_bitmap_2c(x, y, vga_charset(* p++), fg_color, bg_color);
    }

    return x;
}


