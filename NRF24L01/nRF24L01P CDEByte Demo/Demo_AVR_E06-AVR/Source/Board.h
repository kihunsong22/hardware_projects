/*
================================================================================
Function : Operation for SI446x
网址：http://shop57165217.taobao.com
作者：李勇  原  亿和电子工作室  现 亿佰特电子科技有限公司
TEL：18615799380, e-mail: yihe_liyong@126.com
================================================================================
*/
#ifndef _BOARD_H_
#define _BOARD_H_

#include "iom8v.h"
#include "mytypedef.h"

/*Exchange a byte via the SPI bus*/
INT8U SPI_ExchangeByte( INT8U input );

/*Initialize the SPI bus*/
void SPI_Initial( void );

/*Initialize the other GPIOs of the board*/
void GPIO_Initial( void );

/*turn on the LED*/
#define LED_On( )       {PORTD &= ~0x03; DDRD |= 0x03; }

/*turn off the LED*/
#define LED_Off( )      {PORTD |= 0x03; DDRD |= 0x03; }

/*toggle the LED*/
#define LED_Toggle( )   {PORTD ^= 0x03; DDRD |= 0x03; }


//--------------operations for OLED module------------------------
#define OLED_CSN_H( )   {PORTB |= ( 1<<1 ); DDRB |= ( 1<<1 ); }
#define OLED_CSN_L( )   {PORTB &= ~( 1<<1 ); DDRB |= ( 1<<1 ); }

#define OLED_CMD_H( )   {PORTD |= ( 1<<7 ); DDRD |= ( 1<<7 ); }
#define OLED_CMD_L( )   {PORTD &= ~( 1<<7 ); DDRD |= ( 1<<7 ); }

#define OLED_RST_H( )   {PORTD |= ( 1<<6 ); DDRD |= ( 1<<6 ); }
#define OLED_RST_L( )   {PORTD &= ~( 1<<6 ); DDRD |= ( 1<<6 ); }

#define L01_CSN_LOW( )      {PORTB &= ~( 1<<2 ); DDRB |= ( 1<<2 ); }
#define L01_CSN_HIGH( )     {PORTB |= ( 1<<2 ); DDRB |= ( 1<<2 ); }
#define L01_CE_LOW( )       {PORTB &= ~( 1<<7 ); DDRB |= ( 1<<7 ); }
#define L01_CE_HIGH( )      {PORTB |= ( 1<<7 ); DDRB |= ( 1<<7 ); }




#endif //_BOARD_H_
/*
=================================================================================
------------------------------------End of FILE----------------------------------
=================================================================================
*/
