/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    IP-over-RS232 enabled weather station
**                      Main
**
**************************************************************************/

#include <string.h>
#include "tealib\i2c.h"
#include "tealib\lcd0.h"
#include "tealib\keypad.h"
#include "tealib\timer0.h"
#include "main.h"
#include "tealib\uart0.h"
#include "sms_settings.h"
#include "sms_init.h"
#include "sms_out.h"
#include "sms_comm.h"

#define SPEED_IN        P3_3
#define FILTER_SIZE     150     /* filter size for speed calculation */
#define K               25      /* constant for speed_calculation */
#define UPDATE_RATE     100L
#define SMS_TIME        3600L   /* SMS interval time ( in seconds ) */

typedef unsigned char KEY;

TMR_TIMER time;
static char format_speed;
static char format_dir;
static char format_temp;
static unsigned int speed_mps;      /* speed in m/s */
static unsigned int speed_kmph;     /* speed in km/h */
static unsigned int speed_mph;      /* speed in MPH */
static int temp_fahrenheit;
static char temp_corr;
static unsigned int direction_corr = 134;
static unsigned char ws_address = 1;
static char ascii[10];
unsigned char filter[FILTER_SIZE];
struct
{
    int min_temp;
    int max_temp;
} minmax_temp[T_MINMAX_ARRAY_SIZE];
unsigned int maxavg_speed[S_MAXAVR_ARRAY_SIZE];

unsigned int speed_knots;           /* speed in knots */
unsigned int max_speed;
unsigned int avg_speed;
static unsigned long speed_sum;
int temp_celcius;
int max_temp;
int min_temp;
unsigned int wind_direction;

TMR_TIMER uptime;
unsigned short uptime_seconds;
unsigned short uptime_minutes;
unsigned short uptime_hours;
unsigned short uptime_days;


__rom const int celcius_table[] =
{
    200, 178, 145, 128, 116, 108, 101, 96, 91, 87, 84, 81, 78, 75, 73, 71, 69, 67, 65, 63, 62, 60, 59, 58, 56, 55, 54, 53, 52, 51, 50, 49,
        48, 47, 46, 45, 44, 44, 43, 42, 41, 41, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34, 33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27,
        27, 26, 26, 26, 25, 25, 24, 24, 23, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15, 14,
        14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 2, 2, 2
        , 1, 1, 1, 1, 0, 0, 0, - 1, - 1, - 1, - 1, - 2, - 2, - 2, - 3, - 3, - 3, - 4, - 4, - 4, - 4, - 5, - 5, - 5, - 6, - 6, - 6, - 7, - 7,
        - 7, - 7, - 8, - 8, - 8, - 9, - 9, - 9, - 10, - 10, - 10, - 11, - 11, - 11, - 12, - 12, - 12, - 12, - 13, - 13, - 13, - 14, - 14, -
        14, - 15, - 15, - 16, - 16, - 16, - 17, - 17, - 17, - 18, - 18, - 18, - 19, - 19, - 20, - 20, - 20, - 21, - 21, - 22, - 22, - 22, -
        23, - 23, - 24, - 24, - 25, - 25, - 26, - 26, - 26, - 27, - 27, - 28, - 29, - 29, - 30, - 30, - 31, - 31, - 32, - 33, - 33, - 34, -
        35, - 35, - 36, - 37, - 38, - 39, - 39, - 40, - 41, - 42, - 43, - 45, - 46, - 47, - 49, - 51, - 53, - 55, - 57, - 61, - 65, - 73
};

__rom const int fahrenheit_table[] =
{
    400, 352, 293, 262, 241, 226, 214, 205, 196, 189, 183, 177, 172, 167, 163, 159, 156, 152, 149, 146, 143, 141, 138, 136, 133, 131, 129,
        127, 125, 123, 121, 120, 118, 116, 115, 113, 112, 110, 109, 108, 106, 105, 104, 102, 101, 100, 99, 98, 97, 96, 94, 93, 92, 91, 90,
        89, 88, 87, 87, 86, 85, 84, 83, 82, 81, 80, 80, 79, 78, 77, 76, 76, 75, 74, 73, 73, 72, 71, 70, 70, 69, 68, 68, 67, 66, 65, 65, 64,
        63, 63, 62, 62, 61, 60, 60, 59, 58, 58, 57, 57, 56, 55, 55, 54, 54, 53, 52, 52, 51, 51, 50, 49, 49, 48, 48, 47, 47, 46, 46, 45, 44,
        44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27,
        26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 12, 12, 11, 11, 10, 10, 9, 8, 8,
        7, 7, 6, 5, 5, 4, 3, 3, 2, 1, 1, 0, 0, - 1, - 2, - 3, - 3, - 4, - 5, - 5, - 6, - 7, - 8, - 8, - 9, - 10, - 11, - 11, - 12, - 13, -
        14, - 15, - 16, - 17, - 17, - 18, - 19, - 20, - 21, - 22, - 23, - 24, - 26, - 27, - 28, - 29, - 30, - 32, - 33, - 34, - 36, - 37, -
        39, - 41, - 42, - 44, - 46, - 48, - 51, - 53, - 56, - 59, - 63, - 67, - 71, - 77, - 86, - 99
};

void init_periph(void);
void init_temp_array(void);
void update_lcd(void);
void display_lcd( void );
char format(void);
void calc_wind_speed( unsigned char count );
void calc_minmax_temp( void );
void calc_maxavr_speed( void );
void menu(void);
void cal_temp(void);
void cal_dir(void);
void set_address(void);
void putval_lcd( long val, char prec );
void puts_lcd( char *s );


void main(void)
{
    char last_speed_in;
    TMR_TIMER update;
    TMR_TIMER meas_time;
    TMR_TIMER sms_send_time;
    char puls_count;
    char sms_text[160];

    init_periph();
    init_temp_array();
    update = tmr0_settimeout( 0 );
    sms_send_time = tmr0_settimeout( 0 );

    while (1)
    {
        uptime = tmr0_getclock() / 100;
        uptime_seconds = uptime % 60;
        uptime /= 60;
        uptime_minutes = uptime % 60;
        uptime /= 60;
        uptime_hours = uptime % 24;
        uptime /= 24;
        uptime_days = uptime;

        if ( tmr0_expired( update ))
        {
            read_temp_dir();
            update = tmr0_settimeout(UPDATE_RATE);
        }

        puls_count = 0;
        meas_time = tmr0_settimeout( 100 / K );
        last_speed_in = SPEED_IN;
        while ( !tmr0_expired( meas_time ))
        {
            if ( last_speed_in != SPEED_IN )
            {
                last_speed_in = SPEED_IN;
                if ( SPEED_IN == 0 )
                {
                    puls_count++;
                    tmr0_delay( 1 );
                }
            }
        }
        calc_wind_speed( puls_count );

        calc_maxavr_speed();
        calc_minmax_temp();

        update_lcd();

        if ( tmr0_expired( sms_send_time ))
        {
            /* set timer for next sms */
            sms_send_time = tmr0_settimeout( TMR0_TICKS_PER_SECOND * SMS_TIME );

            /* create sms text */
            strcpy(sms_text, "temperature=");
            strcat(sms_text, printval( temp_celcius, 0 ));
            strcat(sms_text, ";mintemp=");
            strcat(sms_text, printval( min_temp, 0 ));
            strcat(sms_text, ";maxtemp=");
            strcat(sms_text, printval( max_temp, 0 ));
            strcat(sms_text, ";windspeed=");
            strcat(sms_text, printval( speed_knots, 0 ));
            strcat(sms_text, ";avgspeed=");
            strcat(sms_text, printval (avg_speed, 0 ));
            strcat(sms_text, ";winddirection=");
            strcat(sms_text, printval( wind_direction, 0 ));

            /* send sms */
            send_sms( SMS_DESTINATION, sms_text );
        }
    }
}


void init_periph(void)
{
    unsigned char ad_setup = 0x82; /* 1110.0010 */
    unsigned char ad_config = 0x03; /* 0000.0011 */
    TMR_TIMER timeout;
    char sms_resp[20];

    /*Initialize the LCD */
    lcd_init();
    puts_lcd( "Weather  Station" );
    puts_lcd( "  Initializing" );

    /* Initialize serial port */
    uart0_init();

    /* Initialize the Timer-0 */
    tmr0_init();

    tmr0_delay( 1 * TMR0_TICKS_PER_SECOND );
    lcd_clear_screen();
    puts_lcd( " Searching for" );
    lcd_gotoxy( 3, 1 );
    puts_lcd( "GSM modem" );
    timeout = tmr0_settimeout( 20 * TMR0_TICKS_PER_SECOND );
    do {
        exec_cmd( "ATE0", sms_resp, 1, "OK", 0 );
        if ( strstr( sms_resp, "OK" ) != 0 )
        {
            break;
        }
    } while ( !tmr0_expired( timeout ));


    lcd_clear_screen();
    puts_lcd( "GSM modem found" );
    lcd_gotoxy( 0 ,1 );
    puts_lcd( "PINcode" );

    if ( check_for_pin( SMS_PINCODE ) == 0 )
    {
       puts_lcd( " FAILED" );
       for (;;);
    }
    else
    {
       puts_lcd( " OK" );
    }

    tmr0_delay( 1 * TMR0_TICKS_PER_SECOND );

    /* Initialize the I2C */
    I2C_init();

    /* Initalize the ADC to read channel 0 and 1 ... */
    I2C_write(ADC, &ad_setup, 1);
    I2C_write(ADC, &ad_config, 1);
    /* ... and read ones the values */
    read_temp_dir();
}

void init_temp_array( void )
{
    int i;
    
    for ( i = 0; i < T_MINMAX_ARRAY_SIZE; i++ )
    {
       minmax_temp[i].min_temp = temp_celcius;
       minmax_temp[i].max_temp = temp_celcius;
    }
}


void read_temp_dir( void )
{
    unsigned char ad_setup = 0x82; /* 1110.0010 */
    unsigned char ad_config = 0x03; /* 0000.0011 */
    unsigned char analog[2];
    int           index;

    /* Re-initialize the I2C */
    I2C_init();
    /* Re-initalize the ADC to read channel 0 and 1 ... */
    I2C_write(ADC, &ad_setup, 1);
    I2C_write(ADC, &ad_config, 1);

    I2C_read(ADC, analog, 2);

    wind_direction = (((analog[0] * 45) / 32) + direction_corr) % 360;

    index = analog[1] - temp_corr;
    index = (index < 0) ? 0: index;
    index = (index > 255) ? 255: index;

    temp_celcius = celcius_table[index];
    temp_fahrenheit = fahrenheit_table[index];
}


char format(void)
{
    KEY key;

    key = keypad();
    if (key != 0x00)
    {
        switch (key)
        {
            case 0x01:
                format_speed = 0;
                format_dir = 0;
                format_temp = 0;
                break;
            case 0x02:
                format_speed++;
                format_speed &= 0x03;
                break;
            case 0x03:
                format_dir++;
                format_dir &= 0x01;
                break;
            case 0x04:
                format_temp++;
                format_temp &= 0x01;
                break;
            case 0x10:
                menu();
                break;
            default:
                break;
        }
    }
    return key ? 1: 0;
}

void update_lcd(void)
{
    static TMR_TIMER update = 0;

    if ( format() || tmr0_expired( update ))
    {
        update = tmr0_settimeout( LCD_RATE );
        display_lcd();
    }
}


void display_lcd(void)
{
    lcd_clear_screen();

#if 0
    putval_lcd(speed_knots, 1);
    lcd_gotoxy(6, 0);
    putval_lcd(max_speed, 1);
    lcd_gotoxy(11, 0);
    putval_lcd(avg_speed, 1);
    
    lcd_gotoxy(0, 1);
    putval_lcd(temp_celcius, 0);    
    lcd_gotoxy(6, 1);
    putval_lcd(min_temp, 0);    
    lcd_gotoxy(11, 1);
    putval_lcd(max_temp, 0);    
#else
    switch (format_speed)
    {
        case 0:
            putval_lcd(speed_knots, 1);
            puts_lcd(" Knots");
            break;
        case 1:
            putval_lcd(speed_mps, 1);
            puts_lcd(" m/s");
            break;
        case 2:
            putval_lcd(speed_kmph, 1);
            puts_lcd(" km/h");
            break;
        case 3:
            putval_lcd(speed_mph, 1);
            puts_lcd(" MPH");
            break;
        default:
            break;
    }

    lcd_gotoxy(12, 0);
    switch (format_dir)
    {
        case 0:
            puts_lcd(compass_card(wind_direction));
            break;
        case 1:
            putval_lcd(wind_direction, 0);
            lcd_putc(0xDF);
            break;
        default:
            break;
    }

    lcd_gotoxy(6, 1);
    switch (format_temp)
    {
        case 0:
            putval_lcd( temp_celcius, 0 );
            lcd_putc( 0xDF );
            lcd_putc( 'C' );
            break;
        case 1:
            putval_lcd( temp_fahrenheit, 0 );
            lcd_putc( 0xDF );
            lcd_putc( 'F' );
            break;
        default:
            break;
    }
#endif
}


void calc_wind_speed( unsigned char count )
{
    unsigned int total_count = 0;
    unsigned char i;

    for (i = 1; i < FILTER_SIZE; i++)
    {
        filter[i - 1] = filter[i];
        total_count += filter[i];
    }

    filter[FILTER_SIZE - 1] = count;
    total_count += count;
    speed_knots = ( total_count * 22 * K ) / FILTER_SIZE;
    speed_mps = ( total_count * 11 * K ) / FILTER_SIZE;
    speed_kmph = ( total_count * 41 * K ) / FILTER_SIZE;
    speed_mph = ( total_count * 25 * K ) / FILTER_SIZE;
}


void calc_minmax_temp( void )
{
    static TMR_TIMER sample;
    static int temp_high = -100;         /* max temperature since the last array-update */
    static int temp_low = 200;          /* min temperature since the last array-update */
    static int array_temp_high;   /* max temperature over the whole array */
    static int array_temp_low;    /* min temperature over the whole array */
    int i;
    
    temp_high = ( temp_celcius < temp_high ) ? temp_high : temp_celcius;
    temp_low  = ( temp_celcius > temp_low  ) ? temp_low   : temp_celcius;
    
    if ( tmr0_expired( sample ))
    {
       sample = tmr0_settimeout( T_MINMAX_INTERVAL );

       array_temp_high = temp_high;
       array_temp_low  = temp_low;
       for ( i = 0; i < T_MINMAX_ARRAY_SIZE-1; i++ )
       {
          minmax_temp[i].max_temp = minmax_temp[i+1].max_temp;
          minmax_temp[i].min_temp = minmax_temp[i+1].min_temp;
          array_temp_high = ( minmax_temp[i].max_temp < array_temp_high ) ? array_temp_high : minmax_temp[i].max_temp;
          array_temp_low  = ( minmax_temp[i].min_temp > array_temp_low  ) ? array_temp_low  : minmax_temp[i].min_temp;
       }
       minmax_temp[i].max_temp = temp_high;
       minmax_temp[i].min_temp = temp_low;
       temp_high = temp_celcius;
       temp_low  = temp_celcius;
    }
    
    max_temp = ( temp_high > array_temp_high ) ? temp_high : array_temp_high;
    min_temp = ( temp_low  < array_temp_low  ) ? temp_low  : array_temp_low;
}


void calc_maxavr_speed( void )
{
    static TMR_TIMER sample;
    int i;
    
    if ( tmr0_expired( sample ))
    {
       sample = tmr0_settimeout( S_MAXAVR_INTERVAL );

       max_speed = 0;
       speed_sum = 0;
       for ( i = 0; i < S_MAXAVR_ARRAY_SIZE-1; i++ )
       {
          maxavg_speed[i] = maxavg_speed[i+1];
          max_speed = ( maxavg_speed[i] < max_speed ) ? max_speed : maxavg_speed[i];
          speed_sum += maxavg_speed[i];
       }
       maxavg_speed[i] = speed_knots;
       max_speed = ( maxavg_speed[i] < max_speed ) ? max_speed : maxavg_speed[i];
       speed_sum += maxavg_speed[i];
       speed_sum /= S_MAXAVR_ARRAY_SIZE;
       avg_speed = (unsigned int) speed_sum;
    }
}


void menu(void)
{
    KEY key = 0;
    char menu_state = 0;
    char menu_update = 1;

    while (menu_state != -1)
    {
        switch (menu_state)
        {
            case 0: /* entry of menu, want to enter calibration menu? */
                if (menu_update)
                {
                    lcd_clear_screen();
                    puts_lcd("  CALIBRATION");
                    lcd_gotoxy( 0, 1 );
                    puts_lcd("Ok     next  Esc");
                    menu_update = 0;
                }
                switch (key)
                {
                    case 0x01: /* OK */
                        menu_state = 1;
                        menu_update = 1;
                        break;
                    case 0x03: /* next */
                        menu_state = 10;
                        menu_update = 1;
                        break;
                    case 0x04: /* Esc */
                        menu_state = -1;
                        break;
                    default:
                        break;
                }
                break;
            case 1: /* want to calibrate temerature? */
                if (menu_update)
                {
                    lcd_clear_screen();
                    puts_lcd("Cal: temperature");
                    lcd_gotoxy( 0, 1 );
                    puts_lcd("Ok     next  Esc");
                    menu_update = 0;
                }
                switch (key)
                {
                    case 0x01: /* OK */
                        menu_state = 3;
                        menu_update = 1;
                        break;
                    case 0x03: /* next */
                        menu_state = 2;
                        menu_update = 1;
                        break;
                    case 0x04: /* Esc */
                        menu_state = -1;
                        break;
                    default:
                        break;
                }
                break;
            case 2: /* want to calibrate wind direction */
                if (menu_update)
                {
                    lcd_clear_screen();
                    puts_lcd("Cal: direction");
                    lcd_gotoxy( 0, 1 );
                    puts_lcd("Ok     next  Esc");
                    menu_update = 0;
                }
                switch (key)
                {
                    case 0x01: /* OK */
                        menu_state = 4;
                        menu_update = 1;
                        break;
                    case 0x03: /* next */
                        menu_state = 1;
                        menu_update = 1;
                        break;
                    case 0x04: /* Esc */
                        menu_state = -1;
                        break;
                    default:
                        break;
                }
                break;
            case 3: /* calibrate temperature */
                cal_temp();
                menu_state = 0;
                menu_update = 1;
                break;
            case 4: /* calibrate direction */
                cal_dir();
                menu_state = 0;
                menu_update = 1;
                break;
            case 10: /* want to enter setup menu? */
                if (menu_update)
                {
                    lcd_clear_screen();
                    puts_lcd("     SETUP");
                    lcd_gotoxy( 0, 1 );
                    puts_lcd("Ok     next  Esc");
                    menu_update = 0;
                }
                switch (key)
                {
                    case 0x01: /* OK */
                        menu_state = 11;
                        menu_update = 1;
                        break;
                    case 0x03: /* next */
                        menu_state = 0;
                        menu_update = 1;
                        break;
                    case 0x04: /* Esc */
                        menu_state = -1;
                        break;
                    default:
                        break;
                }
                break;
            case 11: /* want to Setup address? */
                if (menu_update)
                {
                    lcd_clear_screen();
                    puts_lcd("Set Address");
                    lcd_gotoxy( 0, 1 );
                    puts_lcd("Ok           Esc");
                    menu_update = 0;
                }
                switch (key)
                {
                    case 0x01: /* OK */
                        menu_state = 13;
                        menu_update = 1;
                        break;
                    case 0x04: /* Esc */
                        menu_state = -1;
                        break;
                    default:
                        break;
                }
                break;
            case 13: /* set address */
                set_address();
                menu_state = 10;
                menu_update = 1;
                break;
            default:
                break;
        }
        key = keypad();
    }
}


void cal_temp(void)
{
    KEY key = 0;
    TMR_TIMER update;

    update = tmr0_settimeout( 0 );


    while ( key != 0x01 )
    {
        if ( tmr0_expired( update ) || ( key = keypad()))
        {
            switch (key)
            {
                case 0x02: /* change units */
                    format_temp++;
                    format_temp &= 0x01;
                    break;
                case 0x03: /* increase */
                    temp_corr = (temp_corr < 50) ? temp_corr + 1: temp_corr;
                    break;
                case 0x04: /* decrease */
                    temp_corr = (temp_corr > -50) ? temp_corr - 1: temp_corr;
                    break;
                default:
                    break;
            }

            read_temp_dir();
            lcd_clear_screen();
            puts_lcd("Temp: ");
            if ( format_temp )
            {
                putval_lcd( temp_fahrenheit, 0 );
                lcd_putc( 0xDF );
                lcd_putc( 'F' );
            }
            else
            {
                putval_lcd( temp_celcius, 0 );
                lcd_putc( 0xDF );
                lcd_putc( 'C' );
            }

            lcd_gotoxy( 0, 1 );
            puts_lcd("Ok  \xDF");
            if (format_temp)
            {
                lcd_putc('C');
            }
            else
            {
                lcd_putc('F');
            }
            lcd_gotoxy( 9, 1 );
            puts_lcd("+    -");
            update = tmr0_settimeout( 50 );
        }
    }
}


void cal_dir(void)
{
    KEY key = 0;
    TMR_TIMER update;

    update = tmr0_settimeout( 0 );


    while (key != 0x01)         /* OK */
    {
        if ( tmr0_expired( update ) || ( key = keypad()))
        {
            switch (key)
            {
                case 0x02: /* clear correction */
                    direction_corr += 360 - wind_direction;
                    break;
                case 0x03: /* increase */
                    direction_corr++;
                    break;
                case 0x04: /* decrease */
                    direction_corr = (direction_corr != 0) ? direction_corr - 1 : 359;
                    break;
                default:
                    break;
            }
            direction_corr %= 360;

            read_temp_dir();
            lcd_clear_screen();
            puts_lcd("Dir: ");
            putval_lcd(wind_direction, 0);
            lcd_putc(0xDF);
            lcd_gotoxy( 10, 0 );
            puts_lcd(compass_card(wind_direction));
            lcd_gotoxy( 0, 1 );
            puts_lcd("Ok North +    -");
            update = tmr0_settimeout( 50 );
        }
    }
}


void set_address(void)
{
    KEY key = 0x02;     /* key is not '0', not '3' and not '4' -> update screen without side-effects */


    while (key != 0x01)
    {
        if ( key )
        {
            if ( key == 0x03 )
            {
                    ws_address = (ws_address < 255) ? ws_address + 1: ws_address;
            }
            else if ( key == 0x04 )
            {
                    ws_address = (ws_address > 1) ? ws_address - 1: ws_address;
            }

            lcd_clear_screen();
            puts_lcd("Address: ");
            putval_lcd(ws_address, 0);
            lcd_gotoxy( 0 , 1 );
            puts_lcd("Ok       +    -");
        }
        key = keypad();
    }
}


char * compass_card(unsigned int dir)
{
    char * result;

    if ((dir < 12) || (dir > 348))
    {
        result = "N";
    }
    else if (dir < 34)
    {
        result = "NNE";
    }
    else if (dir < 57)
    {
        result = "NE";
    }
    else if (dir < 79)
    {
        result = "ENE";
    }
    else if (dir < 102)
    {
        result = "E";
    }
    else if (dir < 124)
    {
        result = "ESE";
    }
    else if (dir < 147)
    {
        result = "SE";
    }
    else if (dir < 169)
    {
        result = "SSE";
    }
    else if (dir < 192)
    {
        result = "S";
    }
    else if (dir < 214)
    {
        result = "SSW";
    }
    else if (dir < 237)
    {
        result = "SW";
    }
    else if (dir < 259)
    {
        result = "WSW";
    }
    else if (dir < 282)
    {
        result = "W";
    }
    else if (dir < 304)
    {
        result = "WNW";
    }
    else if (dir < 327)
    {
        result = "NW";
    }
    else
    {
        result = "NNW";
    }

    return result;

}


void puts_lcd( char *s )
{
    while ( *s )
    {
        lcd_putc(*s++);
    }
}

void putval_lcd( long val, char prec )
{
    char  *p;
    p = printval( val, prec );

    while( *p )
    {
       lcd_putc( *p++ );
    }
}

char *printval( long val, char prec )
{
    char  *p;
    char  i = 0;
    char sign = 0;

    if ( val < 0 )
    {
       val = -val;
       sign = 1;
    }

    p = ascii + sizeof( ascii );
    *--p = '\0';
    do
    {
       *--p = '0' + val % 10;
       val /= 10;
       i++;
       if ( i == prec )
       {
           *--p = '.';
       }
    } while( val || i <= prec );

    if ( sign )
    {
       *--p = '-';
    }

    return p;
}


