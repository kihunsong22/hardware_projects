/*************************************************************************
**
**  VERSION CONTROL:	@(#)smtp_client.h	1.2	03/10/31
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Demo application: basic SMTP client
**
**************************************************************************/

#include "tcpipset.h"
#include "common/tcpip_global.h"

#ifndef _SMTP_CLIENT_H_
#define _SMTP_CLIENT_H_

//**************************************************************************

// format of our communicationscript for the SMPT server
typedef struct
{
    char *check;	// optional response-string we expect from the server
    char *msg;	// text we are sending back if the check went OK
}
SMTP_MSG;


// map a convenient structure over the cargo bytes in the session
// to keep track of our position in the SMTP script
typedef struct
{
    SMTP_MSG *nextmsg;
}
SMTP_CLIENT_CARGO;

extern Uint16 CALLBACKMEMSPEC smtp_client(Sint8 resend, TCP_SESSION * session,
                                          Uint16 datalength, Uint8 * tcp_flags);

#endif
