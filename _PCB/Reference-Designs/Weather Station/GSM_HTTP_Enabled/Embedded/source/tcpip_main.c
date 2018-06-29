/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    IP-over-RS232 enabled weather station
**                  Interface between main program and TCPIP library
**
**************************************************************************/

#include "tcpipset.h"

#include "tcpip_main.h"

#include "common/tcpip.h"
#include "common/dns.h"
#include "common/servers/http_server.h"
#include "tealib/timer0.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//**************************************************************************

// placeholders for ipnumbers refered to in tcpip_settings
static Uint8 self_ip[] = SELF_IP;
static Uint8 ppp_client_ip[] = PPP_CLIENT_IP;
static char dns_self_name[] = DNS_SELF_NAME;
static char modem_init_string[] = MODEM_INIT;

// list of available TCP servers
static TCP_SERVER tcpservers[] = {
    {TCP_PORT_HTTP, http_server},
//    {TCP_PORT_FTP, ftp_server},
    {0, NULL}
};

// list of available UDP clients & servers
static UDP_CLIENTSERVER udpclientservers[] = {
    {UDP_PORT_DNS_SERVER, dns_server},
    {0, NULL}
};


static void CALLBACKMEMSPEC serial_statuschange_callback(Uint16 state);
static Uint8 CALLBACKMEMSPEC ppp_usercheck_callback(char *user, char *password);
//static Uint8 CALLBACKMEMSPEC ftp_usercheck_callback(char *user, char *password, char *rootdir);

//**************************************************************************

static int timer = 0;

void tcpipmain_init(void)
{
    // fill some settings
    tcpip_settings.tcpservers = tcpservers;
    tcpip_settings.udpclientservers = udpclientservers;
    memcpy(tcpip_settings.self_ip, self_ip, 4);
    tcpip_settings.dns_self_name = dns_self_name;
    tcpip_settings.serial_statuschange = &serial_statuschange_callback;
    memcpy(tcpip_settings.ppp_client_ip, ppp_client_ip, 4);
    tcpip_settings.ppp_usercheck = &ppp_usercheck_callback;
//    tcpip_settings.ftp_server_usercheck = &ftp_usercheck_callback;
    tcpip_settings.modem_init = modem_init_string;

    tcpipstack_init();
}


int tcpipmain(void)
{
    // call timer entry of tcpip stack every second
    if ((tmr0_getclock() / 1000) != timer)
    {
        timer = tmr0_getclock() / 1000;
        tcpipstack_timertick();
        return 1;
    }

    return tcpipstack_main();
}


/***************************************************************************
 * FUNCTION:    serial_statuschange_callback
 *
 * This function is called by the line control part of the TCPIP stack
 * whenever a major state change occurs.
 */
static void serial_statuschange_callback(Uint16 state)
{
    static Uint8 connected = 0;

    switch (state)
    {
    case LINESTATE_IDLE:
    if (connected)
    {
        tcp_resetall();
        connected = 0;
    }
    break;

    case LINESTATE_CALLIN_PPPCONNECTED:
    connected = 1;
    break;

    }
}


/***************************************************************************
 * FUNCTION: ppp_usercheck_callback
 *
 * This funcion is called by the line/ppp module to verify PAP user/pwd
 * combinations when someone calls in. It should return non-zero when ok,
 * otherwise 0.
 */
static Uint8 ppp_usercheck_callback(char *user, char *password)
{
    if (strcmp_rom(user, PPP_IN_USER))
    {
    return 0;
    }

    if (strcmp_rom(password, PPP_IN_PASSWORD))
    {
    return 0;
    }

    return 1;
}


/*
static Uint8 ftp_usercheck_callback(char *user, char *password, char *rootdir)
{
    strcpy_rom(rootdir, "/");

    if (strcmp_rom(user, PPP_IN_USER))
    {
    return 0;
    }

    if (strcmp_rom(password, PPP_IN_PASSWORD))
    {
    return 0;
    }

    return 1;
}
*/


//*****************************************************************************
