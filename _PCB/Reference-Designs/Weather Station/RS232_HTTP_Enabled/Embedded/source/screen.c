#include "tealib\timer0.h"
#include "tealib\vga.h"
#include "char\garamond36.h"
#include "char\arial8.h"
#include "bmp\bitmap.h"
#include "screen.h"
#include "main.h"

/* typedefs */
typedef __rom unsigned char *( *charset )( const char c );

/* defines, macros */

#define LABEL_DIST (10 * (SCR_HIST_REFRESH_INTERVAL) / (TMR0_TICKS_PER_SECOND))
#if LABEL_DIST < 10
#error history update time is smaller than 1 second, increase "SCR_HIST_REFRESH_INTERVAL"
#elif LABEL_DIST > 26000
#error history update time is to long, decrease "SCR_HIST_REFRESH_INTERVAL"
#endif

#if LABEL_DIST <= 13
#undef LABEL_DIST
#define LABEL_DIST (60UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0min", "-1min", "-2min", "-3min", "-4min", "-5min", "-6min", "-7min", "-8min", "-9min", "-10min", "-11min", "-12min", "-13min"};
#elif LABEL_DIST <= 25
#undef LABEL_DIST
#define LABEL_DIST (120UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0min", "-2min", "-4min", "-6min", "-8min", "-10min", "-12min", "-14min", "-16min", "-18min", "-20min", "-22min", "-24min", "-26min"};
#elif LABEL_DIST <= 63
#undef LABEL_DIST
#define LABEL_DIST (300UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0min", "-5min", "-10min", "-15min", "-20min", "-25min", "-30min", "-35min", "-40min", "-45min", "-50min", "-55min", "-60min", "-65min"};
#elif LABEL_DIST <= 125
#undef LABEL_DIST
#define LABEL_DIST (600UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0min", "-10min", "-20min", "-30min", "-40min", "-50min", "-60min", "-70min", "-80min", "-90min", "-100min", "-110min", "-120min", "-130min"};
#elif LABEL_DIST <= 188
#undef LABEL_DIST
#define LABEL_DIST (900UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0min", "-15min", "-30min", "-45min", "-60min", "-75min", "-90min", "-105min", "-120min", "-135min", "-150min", "-165min", "-180min", "-195min"};
#elif LABEL_DIST <= 500
#undef LABEL_DIST
#define LABEL_DIST (1800UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0h", "-0.5h", "-1h", "-1.5h", "-2h", "-2.5h", "-3h", "-3.5h", "-4h", "-4.5h", "-5h", "-5.5h", "-6h", "-6.5h"};
#elif LABEL_DIST <= 1000
#undef LABEL_DIST
#define LABEL_DIST (3600UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0h", "-1h", "-2h", "-3h", "-4h", "-5h", "-6h", "-7h", "-8h", "-9h", "-10h", "-11h", "-12h", "-13h"};
#elif LABEL_DIST <= 2000
#undef LABEL_DIST
#define LABEL_DIST (7200UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0h", "-2h", "-4h", "-6h", "-8h", "-10h", "-12h", "-14h", "-16h", "-18h", "-20h", "-22h", "-24h", "-26h"};
#elif LABEL_DIST <= 6000
#undef LABEL_DIST
#define LABEL_DIST (21600UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0h", "-6h", "-12h", "-18h", "-24h", "-30h", "-36h", "-42h", "-48h", "-54h", "-60h", "-66h", "-72h", "-78h"};
#elif LABEL_DIST <= 12000
#undef LABEL_DIST
#define LABEL_DIST (43200UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0d", "-0.5d", "-1d", "-1.5d", "-2d", "-2.5d", "-3d", "-3.5d", "-4d", "-4.5d", "-5d", "-5.5d", "-6d", "-6.5d"};
#else
#undef LABEL_DIST
#define LABEL_DIST (86400UL * (TMR0_TICKS_PER_SECOND) / (SCR_HIST_REFRESH_INTERVAL))
__rom char * x_axis[] = {"0d", "-1d", "-2d", "-3d", "-4d", "-5d", "-6d", "-7d", "-8d", "-9d", "-10d", "-11d", "-12d", "-13d"};
#endif

/* static functions */
static void screen_splash(void);
static void update_temp_hist(void);
static void update_speed_hist(void);
static void update_dir_hist(void);
static void screen_temp_hist(void);
static void screen_speed_hist(void);
static void screen_dir_hist(void);
static void screen_temp_val(void);
static void screen_speed_val(void);
static void screen_dir_val(void);
static unsigned int screen_getvalwidth(long val, char prec, charset cs);
static unsigned int screen_putval(unsigned int x, unsigned int y, long val, char prec, charset cs, unsigned char fg_color, unsigned char bg_color);
static unsigned int screen_puts(unsigned int x, unsigned int y, __rom char * s, charset cs, unsigned char fg_color, unsigned char bg_color);
static unsigned int screen_getswidth(__rom char * s, charset cs);

/* variables */
unsigned char chart_type = fill;

/* static variables */
static int temp_array[HIST_ARRAYSIZE];
static int speed_array[HIST_ARRAYSIZE];
static int dir_array[HIST_ARRAYSIZE];


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

void screen_init(void)
{
    /* clear VGA */
    vga_init();
    
    vga_init();
    
    /* show altium bitmap */
    vga_bitmap_16c(L_POS_X2 - vga_getbmwidth(bmp_altium16), 0, bmp_altium16);

    /* init temperature history */
    screen_temp_hist_init();

    /* draw value */
    vga_bitmap_2c(T_VAL_POS_X2 - vga_getbmwidth(bmp_temp_c), T_LBL_POS_Y, bmp_temp_c, T_FG1_COLOR, black);
    screen_temp_val();

    /* init wind speed history */
    screen_speed_hist_init();

    /* draw value */
    vga_bitmap_2c(S_VAL_POS_X2 - vga_getbmwidth(bmp_speed), S_LBL_POS_Y, bmp_speed, S_FG1_COLOR, black);
    screen_speed_val();

    /* init wind direction history */
    screen_dir_hist_init();

    /* draw value */
    vga_bitmap_2c(D_VAL_POS_X2 - vga_getbmwidth(bmp_direction), D_LBL_POS_Y, bmp_direction, D_FG1_COLOR, black);
    screen_dir_val();
}


/**********************************************************************
|*
|*  FUNCTION    : screen_temp_hist_init
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Initialize the temperature history
*/

static void screen_temp_hist_init(void)
{
    char i;

    /* draw labels for temp-history y_axis */
    for (i = -2; i < 6; i++)
    {
        screen_putval(T_HIST_POS_X - 2 - screen_getvalwidth( 10 * i, 0, arial8 ), T_HIST_POS_Y - 4 - (i * 10 * T_HIST_MULT), 10 * i, 0, arial8, T_BG_COLOR, black);
    }

    /* draw background for Temperature History */
    vga_fill(T_HIST_POS_X, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X + HIST_ARRAYSIZE, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_BG_COLOR);
    vga_line(T_HIST_POS_X + HIST_ARRAYSIZE, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X + HIST_ARRAYSIZE, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_LINE_COLOR);
    vga_line(T_HIST_POS_X, T_HIST_POS_Y - (50 * T_HIST_MULT), T_HIST_POS_X, T_HIST_POS_Y - (- 20 * T_HIST_MULT), T_LINE_COLOR);

    /* draw labels for temp-history x_axis */
    for (i = 0; i <= ( HIST_ARRAYSIZE/LABEL_DIST ); i++)
    {
        screen_puts(T_HIST_POS_X + HIST_ARRAYSIZE - ( i * LABEL_DIST ) - ( screen_getswidth( x_axis[i], arial8 )/2), T_HIST_POS_Y + 6 - (- 20 * T_HIST_MULT), x_axis[i], arial8,T_BG_COLOR, black);
    }

    /* draw history */
    screen_temp_hist();
}


/**********************************************************************
|*
|*  FUNCTION    : screen_speed_hist_init
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Initialize the wind speed history
*/

static void screen_speed_hist_init(void)
{
    char i;

    /* draw labels for speed-history y_axis */
    for (i = 0; i < 6; i++)
    {
        screen_putval(S_HIST_POS_X - 2 - screen_getvalwidth( 10 * i, 0, arial8 ), S_HIST_POS_Y - 4 - (i * 10 * S_HIST_MULT), 10 * i, 0, arial8, S_BG_COLOR, black);
    }

    /* draw background for Wind Speed History */
    vga_fill(S_HIST_POS_X, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X + HIST_ARRAYSIZE, S_HIST_POS_Y, S_BG_COLOR);
    vga_line(S_HIST_POS_X + HIST_ARRAYSIZE, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X + HIST_ARRAYSIZE, S_HIST_POS_Y, S_LINE_COLOR);
    vga_line(S_HIST_POS_X, S_HIST_POS_Y - (50 * S_HIST_MULT), S_HIST_POS_X, S_HIST_POS_Y, S_LINE_COLOR);

    /* draw labels for speed-history x_axis */
    for (i = 0; i <= ( HIST_ARRAYSIZE/LABEL_DIST ); i++)
    {
        screen_puts(S_HIST_POS_X + HIST_ARRAYSIZE - ( i * LABEL_DIST ) - ( screen_getswidth( x_axis[i], arial8 )/2), S_HIST_POS_Y + 6, x_axis[i], arial8,S_BG_COLOR, black);
    }

    /* draw history */
    screen_speed_hist();
}


/**********************************************************************
|*
|*  FUNCTION    : screen_dir_hist_init
|*
|*  INPUT       : None
|*
|*  OUTPUT      : None
|*
|*  DESCRIPTION : Initialize the wind direction history
*/

static void screen_dir_hist_init(void)
{
    char i;

    /* draw labels for direction-history y_axis */
    vga_bitmap_2c(D_HIST_POS_X - 2 - screen_getswidth("N", arial8) , D_HIST_POS_Y - 4 - (0 * 10 * D_HIST_MULT), arial8('N'), D_BG_COLOR, black);
    vga_bitmap_2c(D_HIST_POS_X - 2 - screen_getswidth("E", arial8) , D_HIST_POS_Y - 4 - (1 * 10 * D_HIST_MULT), arial8('E'), D_BG_COLOR, black);
    vga_bitmap_2c(D_HIST_POS_X - 2 - screen_getswidth("S", arial8) , D_HIST_POS_Y - 4 - (2 * 10 * D_HIST_MULT), arial8('S'), D_BG_COLOR, black);
    vga_bitmap_2c(D_HIST_POS_X - 2 - screen_getswidth("W", arial8) , D_HIST_POS_Y - 4 - (3 * 10 * D_HIST_MULT), arial8('W'), D_BG_COLOR, black);
    vga_bitmap_2c(D_HIST_POS_X - 2 - screen_getswidth("N", arial8) , D_HIST_POS_Y - 4 - (4 * 10 * D_HIST_MULT), arial8('N'), D_BG_COLOR, black);

    /* draw background for Wind Direction History */
    vga_fill(D_HIST_POS_X, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X + HIST_ARRAYSIZE, D_HIST_POS_Y, D_BG_COLOR);
    vga_line(D_HIST_POS_X + HIST_ARRAYSIZE, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X + HIST_ARRAYSIZE, D_HIST_POS_Y, D_LINE_COLOR);
    vga_line(D_HIST_POS_X, D_HIST_POS_Y - (40 * D_HIST_MULT), D_HIST_POS_X, D_HIST_POS_Y, D_LINE_COLOR);

    /* draw labels for speed-history x_axis */
    for (i = 0; i <= ( HIST_ARRAYSIZE/LABEL_DIST ); i++)
    {
        screen_puts(D_HIST_POS_X + HIST_ARRAYSIZE - ( i * LABEL_DIST ) - ( screen_getswidth( x_axis[i], arial8 )/2), D_HIST_POS_Y + 6, x_axis[i], arial8,D_BG_COLOR, black);
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
        update_hist = tmr0_settimeout(SCR_HIST_REFRESH_INTERVAL);
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
        update_val = tmr0_settimeout(SCR_VAL_REFRESH_INTERVAL);
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

    for (i = 1; i < HIST_ARRAYSIZE; i++)
    {
        temp_array[i - 1] = temp_array[i];
    }

    /* clip current value */
    if (temp_celcius < - 20)
    {
        temp_array[HIST_ARRAYSIZE - 1] = - 20;
    }
    else if (temp_celcius > 50)
    {
        temp_array[HIST_ARRAYSIZE - 1] = 50;
    }
    else
    {
        temp_array[HIST_ARRAYSIZE - 1] = temp_celcius;
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

    for (i = 1; i < HIST_ARRAYSIZE; i++)
    {
        speed_array[i - 1] = speed_array[i];
    }

    /* clip value */
    if (speed_knots > 500)
    {
        speed_array[HIST_ARRAYSIZE - 1] = 500;
    }
    else
    {
        speed_array[HIST_ARRAYSIZE - 1] = speed_knots;
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

    for (i = 1; i < HIST_ARRAYSIZE; i++)
    {
        dir_array[i - 1] = dir_array[i];
    }
    dir_array[HIST_ARRAYSIZE - 1] = wind_direction;
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
        x = T_VAL_POS_X2 - screen_getvalwidth(temp_celcius, 0, garamond36);
        vga_fill(T_VAL_POS_X2 - 135, T_VAL_POS_Y, x, T_VAL_POS_Y + 35, black);
        x = screen_putval(x, T_VAL_POS_Y, temp_celcius, 0, garamond36, T_FG1_COLOR, black);

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
        x = S_VAL_POS_X2 - screen_getvalwidth(speed_knots, 1, garamond36);
        vga_fill(S_VAL_POS_X2 - 135, S_VAL_POS_Y, x, S_VAL_POS_Y + 35, black);
        x = screen_putval(x, S_VAL_POS_Y, speed_knots, 1, garamond36, S_FG1_COLOR, black);

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
        x = D_VAL_POS_X2 - screen_getswidth(dir, garamond36);
        vga_fill(D_VAL_POS_X2 - 135, D_VAL_POS_Y, x, D_VAL_POS_Y + 35, black);
        x = screen_puts(x, D_VAL_POS_Y, dir, garamond36, D_FG1_COLOR, black);

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
    static char old_chart_type = fill;

    if (chart_type == line)
    {
        for (i = 1; i < HIST_ARRAYSIZE - 1; i++)
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
        for (i = 1; i < HIST_ARRAYSIZE; i++)
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
                if (i < HIST_ARRAYSIZE - 1)
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
    static char old_chart_type = fill;

    if (chart_type == line)
    {
        for (i = 1; i < HIST_ARRAYSIZE - 1; i++)
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
        for (i = 1; i < HIST_ARRAYSIZE; i++)
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
                if (i < HIST_ARRAYSIZE - 1)
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
    static char old_chart_type = fill;

    if (chart_type == line)
    {
        for (i = 1; i < HIST_ARRAYSIZE - 1; i++)
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
        for (i = 1; i < HIST_ARRAYSIZE; i++)
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
                if (i < HIST_ARRAYSIZE - 1)
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
|*  FUNCTION    : screen_getswidth
|*
|*  INPUT       : s = pointer to string in ROM
|*
|*  OUTPUT      : Width of the string in pixels
|*
|*  DESCRIPTION : Get the width of a string
*/

static unsigned int screen_getswidth(__rom char * s, charset cs )
{
    unsigned int x = 0;
    while (* s)
    {
        x += vga_getbmwidth(cs(* s++));
    }

    return x;
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

static unsigned int screen_puts(unsigned int x, unsigned int y, __rom char * s, charset cs, unsigned char fg_color, unsigned char bg_color)
{
    while (* s)
    {
        x += vga_bitmap_2c(x, y, cs(* s++), fg_color, bg_color);
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

static unsigned int screen_getvalwidth(long val, char prec, charset cs)
{
    unsigned int x = 0;
    char * p;
    p = printval(val, prec);

    while (* p)
    {
        x += vga_getbmwidth(cs(* p++));
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

static unsigned int screen_putval(unsigned int x, unsigned int y, long val, char prec, charset cs, unsigned char fg_color, unsigned char bg_color)
{
    char * p;
    p = printval(val, prec);

    while (* p)
    {
        x += vga_bitmap_2c(x, y, cs(* p++), fg_color, bg_color);
    }

    return x;
}

