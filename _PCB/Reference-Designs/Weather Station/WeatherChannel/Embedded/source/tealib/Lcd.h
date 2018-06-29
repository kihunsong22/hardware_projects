/*****************************************************************************
 *
 *  VERSION:    %W% %E%
 *
 *      IN PACKAGE: TASKING Peripheral Library for 80515
 *
 *      AUTHORS:        SAPA
 *
 *      COPYRIGHT:  Copyright (c) 2003, Altium BV
 *
 *      DESCRIPTION:    Include file for alphanumeric lcd module
 *
 ****************************************************************************/

#ifndef _LCD_H
#define _LCD_H


/*
 * Perform some checking first
 */
#if (!defined ALPHALCD_WIDTH) || (ALPHALCD_WIDTH == 0)
#error > Specify valid display width
#endif

#if (!defined ALPHALCD_HEIGHT) || (ALPHALCD_HEIGHT == 0) || (ALPHALCD_HEIGHT == 3)
#error > Specify valid display height
#endif

#if (ALPHALCD_HEIGHT == 4) && (ALPHALCD_WIDTH > 20)
#error > Sorry, displays that are 4 line high are supported up to 20 characters wide.
#endif


#ifdef ALPHALCD_BITBANG
#ifndef ALPHALCD_BITBANG_DB
#error > Specify port for LCD databus.
#endif
#ifndef ALPHALCD_BITBANG_RS
#error > Specify portpin for LCD RS pin.
#endif
#ifndef ALPHALCD_BITBANG_RW
#error > Specify portpin for LCD RW pin.
#endif
#ifndef ALPHALCD_BITBANG_E
#error > Specify portpin for LCD E pin.
#endif
#else
#ifndef ALPHALCD_BASEADDR
#error > Specify baseaddress
#endif
#endif

#define DISPLAY_POWER_ON    0x04
#define DISPLAY_POWER_OFF   ~DISPLAY_POWER_ON

#define CURSOR_TYPE_OFF     0x00
#define CURSOR_TYPE_BLOCK   0x01
#define CURSOR_TYPE_UNDERSCORE  0x02
#define CURSOR_TYPE_BOTH    (CURSOR_TYPE_BLOCK | CURSOR_TYPE_UNDERSCORE)


typedef struct
{
    unsigned char x;
    unsigned char y;
} lcd_cursorpos_t;



/*
 * Backwards compatibility with pre-Viper Technology compilers
 */
#ifndef __C51__
#define __at    _at
#define __rom   _rom
#define __xdata _xdat
#endif


/* Prototypes public functions */

void lcd_init ( void );

void lcd_showcursor ( unsigned char type );
void lcd_hidecursor ( void );
void lcd_display_on ( void );
void lcd_display_off ( void );
void lcd_clear_screen ( void );

void lcd_putc ( unsigned char character );

void lcd_gotoxy ( unsigned char x, unsigned char y );
lcd_cursorpos_t lcd_getxy ( void );

void lcd_create_char ( unsigned char character, __rom unsigned char * pattern );

#endif
