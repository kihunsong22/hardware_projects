/*************************************************************************
**
**  VERSION CONTROL:	@(#)dns.c	1.2	03/04/17
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	DNS related functions
**
**************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "dns.h"

#include "tcpip.h"

//**************************************************************************

#if (!defined(OPTION_DNS_SERVER)) && (!defined(OPTION_DNS_CLIENT)) 
#ifndef TCPIPDEBUG
#error OPTION_DNS_SERVER and OPTION_DNS_CLIENT are not defined, but dns.c is included in project
#endif
#endif

#ifndef OPTION_UDP
#ifndef TCPIPDEBUG
#error OPTION_UDP is not defined, but dns.c is included in project
#endif
#endif

#if defined(OPTION_DNS_SERVER) || defined(OPTION_DNS_CLIENT)

//**************************************************************************

#ifdef DEBUG_DNS
#include "debug\dns_debug.c"
#endif

#if DEBUG_DNS >= 1
#define debug_dns(x) x
#else
#define debug_dns(x) {}
#endif

//**************************************************************************

#ifdef OPTION_DNS_CLIENT
static Uint16 udp_port_next;	// UDP next portnumber to be used (our side)
static Uint16 dns_id_next;	// DNS identifier to be used next
#endif

/***************************************************************************
 * FUNCTION:	dns_init
 *
 * Initialize the DNS module
 */
void dns_init(void)
{
#ifdef OPTION_DNS_CLIENT
    udp_port_next = UDP_PORT_DNS_CLIENT_START;
    dns_id_next = 0;
#endif
}


#ifdef OPTION_DNS_SERVER
/***************************************************************************
 * FUNCTION:	dns_server
 *
 * process incoming DNS request in given buffer, only supports questions
 * for our own IP number, all other requests are ignored
 *
 * udp_port		UDP-port on which the request came in
 * ip_datalength	number of bytes present after IP-header
 *
 * returns the length of the packet answered, 0 if no answer
 */
Uint16 CALLBACKMEMSPEC dns_server(Uint16 udp_port, Uint16 ip_datalength)
{
    char *p;
    char *q;

    // not used for this server
    udp_port = udp_port;

    debug_dns(dns_show("received"));

    // on this port our very simple DNS-server can answer only
    // the question "what IP address belongs to our name"

    // did they ask any questions?
    if (DNS->qd_count == 0)
    {
	// not a question, don't respond  TODO: should we respond we don't know?
	return 0;
    }

    // do we know the answer to the first question?
    // we are a very limited DNS server and only know our own name...
    // if the name matches: just presume it's a hostname resolve for internet :-)
    q = DNSDATA;
    p = tcpip_settings.dns_self_name;

    // q points to DNS-type string, p to human-type hostname
    while (*q != 0)
    {
	if (q >= (IPBUF + IP_MAXLEN))
	{
	    // invalid data caused bufferoverrun, discard
	    return 0;
	}

	if (strncmp(q + 1, p, *q) != 0)
	{
	    break;
	}
	p += *q;
	q += *q;
	++q;
	if (*q == 0)
	{
	    if (*p != 0)
	    {
		break;
	    }
	}
	else
	{
	    if (*p != '.')
	    {
		break;
	    }
	}
	++p;
    }

    if (*q != 0)
    {
	// unknown question, don't respond
	// TODO: should we respond we don't know?
	return 0;
    }

    // copy the name in the answer,
    // update the ip_datalength to keep track of the
    // length of the resulting packet
    p = IPDATA + ip_datalength;
    q = DNSDATA;
    strcpy(p, q);
    p += (strlen(q) + 1);
    ip_datalength += (strlen(q) + 1);

    *p++ = 0;
    *p++ = 1;	// type 'A' (hostname resolve)
    *p++ = 0;
    *p++ = 1;	// class 1 (internet)
    *p++ = 0;
    *p++ = 0;	// ttl (Uint32 in seconds, just about 4 minutes)
    *p++ = 1;
    *p++ = 0;
    *p++ = 0;
    *p++ = 4;	// length of the following IP-address
    memcpy(p, tcpip_settings.self_ip, 4);
    p += 4;	// IP-addres for the resolved host (=ourselves)
    ip_datalength += 14;

    DNS->flags |= htonw(DNS_FLAG_ANSWER);	// mark the DNS frame as answer
    DNS->an_count = htonw(1);	// set number of answers to 1

    debug_dns(dns_show("send"));

    return udp_sendprep(ip_datalength);
}
#endif // OPTION_DNS_SERVER


#ifdef OPTION_DNS_CLIENT
/***************************************************************************
 * FUNCTION:	dns_client
 *
 * Very basic DNS clientfunction able to recognize only answers on
 * IP address resolve questions. The answer is put together with our
 * client UDP-port through the callback function.
 *
 * udp_port		UDP-port on which the request came in
 * ip_datalength	number of bytes present after IP-header
 *
 * returns the length of the response (always 0)
 */
Uint16 CALLBACKMEMSPEC dns_client(Uint16 udp_port, Uint16 ip_datalength)
{
    char *p;
    Uint8 i;

    // not used for this server
    ip_datalength = ip_datalength;

    debug_dns(dns_show("received"));

    // on all non-server ports we presume we received an answer to
    // our question on that port "what IP address does a certain host have"

    // did they answer our question?
    if (DNS->an_count == 0)
    {
	// TODO: problem, DNS server didn't answer our question, hangup?
	return 0;
    }

    // we presume they understood our question and have the
    // ultimate answer, if not, we're not smart enough to try
    // anywere else anyway...

    p = DNSDATA;

    // skip all questions
    for (i = (Uint8) ntohw(DNS->qd_count); i != 0; --i)
    {
	if (p >= (IPBUF + IP_MAXLEN))
	{
	    // invalid data caused bufferoverrun, discard
	    return 0;
	}

	while (*p != 0)
	{
	    p += *p;
	    ++p;
	}
	++p;	// skip hostname
	p += 4;	// skip Qclass & Qtype
    }

    // scan all answers if one matches our request
    while (DNS->an_count-- > 0)
    {
	if (p >= (IPBUF + IP_MAXLEN))
	{
	    // invalid data caused bufferoverrun, discard
	    return 0;
	}

	if (*p & 0xC0)
	{
	    // skip pointer to our own question
	    p += 2;
	}
	else
	{
	    while (*p != 0)
	    {
		if (p >= (IPBUF + IP_MAXLEN))
		{
		    // invalid data caused bufferoverrun, discard
		    return 0;
		}

		p += *p;
		++p;
	    }
	    ++p;	// skip hostname
	}

	// check if it's a type A class 1 size 4 answer, should be the IP number we asked
	if ((p[0] == 0) && (p[1] == 1) && (p[2] == 0) && (p[3] == 1) && (p[8] == 0) && (p[9] == 4))
	{
	    p += 10;	// skip Qclass & Qtype & TTL & datalength

	    // found our answer
	    tcpip_settings.dns_portip(udp_port, p);
	    return 0;	// no answer needed
	}
	else
	{
	    // skip Qclass & Qtype & TTL & datalength & data
	    p += ntohwp(p + 8) + 10;
	}
    }

    // if we reach this line we couldn't understand the answer
    // TODO: if correct answer not found report it back?

    return 0;	// no answer needed
}
#endif // OPTION_DNS_CLIENT


#ifdef OPTION_DNS_CLIENT
/***************************************************************************
 * FUNCTION:	dns_create
 *
 * Create a request to the DNS-server we know (IP-number should be in
 * tcp_settings) to find the IP address of server_name
 *
 * returns our client UDP-port we expect the answer on, or 0 if we could't
 * send the question for any reason
 */
Uint16 dns_create(char *server_name)
{
    char *p;
    char *q;
    Uint8 i;

    if (!ip_create(*(Uint32 *) tcpip_settings.dns_world_ip))
    {
	return 0;
    }

    // use the next port in our range
    if (++udp_port_next > UDP_PORT_DNS_CLIENT_END)
    {
	udp_port_next = UDP_PORT_DNS_CLIENT_START;
    }

    udp_create(udp_port_next, UDP_PORT_DNS_SERVER);

    // construct a DNS frame
    DNS->id = htonw(++dns_id_next);
    DNS->flags = htonw(DNS_FLAG_RECURSE);
    DNS->qd_count = htonw(1);
    DNS->an_count = 0;
    DNS->ns_count = 0;
    DNS->ar_count = 0;

    // construct a question to resolve the hostname we want
    // (convert the human-type hostname into a DNS-type string)
    p = DNSDATA;
    q = server_name;
    while (strchr(q, '.') != NULL)
    {
	i = (Uint8) (strchr(q, '.') - q);
	*p++ = i;
	memcpy(p, q, i);
	p += i;
	q += i;
	++q;
    }
    *p++ = (char) strlen(q);
    strcpy(p, q);
    p += strlen(q);
    ++p;
    *p++ = 0;
    *p++ = 1;	// Qtype 'A'
    *p++ = 0;
    *p++ = 1;	// Qclass 'internet'

    debug_dns(dns_show("send"));

    framebuf_send(udp_sendprep((Uint16) (p - ((char*) UDP))));

    return udp_port_next;
}
#endif // OPTION_DNS_CLIENT

//**************************************************************************

#endif // defined(OPTION_DNS_SERVER) || defined(OPTION_DNS_CLIENT)
