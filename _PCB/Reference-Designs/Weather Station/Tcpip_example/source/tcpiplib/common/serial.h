/*************************************************************************
**
**  VERSION CONTROL:	@(#)serial.h	1.3	03/10/31
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Line related functions,	everything below the
**			IP level needed to connect SLIP, PPP on a direct
**			connection or by modem.
**
**************************************************************************/
#include "tcpipset.h"
#include "common/tcpip_global.h"

#ifndef _SERIAL_H_
#define _SERIAL_H_

#if defined(OPTION_SERIAL)

//**************************************************************************

#ifndef SERIAL_BUFLEN
#define SERIAL_BUFLEN		1500
#endif

// this is for SLIP, will be overruled below in case PPP is used
#define IP_MAXLEN		SERIAL_BUFLEN

// SLIP frame and escape ecodes
#define SLIP_END		0300
#define SLIP_ESC		0333
#define SLIP_ESC_END		0334
#define SLIP_ESC_ESC		0335

//*****************************

// PPP has more overhead than SLIP, allowed IP length is shorter
#define PPP_HEADER_OVERHEAD     6
#ifdef SERIAL_PPP
#undef IP_MAXLEN
#define IP_MAXLEN		(SERIAL_BUFLEN - PPP_HEADER_OVERHEAD)
#endif

// PPP frame and escape ecodes
#define PPP_FRAME		0x7E
#define PPP_ESCAPE		0x7D
#define PPP_ESCAPE_MASK		0x20

// supported PPP protocols
#define PPP_PROTOCOL_LCP	0xC021
#define PPP_PROTOCOL_IPCP	0x8021
#define PPP_PROTOCOL_PAP	0xC023
#define PPP_PROTOCOL_IP		0x0021

// CP negotiotion messagetypes
#define CP_CONF_REQ		0x01
#define CP_CONF_ACK		0x02
#define CP_CONF_NAK		0x03
#define CP_CONF_REJ		0x04
#define CP_TERM_REQ		0x05
#define CP_TERM_ACK		0x06
#define CP_CODE_REJ		0x07
#define CP_PROT_REJ		0x08
#define CP_ECHO_REQ		0x09
#define CP_ECHO_REP		0x0A

// supported LCP options
#define LCP_OPTION_MRU		0x01
#define LCP_OPTION_ACCM		0x02
#define LCP_OPTION_AUTHPROT	0x03
#define LCP_OPTION_QUALPROT	0x04
#define LCP_OPTION_MAGIC	0x05
#define LCP_OPTION_PROTCOMP	0x07
#define LCP_OPTION_ADDRCOMP	0x08

// supported IPCP options
#define IPCP_OPTION_ADDRESSES	0x01
#define IPCP_OPTION_COMPRESSION	0x02
#define IPCP_OPTION_ADDRESS	0x03
#define IPCP_OPTION_DNS		0x81

//**************************************************************************

// linestates (including PPP negotiation)
#define LINESTATE_CALLIN		    0x0001
#define LINESTATE_CALLOUT	            0x0002
#define LINESTATE_LCP_IN_RECV		    0x0004
#define LINESTATE_LCP_IN_SEND	            0x0008
#define LINESTATE_LCP_IN_OK (LINESTATE_LCP_IN_RECV|LINESTATE_LCP_IN_SEND)
#define LINESTATE_LCP_OUT_SEND		    0x0010
#define LINESTATE_LCP_OUT_RECV		    0x0020
#define LINESTATE_LCP_OUT_OK (LINESTATE_LCP_OUT_SEND|LINESTATE_LCP_OUT_RECV)
#define LINESTATE_LCP_OK (LINESTATE_LCP_IN_OK|LINESTATE_LCP_OUT_OK)
#define LINESTATE_PAP_IN_RECV                 0x0040
#define LINESTATE_PAP_IN_SEND                 0x0080
#define LINESTATE_PAP_IN_OK (LINESTATE_PAP_IN_RECV|LINESTATE_PAP_IN_SEND)
#define LINESTATE_PAP_OUT_SEND                0x0100
#define LINESTATE_PAP_OUT_RECV                0x0200
#define LINESTATE_PAP_OUT_OK (LINESTATE_PAP_OUT_SEND|LINESTATE_PAP_OUT_RECV)
#define LINESTATE_IPCP_IN_RECV                0x0400
#define LINESTATE_IPCP_IN_SEND                0x0800
#define LINESTATE_IPCP_IN_OK (LINESTATE_IPCP_IN_RECV|LINESTATE_IPCP_IN_SEND)
#define LINESTATE_IPCP_OUT_SEND               0x1000
#define LINESTATE_IPCP_OUT_RECV               0x2000
#define LINESTATE_IPCP_OUT_OK (LINESTATE_IPCP_OUT_SEND|LINESTATE_IPCP_OUT_RECV)
#define LINESTATE_IPCP_OK (LINESTATE_IPCP_IN_OK|LINESTATE_IPCP_OUT_OK)
#define LINESTATE_TERMINATE                   0x4000
#define LINESTATE_TERMINATE_WAITFORMODEM      0x8000

// following states are reported through callback
#define LINESTATE_IDLE			0x0000
#define LINESTATE_CALLIN_PPPCONNECTED (LINESTATE_CALLIN|LINESTATE_LCP_OK|LINESTATE_PAP_IN_OK|LINESTATE_IPCP_OK)
#define LINESTATE_CALLOUT_PPPCONNECTED (LINESTATE_CALLOUT|LINESTATE_LCP_OK|LINESTATE_PAP_OUT_OK|LINESTATE_IPCP_OK)

// line datastate: this decides what is happening to incoming data
#define BUF_DATASTATE_UNKNOWN		0x00
#define BUF_DATASTATE_UNKNOWN_PREVPPP	0x01
#define BUF_DATASTATE_PPP		0x02
#define BUF_DATASTATE_PPPESC		0x03
#define BUF_DATASTATE_SLIP		0x04
#define BUF_DATASTATE_SLIPESC		0x05

// states for modem connection
#define MODEM_STATE_IDLE	    	0x00
#define MODEM_STATE_CALLOUT	    	0x01
#define MODEM_STATE_CALLOUT_WAITCONNECT 0x02
#define MODEM_STATE_WAITCONNECT    	0x03
#define MODEM_STATE_CONNECTED	    	0x04
#define MODEM_STATE_BREAK	    	0x05
#define MODEM_STATE_BREAK_SEND	    	0x06
#define MODEM_STATE_BREAK_WAITOK    	0x07
#define MODEM_STATE_HANGUP	    	0x08
#define MODEM_STATE_HANGUP_WAITOK   	0x09

// state for linebuffer: if not idle full buffer is ready to be processed
#define BUF_STATE_IDLE			0x00
#define BUF_STATE_PPPIN		0x01
#define BUF_STATE_SLIPIN		0x02
#define BUF_STATE_PPPOUT		0x03
#define BUF_STATE_SLIPOUT		0x04

//**************************************************************************

extern Uint16 serbuf_aligned[];
#define PPP_BUF_OFFSET 4
#ifdef SERIAL_PPP
#define IPBUF (((char*) serbuf_aligned) + PPP_BUF_OFFSET)
#else
#define IPBUF ((char*) serbuf_aligned)
#endif

extern void tcpipstack_init(void);
extern Uint8 tcpipstack_main(void);
extern void tcpipstack_timertick(void);

extern void serial_init(void);
extern Uint8 serial_bufempty(void);
extern void serial_recv(Uint8 c);
extern void serial_timertick(void);
extern Uint8 serial_action(void);

extern char *framebuf_init_ip(void);
extern void framebuf_send(Uint16 len);

#ifdef SERIAL_PPP
#ifdef SERIAL_CALLOUT
extern void ppp_logon(void);
#endif
extern void ppp_terminate(void);
#if defined(SERIAL_CALLOUT) || defined(SERIAL_CALLIN)
extern Uint8 ppp_action(void);
#endif
#endif // SERIAL_PPP

#ifdef SERIAL_MODEM
extern void modem_init(void);
#ifdef SERIAL_CALLOUT
#ifdef SERIAL_PPP
extern void modem_dialout_ppp(void);
#endif
#endif
extern void modem_break(void);
extern Uint8 modem_action(void);
#endif

//**************************************************************************

#endif
#endif
