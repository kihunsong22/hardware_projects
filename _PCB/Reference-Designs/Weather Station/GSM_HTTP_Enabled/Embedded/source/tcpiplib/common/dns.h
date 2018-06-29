/*************************************************************************
**
**  VERSION CONTROL:	@(#)dns.h	1.1	03/04/15
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	DNS related functions
**
**************************************************************************/

#ifndef _DNS_H_
#define _DNS_H_

#include "tcpipset.h"
#include "tcpip_global.h"

//**************************************************************************

// UDP portnumbers for DNS services
#define UDP_PORT_DNS_SERVER			53
#define UDP_PORT_DNS_CLIENT_START	1000
#define UDP_PORT_DNS_CLIENT_END		1100

// DNS bitmask flags
#define DNS_FLAG_ANSWER		0x8000
#define DNS_FLAG_RECURSE	0x0100

typedef struct
{
    Uint16 id;
    Uint16 flags;
    Uint16 qd_count;
    Uint16 an_count;
    Uint16 ns_count;
    Uint16 ar_count;
}
DNSHEADER;
#define DNSHEADER_LEN  sizeof(DNSHEADER)
#define DNS ((DNSHEADER*) (IPBUF + IPHEADER_LEN + UDPHEADER_LEN))
#define DNSDATA ((char*) (IPBUF + IPHEADER_LEN + UDPHEADER_LEN + DNSHEADER_LEN))

//**************************************************************************

extern void dns_init(void);

#ifdef OPTION_DNS_SERVER
extern Uint16 CALLBACKMEMSPEC dns_server(Uint16 udp_port, Uint16 ip_datalength);
#endif
#ifdef OPTION_DNS_CLIENT
extern Uint16 CALLBACKMEMSPEC dns_client(Uint16 udp_port, Uint16 ip_datalength);
extern Uint16 dns_create(char *server_name);
#endif

//**************************************************************************

#endif
