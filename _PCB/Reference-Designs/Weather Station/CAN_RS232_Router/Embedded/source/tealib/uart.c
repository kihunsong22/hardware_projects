/*****************************************************************************
 *
 *  VERSION:    %W% %E%
 *
 *      IN PACKAGE: TASKING Peripheral Library for C8051
 *
 *      AUTHORS:        SAPA
 *
 *      COPYRIGHT:  Copyright (c) 2003, Altium BV
 *
 *      DESCRIPTION:    Interface to the UART module. Usually this source
 *          module is NOT added to your project directly, but
 *          included in another C file that includes the uart
 *          configuration file, followed by the inclusion of this
 *          file.
 *
 *
 ****************************************************************************/

#ifdef TEST_ON_PHILIPS_66X
#define T2CON   (*(__bsfr volatile unsigned char *)0xC8)
#endif



#undef UART_PRESCALER
#undef UART_TIMERRESOLUTION
#ifndef TEST_ON_PHILIPS_66X
#define UART_PRESCALER      384
#define UART_TIMERRESOLUTION    256
#else
#define UART_PRESCALER      192
#define UART_TIMERRESOLUTION    256
#endif




/*
 * Include module headerfile
 */
#include "uart.h"


#if (UART_CHANNEL > 0)
#error > The C8051 has only one serial channel!
#endif



/*
 * Build names of serial registers depending on defined UART_CHANNEL
 */
#define GLUE_SFR_NAME(ch, reg)          S ## ch ## reg
#define SFR(ch, reg)                GLUE_SFR_NAME( ch, reg )

#define SFR_SCON    SFR( UART_CHANNEL, CON )        /* Control register */
#define SFR_SBUF    SFR( UART_CHANNEL, BUF )        /* Transmit and receive register */
#define SFR_TMOD    TMOD                    /* Timer mode control register for timer 1 */
#define SFR_TH1     TH1                 /* Timer 1 functions as baudrate generator */

// rename some sfr registers, since the C8051 uses different names for channel 0 registers
#define S0CON       SCON
#define S0BUF       SBUF

// In the new 'Coral' toolset no macros are predefined for bitaddressable sfr's
#if 1 /* bitaddressable sfr's for the C8051 core */
#define SET_RI0     RI = 1;
#define CLR_RI0     RI = 0;
#define GET_RI0     RI
#define SET_TI0     TI = 1;
#define CLR_TI0     TI = 0;
#define GET_TI0     TI
#define SET_ES0     ES = 1;
#define CLR_ES0     ES = 0;
#define GET_ES0     ES
#define SET_RB80    RB8 = 1;
#define CLR_RB80    RB8 = 0;
#define GET_RB80    RB8
#define SET_TB80    TB8 = 1;
#define CLR_TB80    TB8 = 0;
#define GET_TB80    TB8
#define SET_REN0    REN = 1;
#define CLR_REN0    REN = 0;
#define GET_REN0    REN
#endif


/*
 * Build sfr-bit names and define get/set/clr macros for access to these bits.
 */
#define GLUE_SFRBIT_NAME(ch, bit)       bit ## ch
#define SFRBIT(ch, bit)             GLUE_SFRBIT_NAME( ch, bit )

#define GLUE_SET_SFRBIT_NAME(sfrbit)        SET ## sfrbit
#define GLUE_CLR_SFRBIT_NAME(sfrbit)        CLR ## sfrbit
#define GLUE_GET_SFRBIT_NAME(sfrbit)        GET ## sfrbit
#define SET_SFRBIT(sfrbit)          GLUE_SET_SFRBIT_NAME( sfrbit )
#define CLR_SFRBIT(sfrbit)          GLUE_CLR_SFRBIT_NAME( sfrbit )
#define GET_SFRBIT(sfrbit)          GLUE_GET_SFRBIT_NAME( sfrbit )

#define SFR_BIT_RI  SFRBIT( UART_CHANNEL, _RI )     /* Receive interrupt flag */
#define SFR_BIT_TI  SFRBIT( UART_CHANNEL, _TI )     /* Transmit interrupt flag */
#define SFR_BIT_ES  SFRBIT( UART_CHANNEL, _ES )     /* Serial interrupt enable bit */
#define SFR_BIT_TB8 SFRBIT( UART_CHANNEL, _TB8 )        /* 9th transmit bit */
#define SFR_BIT_RB8 SFRBIT( UART_CHANNEL, _RB8 )        /* 9th receive bit */


/*
 * Define masks for non-bitaddressable registers
 */
#define SFR_IPx_MASK    0x10                    /* Mask for the IP register, priority bit for channel 0 */


/*
 * Static data
 */
static volatile unsigned char uart_mode = UART_N81;     /* Holds the current serial mode */
#if ( (!defined UART_IGN_ERRBYTE) && (defined UART_RECEIVER_ON) )
static volatile unsigned char errbyte_dropped   = 0;        /* Signals when an error byte has been dropped */
#endif
#if ( (defined UART_INTERRUPTDRIVEN) && (defined UART_RECEIVER_ON) )
static volatile unsigned char byte_lost     = 0;        /* Signals when a byte was lost due to a buffer overflow */
#endif

#if ( (defined UART_INTERRUPTDRIVEN) && (defined UART_TRANSMITTER_ON) )
static volatile unsigned char tpump_running = 0;        /* If 0, the transmission pump must be (re)started */
#endif

#if ( (defined UART_CIRCULAR_TBUF) && (defined UART_TRANSMITTER_ON) )
#if (UART_TBUF <= 255)
static volatile unsigned char tbuf_head     = 0;
static volatile unsigned char tbuf_tail     = 0;
#else
static volatile unsigned int tbuf_head      = 0;
static volatile unsigned int tbuf_tail      = 0;
#endif
#endif

#if ( (defined UART_CIRCULAR_RBUF) && (defined UART_RECEIVER_ON) )
#if (UART_RBUF <= 255)
static volatile unsigned char rbuf_head     = 0;
static volatile unsigned char rbuf_tail     = 0;
#else
static volatile unsigned int rbuf_head      = 0;
static volatile unsigned int rbuf_tail      = 0;
#endif
#endif

volatile unsigned int rbuf_inuse        = 0;


/*
 * Memory allocation:
 * - If interrupt driven, allocate memory for (circular) buffers. If not interrupt driven: no memory or flags is required.
 * - If a buffer is not circular (size = 1), an extra buffer status flag is declared.
 */
#ifdef UART_INTERRUPTDRIVEN
#ifdef UART_TRANSMITTER_ON
#if (UART_TBUF <= 255)
static UART_BUF_MEMSPACE unsigned char tbuf[UART_TBUF + 1]; /* Allocate memory for transmitbuffer */
#else
static UART_BUF_MEMSPACE unsigned int tbuf[UART_TBUF + 1];  /* Allocate memory for transmitbuffer */
#endif
#ifndef UART_CIRCULAR_TBUF
static unsigned char transmit_buffer_full = 0;          /* Need a flag for one-byte buffer */
#endif
#endif

#ifdef UART_INTERRUPTDRIVEN
#ifdef UART_RECEIVER_ON
#if (UART_RBUF <= 255)
static UART_BUF_MEMSPACE unsigned char rbuf[UART_RBUF + 1]; /* Allocate memory for receivebuffer */
#else
static UART_BUF_MEMSPACE unsigned int rbuf[UART_RBUF + 1];  /* Allocate memory for receivebuffer */
#endif
#ifndef UART_CIRCULAR_RBUF
static unsigned char receiver_buffer_empty = 1;         /* Need a flag for one-byte buffer */
#endif
#endif
#else
/*
 * If not interrupt driven: no memory or flags is required.
 * Direct read/write from/to SBUF.
 */
#endif
#endif



/*
 * Inline helper routines (some may be omitted if not required)
 */
#ifdef UART_INTERRUPTDRIVEN
/*****************************************************************************
 *
 *  FUNCTION:   disable_isr
 *
 *  AVAILABILITY:   LOCAL   (only when UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   None
 *
 *  DESCRIPTION:    Enables serial channel transmit and receive interrupt.
 *          A pending interrupt will be processed immediately.
 */
inline void enable_isr ( void )
{
    SET_SFRBIT ( SFR_BIT_ES );
}
#endif

#ifdef UART_INTERRUPTDRIVEN
/*****************************************************************************
 *
 *  FUNCTION:   disable_isr
 *
 *  AVAILABILITY:   LOCAL   (only when UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   None
 *
 *  DESCRIPTION:    Disables serial channel transmit and receive interrupt.
 *          A pending interrupt flag is not cleared.
 */
inline void disable_isr ( void )
{
    CLR_SFRBIT ( SFR_BIT_ES );
}
#endif

#ifdef PARITY_BIT_INVOLVED
/*****************************************************************************
 *
 *  FUNCTION:   set_9th_bit
 *
 *  AVAILABILITY:   LOCAL   (only when PARITY_BIT_INVOLVED)
 *
 *  PARAMETERS: unsigned char   val
 *
 *  RETURN VALUE:   None
 *
 *  DESCRIPTION:    Sets the 9th bit (paritybit) to 'val'.
 *          This 9th bit will be transmitted when the uart is
 *          configured as a 9 bit uart.
 *
 *          Used by transmit routines only when parity is involved
 *          (thus in UART_O81 or UART_E81 mode).
 */
inline void set_9th_bit ( unsigned char val )
{
    TB8 = val;
}
#endif


#ifndef UART_IGN_ERRBYTE
/*****************************************************************************
 *
 *  FUNCTION:   get_9th_bit
 *
 *  AVAILABILITY:   LOCAL   (only when UART_IGN_ERRBYTE)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   unsigned char
 *
 *  DESCRIPTION:    Returns the 9th bit (paritybit or stopbit) of the last
 *          received asynchronous data frame.
 *
 *          Used by receive routines to perform a parity check or
 *          stopbit check on a received byte.
 */
inline unsigned char get_9th_bit ( void )
{
    return GET_SFRBIT ( SFR_BIT_RB8 );
}
#endif


#ifdef PARITY_BIT_INVOLVED
/*****************************************************************************
 *
 *  FUNCTION:   is_parity_odd
 *
 *  AVAILABILITY:   LOCAL   (only when PARITY_BIT_INVOLVED)
 *
 *  PARAMETERS: unsigned char   byte
 *
 *  RETURN VALUE:   unsigned char
 *
 *  DESCRIPTION:    Returns 1 if number of 1's in 'byte' is odd.
 *          Returns 0 if number of 1's in 'byte' is even.
 *
 *          Used by receive routines to perform a parity check
 *          on a received byte.
 *          Also used by transmit routines to calculate the paritybit
 *          for the byte to transmit (in UART_O81 or UART_E81 mode only).
 */
inline unsigned char is_parity_odd ( unsigned char byte )
{
    unsigned char mask;

    mask = byte >> 4;
    byte = byte ^ mask;

    mask = byte >> 2;
    byte = byte ^ mask;

    mask = byte >> 1;
    byte = byte ^ mask;

    return (byte & 0x01);
}
#endif

/*****************************************************************************
 *
 *  FUNCTION:   UART_INIT
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: unsigned short  baudrate
 *          unsigned char   parms
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:
 *
 *      The header for UART_INIT depends on the macro's UART_BAUDRATE
 *      and UART_PARMS:
 *      - If UART_BAUDRATE is defined, the argument baudrate is omitted.
 *         UART_BAUDRATE is used instead.
 *      - If UART_PARMS is defined, the argument parms is omitted.
 *         UART_PARMS is used instead.
 *
 *      Initializes the serial channel in the following steps:
 *      1) Close serial channel (turn off interrupts).
 *      2) Operation mode valid ?
 *      3) Calculate values for baudrate generator's reload registers ('baudrate').
 *          If requested baudrate is too low, return with -1.
 *      4) Configure and enable baudrate generator.
 *      5) Set register SCON according to the desired mode ('parms').
 *      6) Initialize static variables (like bufferpointers and control flags).
 *      7) Configure the interrupt system (if interrupt driven).
 *      8) Enable the interrupt system (if interrupt driven).
 *      9) Done. Return with 0.
 *          
 *      It is allowed to call uart_init again for reconfiguration purposes,
 *      without the need of calling UART_CLOSE first.
 */
#ifndef UART_PARMS
#ifndef UART_BAUDRATE
char UART_INIT( unsigned short baudrate, unsigned char parms )
{
    unsigned int reload;
#else
char UART_INIT( unsigned char parms )
{
#endif
#else
#ifndef UART_BAUDRATE
char UART_INIT( unsigned short baudrate )
{
    long reload;
#else
char UART_INIT( void )
{
#endif
#endif

/* 1) Close serial channel */
    UART_CLOSE();

#ifndef UART_PARMS
    uart_mode = parms;                  /* Save the current operation mode */
#else
    uart_mode = UART_PARMS;                 /* Save the current operation mode */
#endif

/* 2) Operation mode valid ? */
    if (uart_mode > UART_O81)
    {
        return -1;
    }

/* 3+4) Calculate values for baudrate generator's reload registers ('baudrate').
 *      If requested baudrate is too low, return with -1.
 *      Note that serial channel 0 is more suitable for baudrates > 4800 (@ 12MHz).
 */

/* Calculate reload value for timer 1 (acts as baudrate generator) */
#ifndef UART_BAUDRATE
    reload = UART_TIMERRESOLUTION - (((2 * FOSC / UART_PRESCALER) + (baudrate / 2)) / baudrate);
    if (reload < 0)
    {
        // requested baudrate too low: recalculate with SMOD cleared.
        reload = UART_TIMERRESOLUTION - (((FOSC / UART_PRESCALER) + (baudrate / 2)) / baudrate);

        PCON &= 0x7F;                   /* clear SMOD bit */
    }
    else
    {
        // requested baudrate possible with SMOD set.
        PCON |= 0x80;                   /* set SMOD bit */
    }

    if (reload < 0)
    {
        return -1;                  /* Baudrate is too low for UART 0*/
    }

/* The requested baudrate can be accomplished */
    SFR_TH1 = reload;
#else
#if ( (((2 * FOSC / UART_PRESCALER) + (UART_BAUDRATE / 2)) / UART_BAUDRATE) > UART_TIMERRESOLUTION) /* Calc reload with SMOD = 1 */
#if ( (((1 * FOSC / UART_PRESCALER) + (UART_BAUDRATE / 2)) / UART_BAUDRATE) > UART_TIMERRESOLUTION) /* Calc reload with SMOD = 0 */
#error > Requested baudrate too low
#else
/* With SMOD = 0, the requested baudrate can be accomplished */
    PCON &= 0x7F;                       /* clear SMOD bit */
    SFR_TH1 = (UART_TIMERRESOLUTION - (((1 * FOSC / UART_PRESCALER) + (UART_BAUDRATE / 2)) / UART_BAUDRATE));
#endif
#else
/* With SMOD = 1, the requested baudrate can be accomplished */
    PCON |= 0x80;                       /* set SMOD bit */
    SFR_TH1 = (UART_TIMERRESOLUTION - (((2 * FOSC / UART_PRESCALER) + (UART_BAUDRATE / 2)) / UART_BAUDRATE));
#endif
#endif

#ifdef TEST_ON_PHILIPS_66X
    T2CON   &= 0xCF;                    /* Select timer 1 for baudrate gen. */
#endif
    SFR_TMOD = 0x20 | (SFR_TMOD & 0x0F);            /* Set timer 1 in auto reload mode (but don't touch timer 0) */
    TR1 = 1;                        /* Start timer 1 */


/* 5) Configure serial channel, according to parms */
    if (uart_mode == UART_N81)
    {
#ifdef UART_RECEIVER_ON
        SFR_SCON = 0x50;                /* Serial mode 1 (8 bit uart), receiver enabled, flags cleared */
#else
        SFR_SCON = 0x40;                /* Serial mode 1 (8 bit uart), receiver disabled, flags cleared */
#endif
    }
    else
    {
#ifdef UART_RECEIVER_ON
        SFR_SCON = 0xD0;                /* Serial mode 3 (9 bit uart), receiver enabled, flags cleared */
#else
        SFR_SCON = 0xC0;                /* Serial mode 3 (9 bit uart), receiver disabled, flags cleared */
#endif
    }




/* 6) Initialize static variables */
#if ( (defined UART_INTERRUPTDRIVEN) && (defined UART_TRANSMITTER_ON) )
    tpump_running   = 0;
#endif
#if ( (defined UART_CIRCULAR_TBUF) && (defined UART_TRANSMITTER_ON) )
    tbuf_head   = 0;
    tbuf_tail   = 0;
#endif
#if ( (defined UART_CIRCULAR_RBUF) && (defined UART_RECEIVER_ON) )
    rbuf_head   = 0;
    rbuf_tail   = 0;
#endif
#if ( (!defined UART_IGN_ERRBYTE) && (defined UART_RECEIVER_ON) )
    errbyte_dropped = 0;
#endif
#if ( (defined UART_INTERRUPTDRIVEN) && (defined UART_RECEIVER_ON) )
    byte_lost   = 0;
#endif


#ifdef UART_INTERRUPTDRIVEN
/* 7) Configure the interrupt system. Only two levels supported */
    IP &= ~SFR_IPx_MASK;
#if (UART_ISR_PRIORITY > 0)
    IP0 |= SFR_IPx_MASK;
#endif
#endif

        // watch out: RXD output is connected to RTS!
        UART_RTS = 0;

#ifdef UART_INTERRUPTDRIVEN
/* 8) Enable the interrupt system */
    enable_isr();                       /* Enable serial interupts */
    EA = 1;                         /* Enable all interrupts */
#endif
    return 0;                       /* Return with success */
}


/*****************************************************************************
 *
 *  FUNCTION:   UART_CLOSE
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Closes the serial channel:
 *          - Disable serial interrupts.
 *          - Turn off the serial channel.
 *          - Turn off the baudrate generator (if possible) in
 *            order to reduce power consumption.
 *
 *          The global interrupt enable flag is not cleared in
 *          favor of other peripherals that might use interrupts.
 */
char UART_CLOSE( void )
{
#ifdef UART_INTERRUPTDRIVEN
    disable_isr();                      /* Disable serial interupts */
#endif

    SFR_SCON = 0x00;                    /* Turn off serial channel */
    
    return 0;
}



#if ( (defined UART_TRANSMITTER_ON) && (defined UART_INTERRUPTDRIVEN) )
/*****************************************************************************
 *
 *  FUNCTION:   UART_TBUF_FULL
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_TRANSMITTER_ON and UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Returns 1 when the transmit buffer is 100% full.
 *          0 if there is space available.
 */
char UART_TBUF_FULL( void )
{
#ifdef UART_CIRCULAR_TBUF
#if (UART_TBUF <= 255)
    unsigned char nextpos;
#else
    unsigned int nextpos;
#endif
    nextpos = tbuf_head + 1;
    if (nextpos > UART_TBUF)
    {
        nextpos = 0;    // wrap around
    }
    return nextpos == tbuf_tail;
#else
    return transmit_buffer_full;
#endif
}
#endif




#ifdef UART_TRANSMITTER_ON
/*****************************************************************************
 *
 *  FUNCTION:   UART_PUT
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_TRANSMITTER_ON)
 *
 *  PARAMETERS: unsigned char   byte
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    In interrupt mode: Wait for space in the transmit buffer,
 *          then places 'byte' in the transmit queue and return.
 *          If the transmission-pump has stopped, it will we started
 *          by setting the TI flag.
 *
 *          In polling mode: Transmit 'byte' immediately and wait
 *          until the byte has been transmitted.
 */
void UART_PUT( unsigned char byte )
{
#ifdef UART_INTERRUPTDRIVEN
#ifdef UART_CIRCULAR_TBUF
#if (UART_TBUF <= 255)
    unsigned char nextpos;
#else
    unsigned int nextpos;
#endif
    nextpos = tbuf_head+1;
    if (nextpos > UART_TBUF)
    {
        nextpos = 0;    // wrap around
    }

    while (nextpos == tbuf_tail)                /* Wait until there is space available in buffer */
        ;
    tbuf[tbuf_head] = byte;                 /* Put byte in buffer */

    tbuf_head = nextpos;

    if (!tpump_running)
    {
        SET_SFRBIT ( SFR_BIT_TI );          /* Start the transmit pump! */
    }
#else   /* UART_CIRCULAR_TBUF */
    while (transmit_buffer_full)                /* Wait until the one-byte buffer becomes empty */
        ;

    tbuf[0] = byte;                     /* Put byte in one-byte buffer */
    transmit_buffer_full = 1;
    if (!tpump_running)
    {
        SET_SFRBIT ( SFR_BIT_TI );          /* Start the transmit pump! */
    }
#endif
#else   /* UART_INTERRUPTDRIVEN */
#ifdef PARITY_BIT_INVOLVED
    unsigned char isodd;

    if (uart_mode != UART_N81)
    {
        isodd = is_parity_odd ( byte );
// If the byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

        set_9th_bit(isodd ^ (uart_mode == UART_O81));
    }
#endif
    SFR_SBUF = byte;                    /* Transmit the byte immediately */

    while (!GET_SFRBIT ( SFR_BIT_TI ))          /* Wait until it actually has left the uart */
        ;
    CLR_SFRBIT ( SFR_BIT_TI );              /* Clear TI flag, to signal an empty 'buffer' */
#endif
}
#endif




#ifdef UART_RECEIVER_ON
/*****************************************************************************
 *
 *  FUNCTION:   UART_RBUF_EMPTY
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_RECEIVER_ON)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Returns 1 when the receive buffer is empty. 0 is
 *          returned if it contains any data.
 */
char UART_RBUF_EMPTY( void )
{
#ifdef UART_INTERRUPTDRIVEN
#ifdef UART_CIRCULAR_RBUF
    return rbuf_tail == rbuf_head;
#else
    return receiver_buffer_empty;
#endif
#else   /* UART_INTERRUPTDRIVEN */
    return (!GET_SFRBIT ( SFR_BIT_RI ));            /* Buffer is empty when receive-flag RI is 0 */
#endif
}
#endif


#ifdef UART_RECEIVER_ON
/*****************************************************************************
 *
 *  FUNCTION:   UART_GET
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_RECEIVER_ON)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Returns the byte received. If no byte is available,
 *          we'll wait until we have at least one (valid) byte.
 *
 *          In polling mode, and with UART_IGN_ERRBYTE not defined,
 *          each received byte is tested for errors. Errorneous
 *          bytes will be dropped.
 *          In interrupt driven mode, the UART_ISR performs the
 *          error checks, so we simply wait for a not empty receive
 *          buffer.
 *
 *          User should use UART_RBUF_EMPTY to test if data is available.
 */
unsigned char UART_GET( void )
{
    unsigned char byte;

#ifdef UART_INTERRUPTDRIVEN
#ifdef UART_CIRCULAR_RBUF
    while (rbuf_head == rbuf_tail)
        ;

    disable_isr();              // Enter critical section

        // watch out: RXD output is connected to RTS!
    --rbuf_inuse;
        if (rbuf_inuse > (UART_RBUF - UART_RBUF_RTS))
        { UART_RTS = 1; }
        else
        { UART_RTS = 0; }

        byte = rbuf[rbuf_tail++];
    if (rbuf_tail > UART_RBUF)
    {
        rbuf_tail = 0;  // wrap around
    }
    enable_isr();               // Leave critical section
#else
// get byte from one-byte buffer
    while (receiver_buffer_empty)
        ;
#ifndef UART_IGN_BUFOVR
    disable_isr();              // Enter critical section (only when testing on receiver_buffer_empty in uart_isr)
#endif
    byte = rbuf[0];
    receiver_buffer_empty = 1;
#ifndef UART_IGN_BUFOVR
    enable_isr();               // Leave critical section (only when testing on receiver_buffer_empty in uart_isr)
#endif
#endif
#else   /* UART_INTERRUPTDRIVEN */
#ifdef UART_IGN_ERRBYTE
    while (!GET_SFRBIT ( SFR_BIT_RI ))
        ;
    byte = SFR_SBUF;
    CLR_SFRBIT ( SFR_BIT_RI );
#else
    unsigned char valid;
#ifdef PARITY_BIT_INVOLVED
    unsigned char isodd;
#endif

    do
    {
        while (!GET_SFRBIT ( SFR_BIT_RI ))
            ;
        byte = SFR_SBUF;
        CLR_SFRBIT ( SFR_BIT_RI );
#ifdef PARITY_BIT_INVOLVED
#ifndef UART_PARMS
        if (uart_mode == UART_N81)
        {
            valid = (get_9th_bit() == 1);   // valid when 9th bit is 1 (stop bit)
        }
        else
#endif
        {
            isodd = is_parity_odd ( byte );
// If the received byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

            isodd ^= get_9th_bit();

// - in ODD  mode, the received byte is valid when (isodd == 1)
// - in EVEN mode, the received byte is valid when (isodd != 1)
            (uart_mode == UART_O81) ? (valid = isodd) : (valid = !isodd);
        }
#else
        valid = (get_9th_bit() == 1);       // valid when 9th bit is 1 (stop bit)
#endif
        if (!valid)
        {
            errbyte_dropped = 1;
        }
    }
    while (!valid);
#endif
#endif
    return byte;
}
#endif



#ifdef UART_INTERRUPTDRIVEN
/*****************************************************************************
 *
 *  FUNCTION:   UART_FLUSH
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: char    Type
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Flushes receiver buffer and/or transmitbuffer
 *          acoording to 'type':
 *           UART_RFLUSH    the receive buffer is emptied.
 *           UART_TFLUSH    the transmit buffer is emptied.
 *           UART_TDRAIN    wait until all data in the transmit
 *                  buffer has been sent.
 *
 *          UART_RFLUSH can be OR'd with UART_TFLUSH/UART_TDRAIN
 */
void UART_FLUSH( char type )
{
//enum { UART_TDRAIN = 0x01, UART_TFLUSH = 0x03, UART_RFLUSH = 0x04 } ;

#if ( (defined UART_RECEIVER_ON) && (defined UART_CIRCULAR_RBUF) )
    if ( type & UART_RFLUSH )
    {
// cleans the receive buffer
        disable_isr();          // Enter critical section
        rbuf_head = 0;
        rbuf_tail = 0;
        enable_isr();           // Leave critical section
    }
#endif

#if ( (defined UART_TRANSMITTER_ON) && (defined UART_CIRCULAR_TBUF) )
    if (( type & UART_TFLUSH ) == UART_TFLUSH)
    {
// throw away data that might not have been sent
// if the transmit pump is running, it will be stopped automatically due to the emptied buffer.
        disable_isr();          // Enter critical section
        tbuf_head = 0;
        tbuf_tail = 0;
        enable_isr();           // Leave critical section
    }
    else if ( type & UART_TDRAIN )
    {
// wait for transmit buffer to become empty
        if (tpump_running)      // Dead lock security: Don't enter while-loop when pump is not running
        {
            while (tbuf_head != tbuf_tail)
                ;
        }
    }
#endif
}
#endif



#if ( (!defined UART_IGN_ERRBYTE) && (defined UART_RECEIVER_ON) )
/*****************************************************************************
 *
 *  FUNCTION:   UART_RECV_ERRORBYTE
 *
 *  AVAILABILITY:   GLOBAL  (only when not UART_IGN_ERRBYTE and UART_RECEIVER_ON)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Returns 1 if previously an errorneous received byte
 *          has been dropped. Returns 0 otherwise.
 *          The errbyte_dropped-flag is cleared after it is read.
 */
char UART_RECV_ERRORBYTE ( void )
{
    unsigned char val;
#ifdef UART_INTERRUPTDRIVEN
    disable_isr();              // Enter critical section
#endif
    val = errbyte_dropped;
    errbyte_dropped = 0;
#ifdef UART_INTERRUPTDRIVEN
    enable_isr();               // Leave critical section
#endif
    return val;
}
#endif


#if ( (defined UART_RECEIVER_ON) && (defined UART_INTERRUPTDRIVEN) )
/*****************************************************************************
 *
 *  FUNCTION:   UART_RECV_BUFOVERFLOW
 *
 *  AVAILABILITY:   GLOBAL  (only when UART_RECEIVER_ON and UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   char
 *
 *  DESCRIPTION:    Returns 1 if previously a received byte could not be
 *          stored in the receive buffer because it was full.
 *          Returns 0 otherwise.
 *          The byte_lost-flag is cleared after it is read.
 */
char UART_RECV_BUFOVERFLOW ( void )
{
    unsigned char val;
    disable_isr();              // Enter critical section
    val = byte_lost;
    byte_lost = 0;
    enable_isr();               // Leave critical section
    return val;
}
#endif


#ifdef UART_INTERRUPTDRIVEN
/* Build name for serial channel interrupt service routine */
#define UART_ISR    FUNCTION( UART_CHANNEL, _isr )

/*****************************************************************************
 *
 *  FUNCTION:   UART_ISR
 *
 *  AVAILABILITY:   LOCAL   (only when UART_INTERRUPTDRIVEN)
 *
 *  PARAMETERS: None
 *
 *  RETURN VALUE:   None
 *
 *  DESCRIPTION:    Handles both the transmit and receive interrupt.
 *
 *          When the TI flag was set, the UART is ready to transmit
 *          the next byte, taken from the transmitbuffer (if any).
 *
 *          When the RI flag was set, a byte has been received. If
 *          UART_IGN_ERRBYTE is not defined, the received byte is
 *          checked for errors. When valid, the byte is stored in
 *          the receivebuffer. If this buffer is full, the byte can
 *          either be dropped (UART_IGN_BUFOVR not defined), or
 *          overwrite the oldest data in the buffer (UART_IGN_BUFOVR defined).
 */
#ifdef UART_ISR_REGBANK
__interrupt (UART_ISR_VECTOR) __using (UART_ISR_REGBANK) void UART_ISR ( void )
#else
__interrupt (UART_ISR_VECTOR) void UART_ISR ( void )
#endif
{
#ifdef PARITY_BIT_INVOLVED
    unsigned char isodd;
#endif

#ifdef UART_RECEIVER_ON
    unsigned char recvbyte;
#ifndef UART_IGN_ERRBYTE
    unsigned char valid;
#endif
#ifdef UART_CIRCULAR_RBUF
#if (UART_RBUF <= 255)
    unsigned char nextpos;
#else
    unsigned int nextpos;
#endif
#endif
#endif

P3_2 = 1;
/* Interrupt caused by receiver ? */
    if (GET_SFRBIT ( SFR_BIT_RI ))
    {
#ifdef UART_RECEIVER_ON
        recvbyte = SFR_SBUF;
#ifdef UART_CIRCULAR_RBUF
                nextpos = rbuf_head+1;
        if (nextpos > UART_RBUF)
        {
            nextpos = 0;    // wrap around
        }
#ifdef UART_IGN_ERRBYTE
#ifndef UART_IGN_BUFOVR
        if (nextpos != rbuf_tail)   // only fill buffer when not full
        {
            rbuf[rbuf_head] = recvbyte;
            rbuf_head = nextpos;
                ++rbuf_inuse;
        }
        else
        {
        // byte lost
            byte_lost = 1;
        }
#else
        rbuf[rbuf_head] = recvbyte;
        rbuf_head = nextpos;
        ++rbuf_inuse;

// if buffer is full, we've just overwritten the oldest byte and need to adjust the tailpointer
// byte_lost signals we've lost the oldest byte in the buffer
        if (nextpos == rbuf_tail)
        {
            rbuf_tail = nextpos;
            byte_lost = 1;
                --rbuf_inuse;
        }
#endif


#else   /* UART_IGN_ERRBYTE */



#ifndef UART_IGN_BUFOVR
        if (nextpos != rbuf_tail)   // only fill buffer when not full
        {
#endif
// drop received byte if invalid
#ifdef PARITY_BIT_INVOLVED
#ifndef UART_PARMS
        if (uart_mode == UART_N81)
        {
            valid = (get_9th_bit() == 1);   // valid when 9th bit is 1 (stop bit)
        }
        else
#endif
        {
            isodd = is_parity_odd ( recvbyte );
// If the received byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

            isodd ^= get_9th_bit();

// - in ODD  mode, the received byte is valid when (isodd == 1)
// - in EVEN mode, the received byte is valid when (isodd != 1)
            (uart_mode == UART_O81) ? (valid = isodd) : (valid = !isodd);
        }
#else   /* PARITY_BIT_INVOLVED */
        valid = (get_9th_bit() == 1);   // valid when 9th bit is 1 (stop bit)
#endif
        if (valid)
        {
            rbuf[rbuf_head] = recvbyte;
            rbuf_head = nextpos;

#ifdef UART_IGN_BUFOVR
// if buffer is full, we've just overwritten the oldest byte and need to adjust the tailpointer
// byte_lost signals we've lost the oldest byte in the buffer
            if (nextpos == rbuf_tail)
            {
                rbuf_tail = nextpos;
                byte_lost = 1;
            }
#endif
        }
        else
        {
            errbyte_dropped = 1;
        }
#ifndef UART_IGN_BUFOVR
        }
        else
        {
        // byte lost
            byte_lost = 1;
        }
#endif




#endif

#else   /* UART_CIRCULAR_RBUF */

// put received byte in one-byte buffer

#ifdef UART_IGN_ERRBYTE
#ifndef UART_IGN_BUFOVR
        if (receiver_buffer_empty)  // only fill buffer when empty
        {
#endif
        rbuf[0] = recvbyte;
        receiver_buffer_empty = 0;  // signal we have something
#ifndef UART_IGN_BUFOVR
        }
        else
        {
        // byte lost
            byte_lost = 1;
        }
#endif
#else   /* UART_IGN_ERRBYTE */
#ifndef UART_IGN_BUFOVR
        if (receiver_buffer_empty)  // only fill buffer when empty
        {
#endif
// drop received byte if invalid
#ifdef PARITY_BIT_INVOLVED
#ifndef UART_PARMS
        if (uart_mode == UART_N81)
        {
            valid = (get_9th_bit() == 1);   // valid when 9th bit is 1 (stop bit)
        }
        else
#endif
        {
            isodd = is_parity_odd ( recvbyte );
// If the received byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

            isodd ^= get_9th_bit();

// - in ODD  mode, the received byte is valid when (isodd == 1)
// - in EVEN mode, the received byte is valid when (isodd != 1)
            (uart_mode == UART_O81) ? (valid = isodd) : (valid = !isodd);
        }

#else
        valid = (get_9th_bit() == 1);   // valid when 9th bit is 1 (stop bit)
#endif
        if (valid)
        {
            rbuf[0] = recvbyte;
            receiver_buffer_empty = 0;  // signal we have something
        }
        else
        {
            errbyte_dropped = 1;
        }
#ifndef UART_IGN_BUFOVR
        }
        else
        {
        // byte lost
            byte_lost = 1;
        }
#endif

#endif

#endif
#endif
        CLR_SFRBIT ( SFR_BIT_RI );

        // watch out: RXD output is connected to RTS!
                if (rbuf_inuse > (UART_RBUF - UART_RBUF_RTS))
                { UART_RTS = 1; }
                else
                { UART_RTS = 0; }
    }


/* Interrupt caused by tranmitter ? */
    if (GET_SFRBIT ( SFR_BIT_TI ))
    {
        CLR_SFRBIT ( SFR_BIT_TI );
#ifdef UART_TRANSMITTER_ON
#ifdef UART_CIRCULAR_TBUF
// transmit new byte from circular buffer

        if (tbuf_tail == tbuf_head)
        {
            tpump_running = 0;
        }
        else
        {
#ifdef PARITY_BIT_INVOLVED
            if (uart_mode != UART_N81)
            {
                isodd = is_parity_odd ( tbuf[tbuf_tail] );
// If the byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

                set_9th_bit(isodd ^ (uart_mode == UART_O81));
            }
#endif

/* Transmit the byte */
            SFR_SBUF = tbuf[tbuf_tail++];
            if (tbuf_tail > UART_TBUF)
            {
                tbuf_tail = 0;  // wrap around
            }
            tpump_running = 1;
        }


#else   /* UART_CIRCULAR_TBUF */
// transmit new byte from one-byte buffer
        if (transmit_buffer_full == 0)
        {
            tpump_running = 0;
        }

        if (transmit_buffer_full)
        {
#ifdef PARITY_BIT_INVOLVED
            if (uart_mode != UART_N81)
            {
                isodd = is_parity_odd ( tbuf[0] );
// If the byte is odd:
// - in ODD  mode, the 9th bit should be 0 to keep it odd
// - in EVEN mode, the 9th bit should be 1 to make it even

                set_9th_bit(isodd ^ (uart_mode == UART_O81));
            }
#endif

/* Transmit the byte */
            SFR_SBUF = tbuf[0];
            transmit_buffer_full = 0;
            tpump_running = 1;
        }
#endif
#endif
    }

}
#endif
