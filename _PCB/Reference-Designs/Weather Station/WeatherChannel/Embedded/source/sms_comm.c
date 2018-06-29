/*************************************************************************
**
**  VERSION CONTROL:
**
**  IN PACKAGE:     APS SMS engine
**
**  COPYRIGHT:      Copyright (c) 2003, Altium BV
**
**  DESCRIPTION:    SERIAL COMMUNUCATION
**
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tealib/timer0.h"
#include "tealib/uart0.h"
#include "sms.h"
#include "sms_comm.h"
void uart_puts( const char *str );


/***********************************************************************************
 *
 *  FUNCTION:       uart_puts
 *
 *  ENVIRONMENT:    str,        pointer to string
 *
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    Send string to uart
 *
 */

void uart_puts( const char *str )
{
    while( * str )
    {
       uart0_put( *str++ );
    }
}

/***********************************************************************************
 *
 *  FUNCTION:       exec_cmd
 *
 *  ENVIRONMENT:    cmd,        command to pass to the modem
 *          response,   room to store modem response
 *          timeout_sec,    timeout in seconds
 *          expected,   expected answer (or phrase)
 *
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    execute specified command and wait until the expected answer
 *          was received or a timeout has occured.
 *
 */

void exec_cmd(const char * cmd, char * response, int timeout_sec, const char * expected, char
 new_line)
{
    TMR_TIMER timer;
    TMR_TIMER comm_timer;
    DWORD answer=0;
    char c;
    char resp_ok = 0;

    P3_2 = 0;
    /* write the command to the communications port */
    if (new_line==1)
    {
        uart0_put( '\r' );
    }
    uart_puts(cmd);

    timer = settimeout( timeout_sec * TMR0_TICKS_PER_SECOND );

    /* get answer, as long as data is received or timeout has occured */

    comm_timer = tmr0_settimeout( 100 );
    do
    {
        if ( !uart0_rbuf_empty() )
        {
            c = uart0_get();
            comm_timer = tmr0_settimeout( 10 );
            response[answer++] = c;
            response[answer] = '\0';
            // Must check for answer, if it's there, we can stop at end of line
            if ( strstr( response, expected ) != 0 )
            {
                P3_2 = 1;
                resp_ok = 1;
            }
        }
        else if ( resp_ok == 1 )
        {
            if ( tmr0_expired( comm_timer ))
            {
                break;       // We're done!
            }
        }

    } while( ! tmr0_expired( timer ));
    response[answer] = '\0';
}

