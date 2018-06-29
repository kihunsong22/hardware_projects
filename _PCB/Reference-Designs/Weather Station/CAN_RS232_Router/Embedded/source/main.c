/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:        CAN-RS232-Router
**                      main
**
**************************************************************************/

#include "tcpipset.h"

#include "common/tcpip.h"
#include "common/dns.h"
#include "common/servers/http_server.h"
#include "common/servers/ftp_server.h"
#include "common/drivers/serial_driver.h"
#include "common/filesystem/filesys.h"
#include "common/filesystem/diskdrv.h"
#include "tealib/timer0.h"
#include "tealib/lcd0.h"

#include "back.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//**************************************************************************

#define SELF_IP             {192, 168, 100, 1}  // our IP
#define DNS_SELF_NAME       "weatherdemo.test"     // name for us a caller can use
#define PPP_IN_USER         "user"
#define PPP_IN_PASSWORD     "password"
#define PPP_CLIENT_IP       {192, 168, 100, 2}  // IP we give our client
#define MODEM_INIT          "ATE0M1L1&K0&R1\r"

//**************************************************************************

// CGI exported vars

//**************************************************************************

// placeholders for ipnumbers refered to in tcpip_settings
static Uint8 self_ip[] = SELF_IP;
static Uint8 ppp_client_ip[] = PPP_CLIENT_IP;
static char dns_self_name[] = DNS_SELF_NAME;

// list of available TCP servers
static TCP_SERVER tcpservers[] = {
    {TCP_PORT_HTTP, http_server},
    {TCP_PORT_FTP, ftp_server},
    {0, NULL}
};

// list of available UDP clients & servers
static UDP_CLIENTSERVER udpclientservers[] = {
    {UDP_PORT_DNS_SERVER, dns_server},
    {0, NULL}
};


static void CALLBACKMEMSPEC serial_statuschange_callback(Uint16 state);
static Uint8 CALLBACKMEMSPEC ppp_usercheck_callback(char *user, char *password);
static Uint8 CALLBACKMEMSPEC ftp_usercheck_callback(char *user, char *password, char *rootdir);


//unsigned char diskimg[FS_DISKSIZE];
static char disklabel[] = "Disk";

static void lcd_puts(__rom char *s)
{
    while (*s)
    {
        lcd_putc(*s++);
    }
}


void main(void)
{
    int timer = 0;

    lcd_init();

    lcd_puts("Init....");

    // fill some settings
    tcpip_settings.tcpservers = tcpservers;
    tcpip_settings.udpclientservers = udpclientservers;
    memcpy(tcpip_settings.self_ip, self_ip, 4);
    tcpip_settings.dns_self_name = dns_self_name;
    tcpip_settings.serial_statuschange = &serial_statuschange_callback;
    memcpy(tcpip_settings.ppp_client_ip, ppp_client_ip, 4);
    tcpip_settings.ppp_usercheck = &ppp_usercheck_callback;
    tcpip_settings.ftp_server_usercheck = &ftp_usercheck_callback;

    sys_init();

    drv_clear(0, FS_DISKSIZE);
    fs_format(disklabel);

    tcpipstack_init();

    back_init();

    lcd_gotoxy(0, 0);
    lcd_puts("Weatherstation");
    lcd_gotoxy(0, 1);
    lcd_puts("CAN-RS232 Router");

    for (;;)
    {
        if (serial_bufempty())
        {
            // no data in outgoing buffer,
            // see if backend wants to route anything to the front
            while(back_main())
            {
            }
        }

        if (tcpipstack_main() == 0)
        {
           // stack did nothing
        }

    // call timer entry of tcpip stack every second
    if ((tmr0_getclock() / 1000) != timer)
    {
        timer = tmr0_getclock() / 1000;
            tcpipstack_timertick();
        }
    }
}


Uint16 ip_routing(void)
{
    back_framebuf_send(((Uint8*)&IP->destip)[3] - 3, IPBUF, ntohw(IP->packetlength));

    return 0;
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



//*****************************************************************************

