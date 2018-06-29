/*************************************************************************
**
**  VERSION CONTROL:    $Revision:   1.6  $
**          $Date:   Mar 20 2003 15:20:46  $
**
**  IN PACKAGE:     Embedded TCPIP
**
**  COPYRIGHT:      Copyright (c) 2002 Altium
**
**  DESCRIPTION:    hardware dependent functions for 8051fpga
**
**************************************************************************/

#include <string.h>

// tcpipset.h should include sys_8051fpga.h and sys.h
#include "tcpipset.h"

#include "tealib\uart0.h"

//**************************************************************************


void comm_put(Uint8 c)
{
    uart0_put(c);
}


void comm_putstr(char *s)
{
    while (*s)
    {
        comm_put(*s++);
    }
}


#ifndef ROMMEMSPEC_AUTOCAST
void comm_putstr_rom(ROMMEMSPEC char *s)
{
    while (*s)
    {
        comm_put(*s++);
    }
}
#endif


void comm_flush(void)
{
   uart0_flush(UART_TDRAIN);

}


Uint8 comm_get(Uint8 * c)
{
    if (uart0_rbuf_empty())
    {
        return 0;
    }

    *c = uart0_get();
    return 1;
}


void comm_init(void)
{
    uart0_init();
}


//**************************************************************************
