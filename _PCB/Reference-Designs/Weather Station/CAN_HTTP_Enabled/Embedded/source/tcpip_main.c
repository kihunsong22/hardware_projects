/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	IP-over-CAN enabled weather station
**                 	Interface between main program and TCPIP library
**
**************************************************************************/

#include "tcpipset.h"

#include "tcpip_main.h"

#include "common/tcpip.h"
#include "common/servers/http_server.h"
#include "tealib/timer0.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//**************************************************************************

// placeholders for ipnumbers refered to in tcpip_settings
static Uint8 self_ip[] = SELF_IP;

// list of available TCP servers
static TCP_SERVER tcpservers[] = {
    {TCP_PORT_HTTP, http_server},
    {0, NULL}
};

static void CALLBACKMEMSPEC serial_statuschange_callback(Uint16 state);

static int timer = 0;

//**************************************************************************

void tcpipmain_init(void)
{
    // fill some settings
    tcpip_settings.tcpservers = tcpservers;
    memcpy(tcpip_settings.self_ip, self_ip, 4);

    tcpipstack_init();
}


int tcpipmain(void)
{
    // call timer entry of tcpip stack every second
    if ((tmr0_getclock() / 100) != timer)
    {
        timer = tmr0_getclock() / 100;
        tcpipstack_timertick();
        return 1;
    }

    return tcpipstack_main();
}

void tcpipmain_setstation(int stationnr)
{
    canip_setstation(stationnr);
}

//*****************************************************************************
