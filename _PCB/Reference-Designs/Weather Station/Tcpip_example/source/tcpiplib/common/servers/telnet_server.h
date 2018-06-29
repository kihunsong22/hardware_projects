/*************************************************************************
**
**  VERSION CONTROL:	@(#)telnet_server.h	1.3	03/12/17
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Demo application: simple TELNET server
**
**************************************************************************/

#include "tcpipset.h"
#include "common/tcpip_global.h"

#ifndef _TELNET_SERVER_H_
#define _TELNET_SERVER_H_

//**************************************************************************

#ifndef TELNET_SESSIONS
#define TELNET_SESSIONS 2
#endif

#ifndef TELNET_BUFLENGTH
#define TELNET_BUFLENGTH 12
#endif

//**************************************************************************

#define TELNET_CMD_SE       240
#define TELNET_CMD_SB       250
#define TELNET_CMD_WILL     251
#define TELNET_CMD_WONT     252
#define TELNET_CMD_DO       253
#define TELNET_CMD_DONT     254
#define TELNET_CMD_IAC      255

#define TELNET_OPT_ECHO	    1
#define TELNET_OPT_SGA	    3
#define TELNET_OPT_AUTH	    37

#define TELNET_STATE_INIT	    1
#define TELNET_STATE_NEGOTIATING    2
#define TELNET_STATE_USER	    3
#define TELNET_STATE_PASSWORD	    4
#define TELNET_STATE_CONNECTED	    5
#define TELNET_STATE_QUIT	    6

typedef struct
{
    Uint8 state;
    Uint8 laststate;
    Uint8 cmdbufpos;
    Uint8 cmdstate;
    Uint8 userid;
    Uint8 CALLBACKMEMSPEC (*commandprocessor)(char *cmd, char *buf, char *user, Uint8 userid);
    char cmdbuf[TELNET_BUFLENGTH];
    char username[TELNET_BUFLENGTH];
}
TELNET_SERVER_SESSION;

typedef struct
{
    TELNET_SERVER_SESSION *telnetsession;
}
TELNET_SERVER_CARGO;

//**************************************************************************

extern void telnet_init(void);
extern Uint16 CALLBACKMEMSPEC telnet_server(Sint8 resend, TCP_SESSION * session,
                                            Uint16 datalength, Uint8 * tcp_flags);

//**************************************************************************

#endif
