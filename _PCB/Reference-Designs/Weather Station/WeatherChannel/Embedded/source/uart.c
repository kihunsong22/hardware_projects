#include "uart.h"

#ifndef BAUDRATE
#define BAUDRATE    19200
#endif

#ifndef FOSC
#define FOSC    44237000
#endif

void uart_init( void )
{
    // Initialize the UART

    TMOD    = 0x20; // Timer 1 user in auto-reload, 8bit
    SCON  = 0x50;   // Mode 1, 8-bit mode with variable baudrate

// TH1 = 256 - ( 2^SMOD * Fosc ) / (BAUDRATE * 12 * 32)
    PCON &= 0x7F;
    TH1 = (unsigned char) (256 - FOSC / (BAUDRATE * 384L));
    TR1 = 1;       // Start timer
}

void uart_putchar( char c )
{
    TI = 0;
    SBUF = c;
    while ( !TI );
}

void uart_puts( const char *str )
{
    while( * str )
    {
       TI = 0;
       SBUF = *str++;
       while ( !TI );
    }
}

int uart_getchar( void )
{
    int x;
    if ( RI )
    {
        x = SBUF;
        RI = 0;
    }
    else
    {
        x = -1;
    }
    return x;
}
