/*****************************************************************************
 *
 *	VERSION:	%W%	%E%
 *
 *      IN PACKAGE:	TASKING Peripheral Library for 80515
 *
 *      AUTHORS:        SAPA
 *
 *      COPYRIGHT:	Copyright (c) 2003, Altium BV
 *
 *      DESCRIPTION:	Interface to the UART module
 *			Do not include this file directly! The user should 
 *			create an uart-configuration headerfile that describes 
 *			the uart's configuration, followed by inclusion of 
 *			this file. 
 *			Usually tealib.h includes the user defined config 
 *			file, so including tealib.h in enough to gain access 
 *			to all library functions (including those of other 
 *			peripherals).
 *
 ****************************************************************************/


/*
 * Perform some checking first
 */
#if (!defined UART_RECEIVER_ON) && (!defined UART_TRANSMITTER_ON)
#error > This does not make any sense, now does it?
#endif

#ifndef FOSC
#error > Please define a macro 'FOSC' that represents the oscillator frequency.
#error >   Something like:   #define FOSC   12000000L
#endif

#if (UART_CHANNEL > 1)
#error > Value UART_CHANNEL is not valid. UART_CHANNEL can be either 0 or 1.
#endif

#if (UART_TBUF > 65535)
#error > A transmitbuffer of more than 65535 bytes is not supported.
#endif

#if (UART_RBUF > 65535)
#error > A receivebuffer of more than 65535 bytes is not supported.
#endif

#ifndef UART_TRANSMITTER_ON
#undef UART_TBUF						/* Ignore UART_TBUF */
#endif

#ifndef UART_RECEIVER_ON
#undef UART_RBUF						/* Ignore UART_RBUF */
#endif



/*
 * Define macros to default if undefined
 */
#ifndef UART_CHANNEL
#define UART_CHANNEL	0					/* default serial channel */
#endif

#ifndef UART_TBUF
#define UART_TBUF	0					/* default use one-byte buffer for transmitter */
#endif

#ifndef UART_RBUF
#define UART_RBUF	0					/* default use one-byte buffer for receiver */
#endif

#ifndef UART_BUF_MEMSPACE
#define UART_BUF_MEMSPACE	_data				/* default use direct addressable internal dataspace */
#endif




/* Undefine module control helper definitions. */
#undef UART_CIRCULAR_TBUF
#undef UART_CIRCULAR_RBUF
#undef UART_INTERRUPTDRIVEN
#undef PARITY_BIT_INVOLVED
#undef UART_ISR_VECTOR


/*
 * Implementation rules:
 * - Buffers > 0 are 'circular'
 * - If circular buffers are used, the serial driver becomes interrupt driven.
 * - Parity checking code is omitted when UART_PARMS is defined as UART_N81
 */
#if (UART_TBUF != 0)
#define UART_CIRCULAR_TBUF					/* Circular transmitbuffer is used */
#endif

#if (UART_RBUF != 0)
#define UART_CIRCULAR_RBUF					/* Circular receivebuffer is used */
#endif

#if ( (defined UART_CIRCULAR_TBUF) || (defined UART_CIRCULAR_RBUF) )
#define UART_INTERRUPTDRIVEN					/* Interrupt driven implementation */
#endif

#if (UART_PARMS != UART_N81)
#define PARITY_BIT_INVOLVED					/* Parity checking code is involved */
#endif


/*
 * Define macros to default if undefined
 */
#ifdef UART_INTERRUPTDRIVEN
#if (UART_CHANNEL == 0)
#define UART_ISR_VECTOR		0x0023				/* interrupt vector for channel 0 */
#else
#define UART_ISR_VECTOR		0x0083				/* interrupt vector for channel 1 */
#endif
#endif

#ifdef UART_INTERRUPTDRIVEN
#ifndef UART_ISR_PRIORITY
#define UART_ISR_PRIORITY	0				/* default priority for serial interrupt service routine */
#endif
#if ((UART_ISR_PRIORITY < 0) || (UART_ISR_PRIORITY > 3))
#error > Invalid value specified for UART_ISR_PRIORITY. Must be in range [0 .. 3]
#endif
#endif

#ifdef UART_INTERRUPTDRIVEN
#if ((UART_ISR_REGBANK < 0) || (UART_ISR_REGBANK > 3))
#error > Invalid value specified for UART_ISR_REGBANK. Must be in range [0 .. 3]
#endif
#endif



/*
 * Build names of external uart functions, depending on defined UART_CHANNEL
 */
#define GLUE_FUNCTION_NAME(ch, function)	uart ## ch ## function
#define FUNCTION(ch, function)			GLUE_FUNCTION_NAME( ch, function )

#define UART_INIT		FUNCTION( UART_CHANNEL, _init )
#define UART_CLOSE		FUNCTION( UART_CHANNEL, _close )
#define UART_PUT		FUNCTION( UART_CHANNEL, _put )
#define UART_TBUF_FULL		FUNCTION( UART_CHANNEL, _tbuf_full )
#define UART_GET		FUNCTION( UART_CHANNEL, _get )
#define UART_RBUF_EMPTY		FUNCTION( UART_CHANNEL, _rbuf_empty )
#define UART_RECV_ERRORBYTE	FUNCTION( UART_CHANNEL, _recv_errorbyte )
#define UART_RECV_BUFOVERFLOW	FUNCTION( UART_CHANNEL, _recv_bufoverflow )
#define UART_FLUSH		FUNCTION( UART_CHANNEL, _flush )

/*
 * Function prototypes
 */
#ifndef UART_PARMS
#ifndef UART_BAUDRATE
extern char		UART_INIT( unsigned short baudrate, unsigned char parms ) ;
#else
extern char		UART_INIT( unsigned char parms ) ;
#endif
#else
#ifndef UART_BAUDRATE
extern char		UART_INIT( unsigned short baudrate ) ;
#else
extern char		UART_INIT( void );
#endif
#endif


extern char		UART_CLOSE( void ) ;


#ifdef UART_TRANSMITTER_ON
extern void 		UART_PUT( unsigned char byte );
#ifdef UART_INTERRUPTDRIVEN
extern char		UART_TBUF_FULL( void );
#endif
#endif


#ifdef UART_RECEIVER_ON
extern unsigned char	UART_GET( void );
extern char		UART_RBUF_EMPTY( void );
#ifndef UART_IGN_ERRBYTE
extern char		UART_RECV_ERRORBYTE ( void );
#endif
#ifdef UART_INTERRUPTDRIVEN
extern char		UART_RECV_BUFOVERFLOW ( void );
#endif
#endif


#ifdef UART_INTERRUPTDRIVEN
extern void		UART_FLUSH( char type );
#endif
