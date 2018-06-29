/*************************************************************************
**
**  VERSION CONTROL:    %W% %T%
**
**  IN PACKAGE:     Embedded TCPIP
**
**  COPYRIGHT:      Copyright (c) 2003 Altium
**
**  DESCRIPTION:    8051 demo of TCPIP stack
**
**************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "tcpipset.h"
#include "common/tcpip.h"
#include "common/tcpip_global.h"

#ifdef OPTION_DNS_SERVER
#include "common/dns.h"
#endif

#ifdef OPTION_SERIAL
#include "common/serial.h"
#endif

#ifdef OPTION_HTTP_SERVER
#include "common/servers/http_server.h"
#endif
#ifdef OPTION_TELNET_SERVER
#include "common/servers/telnet_server.h"
#endif
#ifdef OPTION_FTP_SERVER
#include "common/servers/ftp_server.h"
#endif
#ifdef OPTION_SMTP_CLIENT
#include "common/servers/smtp_client.h"
#endif

//**************************************************************************

// our static IP address
#define SELF_STATIC_IP      {192, 168, 100, 1}

// IP we give our client when they call us
#define PPP_CLIENT_IP       {192, 168, 100, 3}

// our name when they ask our DNS server about us
#define DNS_SELF_NAME       "embedded.test"

// string send once to modem upon init
#define MODEM_INIT      "ATE0M0L1&K0&R1\r"

// format: { username, password, commandprocessor, userid }
#define TELNET_USERS \
{ \
    {"root", "root", telnet_commandprocessor, 1}, \
    {"test", "test", telnet_commandprocessor, 101}, \
    {"user", "user", telnet_commandprocessor, 102}, \
    { 0, 0, 0} \
}

// format: { username, password, virtual_root }
#define FTP_USERS \
{ \
    {"root", "root", "/"}, \
    {"test", "test", "/test"}, \
    {"user", "user", 0}, \
    { 0, 0, 0} \
}

// IP address of the SMTP server
#define SMTP_SERVER_IP {192, 168, 100, 100}
#define SMTP_FROM "<dummy_from@dummy.test>"
#define SMTP_TO "<dummy_to@dummy.test>"
#define SMTP_HELO "dummy.test"

// script for our SMTP client
// format: { required_smtp_response_code, string_to_send }
#define SMTP_SCRIPT \
{   \
    {"2", "HELO " SMTP_HELO}, \
    {"2", "MAIL From: " SMTP_FROM}, \
    {"250", "RCPT To: " SMTP_TO}, \
    {"2", "DATA"}, \
    {"3", "From: " SMTP_FROM}, \
    {NULL, "Subject: Hello World"}, \
    {NULL, "This message was send to you by"}, \
    {NULL, "the TCPIP general demo..."}, \
    {NULL, "."}, \
    {"2", "QUIT"}, \
    {NULL, NULL} \
}

//**************************************************************************

// CGI exported vars
Uint16 temperature_threshold_cgi = 512;
Uint16 led_red_cgi = 0;
Uint16 led_yellow_cgi = 0;
Uint16 temperature_cgi = 512;
Uint16 ledarray_cgi = 0x55;
Uint16 seconds_cgi = 0;
Uint16 minutes_cgi = 0;
Uint16 hours_cgi = 0;
Uint16 days_cgi = 0;

//**************************************************************************

static Uint8 self_ip[] = SELF_STATIC_IP;

#ifdef SERIAL_PPP
#ifdef SERIAL_CALLIN
static Uint8 ppp_client_ip[] = PPP_CLIENT_IP;
#endif
#endif

#ifdef OPTION_DNS_SERVER
static char dns_self_name[] = DNS_SELF_NAME;
#endif

// list of available TCP servers
static TCP_SERVER tcpservers[] = {
#ifdef OPTION_HTTP_SERVER
    {TCP_PORT_HTTP, http_server},
#endif
#ifdef OPTION_TELNET_SERVER
    {TCP_PORT_TELNET, telnet_server},
#endif
#ifdef OPTION_FTP_SERVER
    {TCP_PORT_FTP, ftp_server},
#endif
    {0, NULL}
};

#ifdef OPTION_UDP
// list of available UDP clients & servers
static UDP_CLIENTSERVER udpclientservers[] = {
#ifdef OPTION_DNS_SERVER
    {UDP_PORT_DNS_SERVER, dns_server},
#endif
    {0, NULL}
};
#endif

#ifdef OPTION_TELNET_SERVER
static CALLBACKMEMSPEC Uint8 telnet_commandprocessor(char *cmd, char *buf, char *user, Uint8 userid);
typedef struct
{
    char ROMMEMSPEC *user;
    char ROMMEMSPEC *password;
    Uint8 CALLBACKMEMSPEC(*commandprocessor) (char *cmd, char *buf, char *user, Uint8 userid);
    Uint8 userid;
}
TELNETUSER;
static TELNETUSER telnetusers[] = TELNET_USERS;
#endif

#ifdef OPTION_FTP_SERVER
typedef struct
{
    char ROMMEMSPEC *user;
    char ROMMEMSPEC *password;
    char ROMMEMSPEC *rootdir;

}
FTPUSER;
static FTPUSER ftpusers[] = FTP_USERS;
#endif

#ifdef OPTION_SERIAL
static void serial_statuschange_callback(Uint16 state);
#ifdef SERIAL_PPP
#ifdef SERIAL_CALLIN
static Uint8 ppp_usercheck_callback(char *user, char *password);
#endif
#endif
#endif
#ifdef OPTION_FTP_SERVER
static Uint8 ftp_server_usercheck_callback(char *user, char *password, char *rootdir);
#endif
#ifdef OPTION_TELNET_SERVER
static Uint8 CALLBACKMEMSPEC telnet_server_usercheck_callback(char *user, char *password,
							      Uint8 CALLBACKMEMSPEC(**commandprocessor) (char *cmd,
													 char *buf,
													 char *user,
													 Uint8 userid),
							      Uint8 * userid);
#endif

#ifdef OPTION_SMTP_CLIENT
static Uint8 smtp_server_ip[] = SMTP_SERVER_IP;
static SMTP_MSG smtp_script[] = SMTP_SCRIPT;
#endif

#include "common/drivers/serial_driver.h"

#include "tealib/timer0.h"
#include "tealib/keypad.h"

// FPGA watchdog
#define WD              P1_1

/***************************************************************************
 * FUNCTION:    main
 *
 * main of demo, initializes all TCPIP stuff and then enters a
 * neverending loop doing nothing but TCPIP calls.
 * A real application is simulated by changing a "temperature",
 * and a threshold. This threshold can be changed with a browser.
 * A (fake) yellow led is turned on or off based on the difference
 * betweem the temperature and the threshold.
 * A (fake) red led does nothing but can be toggled on or off
 * using the web-interface.
 * If you press any button on the keypad a message is send via an SMTP
 * server (first set all SMTP-defines above to sensible values)
 *
 * IMPORTANT: SMTP client can only work if you can reach the SMTP server,
 * and the SMTP server can reach you. This means using ipforwarding and
 * proxyarp under linux, or if you use windows then you have to ask
 * the sysadmin to set ipenablerouting on your machine and add a route
 * to your embedded device through your desktop.
 * In both cases you must ask for a valid unique static IP address to be
 * used for the SELF_STATIC_IP (the PPP_CLIENT_IP can be left as-is)
 *
 *              DON'T PLAY WITH THESE SETTINGS ON A LIVE NETWORK
 *                  IF YOU DON'T KNOW WHAT YOU ARE DOING!!!
 *
 * (building a direct PPP connection to the device without using SMTP is safe)
 */
void main(void)
{
    Sint16 temperature_step = 1;
    Sint16 lasttimer_tempemulator = 0;
    long lasttimer_tcpiptick = 0;
#ifdef OPTION_SMTP_CLIENT
    TCP_SESSION *smtp_session = NULL;
#endif

    sys_init();

    // fill some settings
    tcpip_settings.tcpservers = tcpservers;
#ifdef OPTION_UDP
    tcpip_settings.udpclientservers = udpclientservers;
#endif
#ifdef OPTION_DHCP
    memset(tcpip_settings.self_ip, 0, 4);
#else
    memcpy(tcpip_settings.self_ip, self_ip, 4);
#endif
#ifdef OPTION_DNS_SERVER
    tcpip_settings.dns_self_name = dns_self_name;
#endif
#ifdef OPTION_ETHERNET
    memcpy(tcpip_settings.mac, mac, 6);
#endif
#ifdef OPTION_SERIAL
    tcpip_settings.serial_statuschange = &serial_statuschange_callback;
#ifdef SERIAL_MODEM
    tcpip_settings.modem_init = MODEM_INIT;
#endif
#ifdef SERIAL_PPP
    memcpy(tcpip_settings.ppp_client_ip, ppp_client_ip, 4);
#ifdef SERIAL_CALLIN
    tcpip_settings.ppp_usercheck = &ppp_usercheck_callback;
#endif
#endif
#endif
#ifdef OPTION_FTP_SERVER
    tcpip_settings.ftp_server_usercheck = &ftp_server_usercheck_callback;
#endif
#ifdef OPTION_TELNET_SERVER
    tcpip_settings.telnet_server_usercheck = &telnet_server_usercheck_callback;
#endif

    tcpipstack_init();

    for (;;)
    {
	long sysclocktick = tmr0_getclock();


#ifdef OPTION_SMTP_CLIENT
        // no sophistication here: if any key pressed send an email
	if (!smtp_session && keypad())
	{
	    // open a TCP connection connected to our SMPT client
	    smtp_session = tcp_create(0, *(Uint32 *) smtp_server_ip, TCP_PORT_SMTP, &smtp_client);
	    if (smtp_session != NULL)
	    {
		// initialize SMTP client for this session
                // to start with the first line of our script
		smtp_session->cargo.smtp_client.nextmsg = smtp_script;
	    }
	}

	if (smtp_session)
	{
	    // check if a running SMTP session has finished
	    if ((smtp_session->state == TCP_STATE_LASTACK) || (smtp_session->state == TCP_STATE_CLOSED))
	    {
		// no need to track this session anymore
		smtp_session = NULL;
	    }
	}
#endif

	WD = 1;

	if ((sysclocktick / 5) != lasttimer_tempemulator)
	{
	    lasttimer_tempemulator = sysclocktick / 5;

	    // emulate of changing temperature
	    if (((temperature_cgi + temperature_step) > 1023) || (((int) temperature_cgi + temperature_step) < 0))
	    {
		temperature_step = -temperature_step;
	    }
	    temperature_cgi += temperature_step;
	}

	// temperature switch update
	led_yellow_cgi = (temperature_cgi > temperature_threshold_cgi) ? 0 : 1;

	// call timer entry of tcpip stack every second
	if ((sysclocktick / 100) != lasttimer_tcpiptick)
	{
	    lasttimer_tcpiptick = sysclocktick / 100;

	    // update the uptimer
	    if (++seconds_cgi >= 60)
	    {
		seconds_cgi = 0;
		if (++minutes_cgi >= 60)
		{
		    minutes_cgi = 0;
		    if (++hours_cgi >= 24)
		    {
			hours_cgi = 0;
			++days_cgi;
		    }
		}
	    }

	    tcpipstack_timertick();
	}

	WD = 0;

	// call main entry of tcpip stack as often as possible
	if (tcpipstack_main() == 0)
	{
	    // tcpip did nothing, we could yield some CPU power here
	}

#ifdef TCPIPDEBUG
	fflush(stdout);
#endif
    }
}


#ifdef OPTION_SERIAL

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
	// reset our IP address to be ready for callin
	memcpy(tcpip_settings.self_ip, self_ip, 4);
	if (connected)
	{
	    tcp_resetall();
	}
	connected = 0;
	break;

    case LINESTATE_CALLIN_PPPCONNECTED:
	connected = 1;
	break;

    case LINESTATE_CALLOUT_PPPCONNECTED:
	connected = 1;
	break;

    case LINESTATE_CALLIN:
	connected = 1;
	break;

    case LINESTATE_CALLOUT:
	connected = 1;

	break;

    }
}

#ifdef SERIAL_PPP

/***************************************************************************
 * FUNCTION: ppp_usercheck_callback
 *
 * This funcion is called by the line/ppp module to verify PAP user/pwd
 * combinations when someone calls in. It should return non-zero when ok,
 * otherwise 0.
 */
static Uint8 ppp_usercheck_callback(char *user, char *password)
{
    if (strcmp_rom(user, "user"))
    {
	return 0;
    }
    if (strcmp_rom(password, "password"))
    {
	return 0;
    }
    return 1;
}

#endif // SERIAL_PPP
#endif // OPTION_SERIAL

#ifdef OPTION_FTP_SERVER

/***************************************************************************
 * FUNCTION: ftp_server_usercheck_callback
 *
 * This function is called by the FTP server to verify user/pwd combinations.
 * It should return non-zero when ok, otherwise 0.
 * if ok the rootcwd should be filled with his rootdirectory for the user given
 */
static Uint8 ftp_server_usercheck_callback(char *user, char *password, char *rootdir)
{
    FTPUSER *ftpuser;

    for (ftpuser = ftpusers; ftpuser->user; ++ftpuser)
    {
	if (!strcmp_rom(user, ftpuser->user) && !strcmp_rom(password, ftpuser->password))
	{
	    if (ftpuser->rootdir)
	    {
		strcpy_rom(rootdir, ftpuser->rootdir);
	    }
	    return 1;
	}
    }

    if (!strcmp_rom(user, "anonymous"))
    {
	strcpy_rom(rootdir, "/");
	return 1;
    }

    return 0;
}
#endif


#ifdef OPTION_TELNET_SERVER
/***************************************************************************
 * FUNCTION: telnet_server_usercheck_callback
 *
 * This function is called by the TELNET server to verify user/pwd combinations.
 * It should return non-zero when ok, otherwise 0.
 * if ok the commandprocessor and userid should be filled with the relevant values
 * for the user concerned
 */
static Uint8 CALLBACKMEMSPEC telnet_server_usercheck_callback(char *user, char *password,
							      Uint8 CALLBACKMEMSPEC(**commandprocessor) (char *cmd,
													 char *buf,
													 char *user,
													 Uint8 userid),
							      Uint8 * userid)
{
    TELNETUSER *telnetuser;

    for (telnetuser = telnetusers; telnetuser->user; ++telnetuser)
    {
	if (!strcmp_rom(user, telnetuser->user) && !strcmp_rom(password, telnetuser->password))
	{
	    *commandprocessor = telnetuser->commandprocessor;
	    *userid = telnetuser->userid;
	    return 1;
	}
    }

    return 0;
}


/***************************************************************************
 * FUNCTION:    telnet_commandprocessor
 *
 * Example TELNET commandprocessor
 * Important: keep in mind te TELNET server as it is only can send a reply
 * fitting inside a single TCP/IP frame!
 *
 * cmd          commandbuffer to process
 * buf          pointer where to place answer
 * username     pointer to username
 * userid       usernumber
 *
 * returns 0 if session should end, <>0 otherwise
 */
static CALLBACKMEMSPEC Uint8 telnet_commandprocessor(char *cmd, char *buf, char *user, Uint8 userid)
{
    if (!*cmd)
    {
	// empty command, just ignore
    }
    else if (!strcmp_rom(cmd, "quit"))
    {
	// end session
	return 0;
    }
    else if (!strcmp_rom(cmd, "help"))
    {
	strcat_rom(buf, "Usage:\r\n"
		   "help - show this text\r\n"
                   "whoami - show user information\r\n"
                   "quit - end session\r\n");
    }
    else if (!strcmp_rom(cmd, "whoami"))
    {
	strcat_rom(buf, "Current user is: ");
	strcat(buf, user);
	if (userid == 1)
	{
	    strcat_rom(buf, " (superuser)\r\n");;
	}
	else
	{
	    strcat_rom(buf, " (just an ordinary user)\r\n");;
	}
    }
    else
    {
	strcat_rom(buf, "Unknown command (use 'help')\r\n");
    }

    // continue session
    return 1;
}

#endif // OPTION_TELNET_SERVER

//*****************************************************************************
