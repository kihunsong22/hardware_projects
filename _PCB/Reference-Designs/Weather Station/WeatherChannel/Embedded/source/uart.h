#ifndef __UART_H
#define __UART_H

extern void uart_init( void );
extern int uart_getchar( void );
extern void uart_putchar( char c );
extern void uart_puts( const char *str );
extern void uart_putval( unsigned short val );

#endif
