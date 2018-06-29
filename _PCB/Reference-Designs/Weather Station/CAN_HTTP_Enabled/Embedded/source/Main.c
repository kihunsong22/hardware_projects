/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:        IP-over-CAN enabled weather station
**                      Main
**
**************************************************************************/

#include "tealib/i2c.h"
#include "tealib/lcd0.h"
#include "tealib/keypad.h"
#include "tealib/timer0.h"
#include "tcpip_main.h"


#define SPEED_IN        P3_3
#define FILTER_SIZE 150   /* filter size for speed calculation */
#define K 25U          /* constant for speed_calculation */


typedef unsigned int KEY;

TMR_TIMER time;
static char format_speed;
static char format_dir;
static char format_temp;
static char temp_unit = 'C';
static int temp_fahrenheit;
static int temperature;
static char temp_corr;
static unsigned int direction_corr;

// export by CGI
int temp_celcius;
unsigned int wind_direction;
unsigned int wind_speed;
unsigned int ws_address = 1;

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
void display(void);
char format(void);
void calc_wind_speed( unsigned char count );
__rom char * compass_card(unsigned int dir);
void read_temp_dir();
void menu(void);
void cal_temp(void);
void cal_dir(void);
void menu_setup(void);
void putval_lcd( long val, char prec );
void puts_lcd( __rom char *s );


void main(void)
{
    char last_speed_in = 0;
    TMR_TIMER update;
    TMR_TIMER meas_time = 0;
    char puls_count;

    init_periph();

    tcpipmain_init();
    tcpipmain_setstation(ws_address);

    update = tmr0_settimeout(2);

    while (1)
    {
        while (tcpipmain()) {}

        if ( format() || tmr0_expired(update))
        {
            update = tmr0_settimeout(50);
            read_temp_dir();
            display();
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
                    tmr0_delay( 10 );
                }
            }
        }

        calc_wind_speed( puls_count );
    }
}


void init_periph(void)
{
    unsigned char ad_setup = 0x82; /* 1110.0010 */
    unsigned char ad_config = 0x03; /* 0000.0011 */

    // Initialize the LCD
    lcd_init();
    puts_lcd( "  Initializing" );

    // Initialize the Timer-0
    tmr0_init();

    // Initialize the I2C
    I2C_init();

    // Initalize the ADC to read channel 0 and 1
    I2C_write(ADC, & ad_setup, 1);
    I2C_write(ADC, & ad_config, 1);
}


void read_temp_dir()
{
    unsigned char analog[2];
    int           index;

    I2C_read(ADC, analog, 2);

    wind_direction = (((analog[0] * 45) / 32) + direction_corr) % 360;

    index = analog[1] - temp_corr;
    index = (index < 0) ? 0: index;
    index = (index > 255) ? 255: index;
    temp_celcius = celcius_table[(unsigned char) index];
    temp_fahrenheit = fahrenheit_table[(unsigned char) index];
    temperature = (format_temp == 0) ? temp_celcius: temp_fahrenheit;
}


char format(void)
{
    KEY key;

    key = keypad();
    if (key != 0x0000)
    {
        switch (key)
        {
            case 0x0001:
                format_speed = 0;
                format_dir = 0;
                format_temp = 0;
                temp_unit = 'C';
                break;
            case 0x0002:
                format_speed++;
                if (format_speed > 3)
                {
                    format_speed = 0;
                }
                break;
            case 0x0003:
                format_dir++;
                if (format_dir > 1)
                {
                    format_dir = 0;
                }
                break;
            case 0x0004:
                format_temp++;
                temp_unit = 'F';
                if (format_temp > 1)
                {
                    format_temp = 0;
                    temp_unit = 'C';
                }
                break;
            case 0x0005:
                lcd_showcursor( CURSOR_TYPE_BLOCK );
                break;
            case 0x0006:
                lcd_showcursor( CURSOR_TYPE_UNDERSCORE );
                break;
            case 0x0007:
                lcd_showcursor( CURSOR_TYPE_BLOCK | CURSOR_TYPE_UNDERSCORE);
                break;
            case 0x0008:
                lcd_hidecursor();
                break;
            case 0x0010:
                menu();
                break;
            default:
                break;
        }
    }
    return key ? 1: 0;
}


void display(void)
{

    lcd_clear_screen();

    switch (format_speed)
    {
        case 0:
            putval_lcd(wind_speed, 1);
            puts_lcd(" Knots");
            break;
        case 1:
            putval_lcd(wind_speed, 1);
            puts_lcd(" m/s");
            break;
        case 2:
            putval_lcd(wind_speed, 1);
            puts_lcd(" km/h");
            break;
        case 3:
            putval_lcd(wind_speed, 1);
            puts_lcd(" MPH");
            break;
        default:
            break;
    }

    switch (format_dir)
    {
        case 0:
            lcd_gotoxy(12, 0);
            puts_lcd(compass_card(wind_direction));
            break;
        case 1:
            lcd_gotoxy(12, 0);
            putval_lcd(wind_direction, 0);
            lcd_putc(0xDF);
            break;
        default:
            break;
    }

    lcd_gotoxy(6, 1);
    putval_lcd(temperature, 0);
    lcd_putc(0xDF);
    lcd_putc(temp_unit);
}


void calc_wind_speed( unsigned char count )
{
    static unsigned char filter[FILTER_SIZE] = { 0 };
    unsigned char total_count = 0;
    unsigned char k;
    unsigned char i;

    switch (format_speed)
    {
        case 0:
            k = 22;
            break;
        case 1:
            k = 11;
            break;
        case 2:
            k = 40;
            break;
        case 3:
            k = 25;
            break;
        default:
            k = 22;
            break;
    }

    for (i = 1; i < FILTER_SIZE; i++)
    {
        filter[i - 1] = filter[i];
        total_count += filter[i];
    }

    filter[FILTER_SIZE - 1] = count;
    total_count += count;
    wind_speed = ( total_count * k * K ) / FILTER_SIZE;
}


void menu(void)
{
    KEY key = 0;
    char menu_state = 0;
    char menu_update = 1;

    while (menu_state != - 1)
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
                    case 0x0001: /* OK */
                        menu_state = 1;
                        menu_update = 1;
                        break;
                    case 0x0003: /* next */
                        menu_state = 10;
                        menu_update = 1;
                        break;
                    case 0x0004: /* Esc */
                        menu_state = - 1;
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
                    case 0x0001: /* OK */
                        menu_state = 3;
                        menu_update = 1;
                        break;
                    case 0x0003: /* next */
                        menu_state = 2;
                        menu_update = 1;
                        break;
                    case 0x0004: /* Esc */
                        menu_state = - 1;
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
                    case 0x0001: /* OK */
                        menu_state = 4;
                        menu_update = 1;
                        break;
                    case 0x0003: /* next */
                        menu_state = 1;
                        menu_update = 1;
                        break;
                    case 0x0004: /* Esc */
                        menu_state = - 1;
                        break;
                    default:
                        break;
                }
                break;
            case 3: /* calibrate temperature */
                cal_temp();
                menu_state = 2;
                menu_update = 1;
                break;
            case 4: /* calibrate direction */
                cal_dir();
                menu_state = 1;
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
                    case 0x0001: /* OK */
                        menu_state = 11;
                        menu_update = 1;
                        break;
                    case 0x0003: /* next */
                        menu_state = 0;
                        menu_update = 1;
                        break;
                    case 0x0004: /* Esc */
                        menu_state = - 1;
                        break;
                    default:
                        break;
                }
                break;
            case 11:
                menu_setup();
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
    char update = 1;
    TMR_TIMER displ;

    displ = tmr0_settimeout(50);

    do
    {
        if (tmr0_expired(displ))
        {
            update = 1;
            displ = tmr0_settimeout(50);
        }

        if (update)
        {
            read_temp_dir();
            lcd_clear_screen();
            puts_lcd("Temp = ");
            putval_lcd(temperature, 0);
            lcd_putc(0xDF);
            lcd_putc(temp_unit);
            lcd_gotoxy( 0, 1 );
            puts_lcd("Ok  ");
            lcd_putc(0xDF);
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
            update = 0;
            displ = tmr0_settimeout(50);
        }

        key = keypad();
        switch (key)
        {
            case 0x0002: /* change units */
                format_temp++;
                temp_unit = 'F';
                if (format_temp > 1)
                {
                    format_temp = 0;
                    temp_unit = 'C';
                }
                update = 1;
                break;
            case 0x0003: /* increase */
                temp_corr = (temp_corr < 50) ? temp_corr + 1: temp_corr;
                update = 1;
                break;
            case 0x0004: /* decrease */
                temp_corr = (temp_corr > - 50) ? temp_corr - 1: temp_corr;
                update = 1;
                break;
            default:
                break;
        }

    }
    while (key != 0x0001); /* OK */
}


void cal_dir(void)
{
    KEY key = 0;
    char update = 1;
    TMR_TIMER displ;

    displ = tmr0_settimeout(50);

    do
    {
        if (tmr0_expired(displ))
        {
            update = 1;
            displ = tmr0_settimeout(50);
        }

        if (update)
        {
            read_temp_dir();
            lcd_clear_screen();
            puts_lcd("Dir: ");
            putval_lcd(wind_direction, 0);
            lcd_putc(0xDF);
            lcd_gotoxy( 10, 0 );
            puts_lcd(compass_card(wind_direction));
            lcd_gotoxy( 0, 1 );
            puts_lcd("Ok North +    -");
            update = 0;
            displ = tmr0_settimeout(50);
        }

        key = keypad();
        switch (key)
        {
            case 0x0002: /* clear correction */
                direction_corr = 0;
                update = 1;
                break;
            case 0x0003: /* increase */
                direction_corr = (direction_corr != 359) ? direction_corr + 1: 0;
                update = 1;
                break;
            case 0x0004: /* decrease */
                direction_corr = (direction_corr != 0) ? direction_corr - 1: 359;
                update = 1;
                break;
            default:
                break;
        }
    }
    while (key != 0x0001); /* OK */
}


void menu_setup(void)
{
    KEY key;
    char update = 1;

    unsigned int ws_address_org = ws_address;

    do
    {
        if (update)
        {
            lcd_clear_screen();
            puts_lcd("Address = ");
            putval_lcd(ws_address, 0);
            lcd_gotoxy( 0 , 1 );
            puts_lcd("Ok       +    -");
            update = 0;
        }
        key = keypad();

        switch (key)
        {
            case 0x0003:
                ws_address = (ws_address < 4) ? ws_address + 1: ws_address;
                update = 1;
                break;
            case 0x0004:
                ws_address = (ws_address > 1) ? ws_address - 1: ws_address;
                update = 1;
                break;
            default:
                break;
        }
    }
    while (key != 0x0001);

    if (ws_address_org != ws_address)
    {
        tcpipmain_setstation(ws_address);
    }
}


__rom char * compass_card(unsigned int dir)
{
    __rom char * result;

    if ((dir < 12) || (dir > 348))
    {
        result = "N  ";
    }
    else if (dir < 34)
    {
        result = "NNE";
    }
    else if (dir < 57)
    {
        result = "NE ";
    }
    else if (dir < 79)
    {
        result = "ENE";
    }
    else if (dir < 102)
    {
        result = "E  ";
    }
    else if (dir < 124)
    {
        result = "ESE";
    }
    else if (dir < 147)
    {
        result = "SE ";
    }
    else if (dir < 169)
    {
        result = "SSE";
    }
    else if (dir < 192)
    {
        result = "S  ";
    }
    else if (dir < 214)
    {
        result = "SSW";
    }
    else if (dir < 237)
    {
        result = "SW ";
    }
    else if (dir < 259)
    {
        result = "WSW";
    }
    else if (dir < 282)
    {
        result = "W  ";
    }
    else if (dir < 304)
    {
        result = "WNW";
    }
    else if (dir < 327)
    {
        result = "NW ";
    }
    else
    {
        result = "NNW";
    }

    return result;

}


void puts_lcd( __rom char *s )
{
    while (*s != 0)
    {
        lcd_putc(*s++);
    }
}
void putval_lcd( long val, char prec )
{
    char  ascii[10];
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

    while( * p )
    {
       lcd_putc( *p++ );
    }
}


