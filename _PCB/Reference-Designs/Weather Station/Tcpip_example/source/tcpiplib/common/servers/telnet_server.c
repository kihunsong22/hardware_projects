/*************************************************************************
**
**  VERSION CONTROL:    @(#)telnet_server.c	1.4 03/12/17
**
**  IN PACKAGE:     Embedded TCPIP
**
**  COPYRIGHT:      Copyright (c) 2002 Altium
**
**  DESCRIPTION:    Demo application: simple TELNET server
**
**************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "telnet_server.h"
#include "common/tcpip.h"

//**************************************************************************

#ifndef OPTION_TELNET_SERVER
#ifndef TCPIPDEBUG
#error OPTION_TELNET_SERVER is not defined, but telnet_server.c is included in project
#endif
#endif

#ifdef OPTION_TELNET_SERVER

//**************************************************************************

#ifdef DEBUG_TELNET
#define debug_telnet(x) x
#else
#define debug_telnet(x)
#endif

const static Uint8 ournegotiation[] = {
    TELNET_CMD_IAC, TELNET_CMD_DO, TELNET_OPT_SGA,
    TELNET_CMD_IAC, TELNET_CMD_WILL, TELNET_OPT_ECHO,
    TELNET_CMD_IAC, TELNET_CMD_WONT, TELNET_OPT_AUTH,
    0
};

static TELNET_SERVER_SESSION telnetsessions[TELNET_SESSIONS];

/***************************************************************************
 * FUNCTION:    telnet_init
 *
 * Initialize the TELNET module
 */
void telnet_init(void)
{
    memset(telnetsessions, 0, sizeof(telnetsessions));
}


/***************************************************************************
 * FUNCTION:    telnet_server
 *
 * TELNET server application. The only thing it does is display a welcome message,
 * echo all input back and react on the command "quit" by ending the telnet session.
 * Negotiation is based on setting echo on and refusing anything else
 *
 * resend   if non-zero the last packet has to be regenerated
 * session  active TCP session
 * datalength   number of incoming bytes
 * tcp_flags    pointer to flags in TCP-header
 *
 * returns number of bytes answered
 */
Uint16 CALLBACKMEMSPEC telnet_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags)
{
    TELNET_SERVER_SESSION *telnetsession = session->cargo.telnet_server.telnetsession;
    Uint8 *p;
    char *retp;
    Uint16 dataleft;
    Uint8 i;

    if (*tcp_flags & (TCP_FLAG_RESET | TCP_FLAG_FIN))
    {
        if (telnetsession)
    {
        // free telnet session
        memset(telnetsession, 0, sizeof(TELNET_SERVER_SESSION));
    }
        return 0;
    }

    if (session->state == TCP_STATE_SYNACK)
    {
        // Find a free slot in our list of TELNET sessions,
        // in our TELNET cargo we will just keep a pointer to the TELNET session.
        for (i = 0; i < TELNET_SESSIONS; ++i)
        {
            if (!telnetsessions[i].state)
            {
                telnetsession = &telnetsessions[i];
                session->cargo.telnet_server.telnetsession = telnetsession;
            }
        }

        if (!telnetsession)
        {
            // no free TELNET sessions
        strcpy_rom(TCPDATA, "TELNET service not available, too many users\r\n");
        *tcp_flags |= TCP_FLAG_FIN;
            return strlen(TCPDATA);
        }

    telnetsession->state = TELNET_STATE_INIT;
    telnetsession->laststate = TELNET_STATE_INIT;

    // if the client doesn't start negotiating soon we will,
    // set a timer so this server is called even if no packet comes in
    
    // -1 is magic number indicating external trigger
    session->retries = -1;

    // about 25 ms would suffice, but out timer resolution is 1 second,
    // and we don't know how long until the next tick.
    session->timer = 2; 
    return 0;
    }

    if (!telnetsession)
    {
    // ACK/FIN on closed sessions, no further processing needed
    return 0;
    }

    if (resend)
    {
        if (telnetsession->laststate != TELNET_STATE_CONNECTED)
        {
            telnetsession->state = telnetsession->laststate;
        debug_telnet(printf("retry state %i\n", telnetsession->state));
        }
        else
        {
            // we only support retries while negotiating, reset connection
            *tcp_flags |= TCP_FLAG_RESET;
            return 0;
        }
    }
    else
    {
    debug_telnet(datalength ? (printf("TELNET received: ", datalength), printbytes(TCPDATA, datalength)) : 0);
    }

    p = (Uint8*) TCPDATA;
    retp = TCPDATA;
    dataleft = datalength;
    while (dataleft--)
    {
    if ((*p == TELNET_CMD_IAC) || (telnetsession->cmdstate == TELNET_CMD_IAC))
    {
        telnetsession->cmdstate = *p;
    }
    else if (telnetsession->cmdstate)
    {
#ifdef DEBUG_TELNET
        printf("TELNET received command ");
        switch (telnetsession->cmdstate)
        {
        case TELNET_CMD_SB:
        printf("suboption");
        break;
        case TELNET_CMD_WILL:
        printf("WILL");
        break;
        case TELNET_CMD_WONT:
        printf("WONT");
        break;
        case TELNET_CMD_DO:
        printf("DO");
        break;
        case TELNET_CMD_DONT:
        printf("DONT");
        break;
        default:
        printf("%i", telnetsession->cmdstate - 0x100);
        break;
        }

        printf(" value %u\n", *p);
#endif
        if (telnetsession->cmdstate != TELNET_CMD_SB)
        {
        telnetsession->cmdstate = 0;
        }
    }
    else if ((*p == '\n') || (*p == '\r'))
    {
        debug_telnet(printf("TELNET received \\n\n"));

        strcpy_rom(retp, "\r\n");
        // terminate buffer
        telnetsession->cmdbuf[telnetsession->cmdbufpos] = '\0';
        debug_telnet(printf("TELNET processing commandbuffer (%i) '%s'\n", telnetsession->cmdbufpos, telnetsession->cmdbuf));
        switch (telnetsession->state)
        {
        case TELNET_STATE_USER:
        strcpy(telnetsession->username, telnetsession->cmdbuf);
                strcat_rom(retp, "Password: ");
        telnetsession->state = TELNET_STATE_PASSWORD;
        break;

        case TELNET_STATE_PASSWORD:
        if (tcpip_settings.telnet_server_usercheck(telnetsession->username, telnetsession->cmdbuf, &telnetsession->commandprocessor, &telnetsession->userid))
                {
            strcat_rom(retp, "Login accepted\r\n\r\n");
            telnetsession->state = TELNET_STATE_CONNECTED;
                }
                else
                {
            strcat_rom(retp, "Invalid user/password\r\n");
                    telnetsession->state = TELNET_STATE_QUIT;
                }

        break;
    
        case TELNET_STATE_CONNECTED:
                if (!telnetsession->commandprocessor(telnetsession->cmdbuf, retp, telnetsession->username, telnetsession->userid))
                {
                    telnetsession->state = TELNET_STATE_QUIT;
                }
        break;

        }

        if (telnetsession->state == TELNET_STATE_CONNECTED)
        {
        strcat_rom(retp, "> ");
        }

        retp += strlen(retp);

        // reset buffer
        telnetsession->cmdbufpos = 0;
        break;
    }
    else if (*p == '\r')
    {
        debug_telnet(printf("TELNET ignored \\r\n"));
        // discard
    }
    else if (telnetsession->cmdbufpos < TELNET_BUFLENGTH)
    {
        telnetsession->cmdbuf[telnetsession->cmdbufpos++] = *p;

        if (telnetsession->state != TELNET_STATE_PASSWORD)
        {
        // echo character
        *retp++ = *p;
        }

        debug_telnet(printf("TELNET added %02X to buffer (%i)\n", (Uint8) *p, telnetsession->cmdbufpos));
    }
    else
    {
        debug_telnet(printf("TELNET dropped character, buffer overflow\n"));
    }
    ++p;
    }

    telnetsession->laststate = telnetsession->state;

    debug_telnet(printf("reply for state %i", telnetsession->state));
    switch (telnetsession->state)
    {
    case TELNET_STATE_INIT:
    strcpy_rom(retp, "TELNET miniserver\r\n\r\nLogin: ");
        retp += strlen(retp);
    telnetsession->state = TELNET_STATE_NEGOTIATING;
    break;

    case TELNET_STATE_NEGOTIATING:
    // send our negotiation options
    strcpy(retp, (char*) ournegotiation);
    retp += strlen(retp);
        telnetsession->state = TELNET_STATE_USER;
    break;

    case TELNET_STATE_QUIT:
        // free & reset TELNET sesion info
        memset(telnetsession, 0, sizeof(TELNET_SERVER_SESSION));

        strcat_rom(retp, "Bye!!\r\n");
        *tcp_flags = TCP_FLAG_FIN;
    break;

    }
    debug_telnet(printf(" length %i\n", retp - TCPDATA));

    // return data or empty ACK
    *tcp_flags |= (TCP_FLAG_ACK | TCP_FLAG_PUSH);
    if (retp == TCPDATA)
    {
    *tcp_flags &= (Uint8) ~TCP_FLAG_PUSH;
    }

    return retp - TCPDATA;
}


//**************************************************************************

#endif // OPTION_TELNET_SERVER
