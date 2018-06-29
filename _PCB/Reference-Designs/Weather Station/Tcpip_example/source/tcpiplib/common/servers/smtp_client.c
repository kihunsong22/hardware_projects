/*************************************************************************
**
**  VERSION CONTROL:	@(#)smtp_client.c	1.2	03/10/31
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Demo application: basic SMTP client
**
**************************************************************************/

#include <string.h>

#include "smtp_client.h"
#include "common/tcpip.h"

#ifndef OPTION_SMTP_CLIENT
#ifndef TCPIPDEBUG
#error OPTION_SMTP_CLIENT is not defined, but smtp_client.c is included in project
#endif
#endif

#ifdef OPTION_SMTP_CLIENT

//**************************************************************************

#ifdef DEBUG_SMTP
#define debug_smtp(x) x
#else
#define debug_smtp(x)
#endif

/***************************************************************************
 * FUNCTION:	smtp_client
 *
 * SMTP client application.
 * This function is used as TCP-callback when opening a session to a SMTP
 * server. The cargo has to be initialized when the session is created
 * to point to the start of a SMTP client script.
 * (See the demo for a sample script.)
 *
 * resend	if non-zero the last packet has to be regenerated
 * session	active TCP session
 * datalength	number of incoming bytes
 * flags	pointer to flags in TCP-header
 *
 * returns number of bytes answered
 */
Uint16 CALLBACKMEMSPEC smtp_client(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags)
{
    SMTP_CLIENT_CARGO *cargo = (SMTP_CLIENT_CARGO*) &session->cargo.smtp_client; // cast only needed to keep some compilers from complaining
    debug_smtp(datalength ? (printf("SMTP_CLIENT received: "), printchars(TCPDATA, datalength), printf("\n")) : NULL);

    if (*tcp_flags & TCP_FLAG_RESET)
    {
	return 0;
    }


    if ((session->state != TCP_STATE_SYNACK) && (session->state != TCP_STATE_CONNECTED))
    {
	// no action, just ignore (probably a reset by the server)
	return 0;
    }

    if ((resend == 0) && (session->state != TCP_STATE_SYNACK))
    {
	// not a retry and not the initial send: move to next scriptline
	++(cargo->nextmsg);
    }

    if (!cargo->nextmsg->msg)
    {
	// finished, server should close the session
	return 0;
    }

    // if our script defined a check verify the incoming data
    if (cargo->nextmsg->check)
    {
	if (datalength == 0)
	{
	    // ignore incoming packet if no data received but a check was defined,
	    // move back to previous scriptline
	    --(cargo->nextmsg);
	    return 0;
	}
	else if (strncmp(TCPDATA, cargo->nextmsg->check, strlen(cargo->nextmsg->check)) != 0)
	{
	    // incoming packet does not match expected response, close session
	    *tcp_flags |= TCP_FLAG_FIN;
	    return 0;
	}
    }

    // copy our scripted answer into the TCP data buffer
    strcpy(TCPDATA, (cargo->nextmsg)->msg);
    strcat_rom(TCPDATA, "\r\n");

    debug_smtp((printf("SMTP_CLIENT send: "), printchars(TCPDATA, strlen(TCPDATA)), printf("\n")));

    // all data from our script always fits into a single TCP packet, push it
    *tcp_flags |= TCP_FLAG_PUSH;

    return strlen(TCPDATA);
}

//*************************************************************************

#endif // OPTION_SMTP_CLIENT
