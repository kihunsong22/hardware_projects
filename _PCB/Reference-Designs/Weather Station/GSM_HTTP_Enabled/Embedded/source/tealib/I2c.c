/*****************************************************************************\
|*
|*  VERSION CONTROL:  $Revision: 1 $
|*                    $Date: 3/22/00 2:01p $
|*
|*  IN PACKAGE:       TASKING I2C Application note
|*
|*  COPYRIGHT:        Copyright (c) 2000 TASKING, Inc.
|*
|*  DESCRIPTION:      Provide I2C functions
|*
|*  REMARKS:          Change the settings in i2c_hw.h to suit you hardware
|*
\*****************************************************************************/

#include "i2c_hw.h"
#include "i2c.h"

/*
 * Private definitions
 */

#define READ(X)     ((X) | 0x01)
#define WRITE(X)    ((X) & 0xFE)
 
/*
 * Private function definitions
 */

static void I2C_delay( Uint16 usec );
static void I2C_start( void );
static I2C_ERR I2C_stop( void );
static void I2C_put( Uint8 val );
static Uint8 I2C_get( void );
static Uint8 I2C_ack( Uint8 val );


/*****************************************************************************\
|*
|*  FUNCTION:         I2C_delay
|*
|*  AVAILABILITY:     LOCAL
|*
|*  PARAMETERS:       None
|*
|*  RETURN VALUE:     None
|*
|*  DESCRIPTION:`     Delay for 10 usec
|*
 */

void I2C_delay( Uint16 usec )
{
    while( usec-- )
        __asm( "nop" );
}


/*****************************************************************************\
|*
|*  FUNCTION:         I2C_start
|*
|*  AVAILABILITY:     STATIC
|*
|*  PARAMETERS:       None
|*
|*  RETURN VALUE:     None
|*
|*  DESCRIPTION:`     Generate a start condition
|*
 */

static void I2C_start( void )
{
    SDA = HIGH ;
    SCL = HIGH ;
    I2C_delay( 5 );
    SDA = LOW;    /* Data going low while clock is still high */
    I2C_delay( 1 );
    SCL = LOW;
    I2C_delay( 5 );
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_stop
|*
|*  AVAILABILITY:     STATIC
|*
|*  PARAMETERS:       None
|*
|*  RETURN VALUE:     None
|*
|*  DESCRIPTION:      Generate a stop condition
|*
 */

static I2C_ERR I2C_stop( void )
{
    Uint16  i;
    SCL = LOW;
    SDA = LOW;
    I2C_delay( 10 );
    SCL = HIGH;

    for (i = 0; i < 2000; i++)
    {
        I2C_delay( 10 );
        if ( SCL == HIGH ) break;
    }
    SDA = HIGH;   /* Data goes high while clock is already high: stop */
    for ( i = 0; i < 2000; i++ )
    {
        I2C_delay( 10 );
        if ( SDA == HIGH ) break;
    }
    return ( SCL == HIGH && SDA == HIGH ) ? I2C_OK : I2C_TOUT;
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_put
|*
|*  AVAILABILITY:     STATIC
|*
|*  PARAMETERS:       val = Value to write
|*
|*  RETURN VALUE:     None
|*
|*  DESCRIPTION:      Clock a single byte to the I2C bus
|*
 */

static void I2C_put( Uint8 val )
{
    Uint8   bit;

    for ( bit = 0x80; bit; bit >>= 1 )
    {
        SDA = (val & bit) ? HIGH : LOW;
        I2C_delay( 1 );
        SCL = HIGH;
        I2C_delay( 9 );
        SCL = LOW;
        I2C_delay( 10 );
    }
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_get
|*
|*  AVAILABILITY:     STATIC
|*
|*  PARAMETERS:       None
|*
|*  RETURN VALUE:     Value read
|*
|*  DESCRIPTION:      Read a single byte from the I2C bus
|*
 */

static Uint8 I2C_get( void )
{
    Uint8   val = 0;
    Uint8   bit;

    SDA = HIGH;
    for ( bit = 0x80; bit; bit >>= 1 )
    {
        SCL = HIGH;
        I2C_delay( 8 );
        if ( SDA ) val |= bit;
        I2C_delay( 1 );
        SCL = LOW;
        I2C_delay( 10 );
    }
    return val;
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_ack
|*
|*  AVAILABILITY:     STATIC
|*
|*  PARAMETERS:       ack = value to set (or read)
|*
|*  RETURN VALUE:     actual value from acknowledge
|*
|*  DESCRIPTION:      Get or set an acknowledge
|*
 */

static Uint8 I2C_ack( Uint8 ack )
{
    SDA = ack;
    __asm( "nop" );
    SCL = HIGH;
    I2C_delay( 8 );
    if ( ack ) ack = SDA;
    I2C_delay( 1 );
    SCL = LOW;
    I2C_delay( 10 );
    return ack;
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_init
|*
|*  AVAILABILITY:     GLOBAL
|*
|*  PARAMETERS:       None
|*
|*  RETURN VALUE:     None
|*
|*  DESCRIPTION:      Initialize the I2C subsystem
|*
 */

I2C_ERR I2C_init( void )
{
    Uint8   i;

    /*
     * Initialize the lines.
     */

    SDA = HIGH;        
    SCL = HIGH;

    /*
     * 9 dummy clocks to make sure that any pending communication is gone
     */
     
    for (i = 0; i < 9; i++)
    {
        SCL = LOW;
        if ( SDA == HIGH )
        {
            SDA = LOW;    /* Drive data low, thus preparing for stop condition */
            I2C_delay( 10 );
            SCL = HIGH;         /* Clock should go high now */
            SDA = HIGH;   /* Followed by data: stop condition */
        }
        else
        {
            I2C_delay( 10 );
            SCL = HIGH;
        }
        I2C_delay( 10 );
    }
    return ( SDA == HIGH && SCL == HIGH ) ? I2C_OK : I2C_ARB;
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_write
|*
|*  REQUIREMENTS:     The I2C bus must have been initialized by means of I2C_init()
|*
|*  AVAILABILITY:     GLOBAL
|*
|*  PARAMETERS:       slave = Device address
|*                    buf   = Pointer to data to be written
|*                    size  = Number of bytes to be written
|*
|*  RETURN VALUE:     status of action
|*
|*  DESCRIPTION:      Write a number of bytes to the I2C device
|*
 */

I2C_ERR I2C_write( Uint8 slave, const Uint8 *buf, Uint16 size )
{
    I2C_start();
    I2C_put( WRITE( slave ));

    if ( I2C_ack( HIGH ) == LOW )
    {
       while ( size )
       {
          I2C_put( *buf++ );
          size--;
          if ( I2C_ack( HIGH ) == HIGH )
          {
             break;
          }
       } 
    }

    I2C_stop();
    return size ? I2C_ARB : I2C_OK;
}

/*****************************************************************************\
|*
|*  FUNCTION:         I2C_read
|*
|*  AVAILABILITY:     GLOBAL
|*
|*  REQUIREMENTS:     The I2C bus must have been initialized by means of I2C_init()
|*
|*  PARAMETERS:       slave = Device address
|*                    buf   = Pointer to data to be written
|*                    size  = Number of bytes to be written
|*
|*  RETURN VALUE:     status of action
|*
|*  DESCRIPTION:      Read a number of bytes from the I2C device
|*
 */

I2C_ERR I2C_read( Uint8 slave, Uint8 *buf, Uint16 size )
{
    Uint16  state = 3;
    Uint8   setack = HIGH;
    Uint8   expect = LOW;

    I2C_start();
    I2C_put( READ( slave ));

    if ( I2C_ack( setack ) == expect )
    {
       while ( size )
       {
          *buf++ = I2C_get();
          setack = --size ? LOW : HIGH ;
          expect = setack;
          if ( I2C_ack( setack ) != expect )
          {
             break;
          }
       }
    }
    I2C_stop();
    return size ? I2C_ARB : I2C_OK;
}
