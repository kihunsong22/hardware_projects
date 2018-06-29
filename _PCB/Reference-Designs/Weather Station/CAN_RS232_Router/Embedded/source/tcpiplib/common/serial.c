/*************************************************************************
**
**  VERSION CONTROL:	@(#)serial.c	1.3	03/04/24
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Serial related functions, everything below the
**			IP level needed to connect SLIP and/or PPP on a direct
**			connection or by modem.
**
**************************************************************************/

#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include <stdlib.h>
#endif

#include "serial.h"
#include "tcpip.h"
#include "drivers/serial_driver.h"

#if defined(OPTION_DNS_CLIENT) || defined(OPTION_DNS_SERVER)
#include "dns.h"
#endif

//**************************************************************************

#if !defined(OPTION_SERIAL)
#ifndef TCPIPDEBUG
#error OPTION_SERIAL not defined, but serial.c is included in project
#endif
#endif

#if defined(OPTION_SERIAL)

#ifdef DEBUG_SERIAL
#include "debug\serial_debug.c"
#endif

#if DEBUG_SERIAL >= 1
#define debug_serial1(x) x
#else
#define debug_serial1(x) {}
#endif
#if DEBUG_SERIAL >= 2
#define debug_serial2(x) x
#else
#define debug_serial2(x) {}
#endif
#if DEBUG_SERIAL >= 3
#define debug_serial3(x) x
#else
#define debug_serial3(x) {}
#endif

//**************************************************************************

// all times below are expressed in serial_timertick() calls

// maximum time allowed for an incoming packet
#ifndef SERIAL_PACKETTIME_TICKS
#define SERIAL_PACKETTIME_TICKS 5
#endif

// maximum time allowed for special actions
#ifndef SERIAL_MODEMCONNECTTIME_TICKS
#define SERIAL_MODEMCONNECTTIME_TICKS 45
#endif
#ifndef SERIAL_PPPCONNECTTIME_TICKS
#define SERIAL_PPPCONNECTTIME_TICKS 15
#endif
#ifndef SERIAL_BREAKTIME_TICKS
#define SERIAL_BREAKTIME_TICKS 15
#endif

#define AT_BUFLEN 20	// longest AT command supported

#define PPP_FCS16_INIT	0xffff	// Initial FCS value
#define PPP_FCS16_GOOD	0xf0b8	// Good final FCS value

//**************************************************************************

// dirty: HTTP needs to be sure it can cat a default filename to the buffer
#if defined(OPTION_HTTP) && defined(HTTP_FILESYS_DEFAULT) && defined(HTTP_DIRINDEX)
#define HTTP_SLACK sizeof(HTTP_DIRINDEX)
#else
#define HTTP_SLACK 0
#endif

Uint16 serbuf_aligned[(SERIAL_BUFLEN + HTTP_SLACK)/2];	// SLIP/PPP frame buffer
#define buf ((Uint8*) serbuf_aligned)

static Uint16 buf_pos;	// current position in frame buffer
static Uint8 buf_state;	// state for processing frame buffer
static Uint8 buf_datastate;	// datastate (PPP, SLIP, ESCAPE etc)
static Uint8 buf_timer;	// timer for watchdog on incoming packets
static Uint8 line_timer;	// timer for watchdog on special actions

static Uint8 ntras_state;	// state for detecting NT RAS direct calling in

static Uint16 line_state;	// overall state for PPP handshake mechanism

#ifdef SERIAL_PPP
static Uint8 ppp_fullescapemode;	// escape all character under 0x20 or not
static Uint8 ppp_ident;	// running ID-counter to discriminate PPP messages
#endif

#ifdef SERIAL_MODEM
static Uint8 modem_state;	// overall state of modem
static char atbuffer[AT_BUFLEN];	// buffer for incoming AT-commands
static Uint8 atpos;	// writing position in atbuffer
#endif

static void buf_reset(Uint8 datastate);

#ifdef SERIAL_PPP
static void buf_init_ppp_cp(Uint16 protocol, Uint8 ident, Uint8 code);
static Uint16 ppp_process(void);
static Uint16 ppp_cp_process(Uint16 protocol, Uint8 * lcp, Sint16 maxlength);
static Uint16 ppp_fcs16(Uint16 fcs, Uint8 * p, Sint16 length);
#endif

#ifdef SERIAL_MODEM
static void modem_recv(char c);
#endif

static void ntras_recv(char c);


/***************************************************************************
 * FUNCTION:	tcpipstack_init
 *
 * called one to initialize all modules,
 * global tcpip_settings must be filled beforehand
 */
void tcpipstack_init(void)
{
    comm_init();
    serial_init();
    tcpip_init();

#if defined(OPTION_DNS_CLIENT) || defined(OPTION_DNS_SERVER)
    dns_init();
#endif
#ifdef OPTION_FTP_SERVER
    ftp_init();
#endif
#ifdef OPTION_TELNET_SERVER
    telnet_init();
#endif
#ifdef SERIAL_MODEM
    modem_init();
#endif
}



/***************************************************************************
 * FUNCTION:	tcpipstack_main
 *
 * In case of using this linedriver this function must be called by the
 * user application on a regular basis. Returns 1 if any action was taken,
 * or 0 if none (can be used to sleep if multitasking)
 */
Uint8 tcpipstack_main(void)
{
    Uint8 c;

#ifdef SERIAL_MODEM
    if (modem_action())
    {
    }
    else
#endif
    if (serial_action())
    {
    }
#ifdef SERIAL_PPP
    else if (ppp_action())
    {
    }
#endif
    else if (tcp_retries())
    {
    }
    else if (comm_get(&c))
    {
	serial_recv(c);
    }
    else
    {
	return 0;
    }

    return 1;
}



/***************************************************************************
 * FUNCTION:	tcpipstack_timertick
 *
 * In case of using this linedriver this function must be called every
 * retry-timer unit. Retry-values default in TCP and LINE modules are
 * based on one tick each second.
 */
void tcpipstack_timertick(void)
{
    tcp_timertick();
    serial_timertick();
}


/***************************************************************************
 * FUNCTION:	serial_init
 *
 * Initialization of line buffer modules
 */
void serial_init(void)
{
    buf_reset(BUF_DATASTATE_UNKNOWN);
    ntras_state = 0;

    line_state = LINESTATE_IDLE;
    debug_serial1(state_show(line_state));
    tcpip_settings.serial_statuschange(line_state);

    line_timer = 0;

#ifdef SERIAL_PPP
    ppp_fullescapemode = 1;
    ppp_ident = 0;
#endif
}


/***************************************************************************
 * FUNCTION:	buf_reset
 *
 * Reset the line buffer
 */
static void buf_reset(Uint8 datastate)
{
    buf_pos = 0;
    buf_state = BUF_STATE_IDLE;
    buf_datastate = datastate;
    buf_timer = 0;
}


Uint8 serial_bufempty(void)
{
    return (buf_pos == 0);
}


/***************************************************************************
 * FUNCTION:	serial_timertick
 *
 * Update & check watchdogtimer for incoming packets
 */
void serial_timertick(void)
{
    debug_serial1(buf_timer ? printf("buftimer %u\n", buf_timer) : 0);
    debug_serial1(line_timer ? printf("linetimer %u\n", line_timer) : 0);

    // handle watchdog timeout on incoming packet
    if ((buf_timer != 0) && (--buf_timer == 0))
    {
	debug_serial1(printf(s("LINE buffer reset by watchdog\n")));
	buf_reset(BUF_DATASTATE_UNKNOWN);
    }

    // handle watchdog timeout on building connection
    if ((line_timer != 0) && (--line_timer == 0))
    {
	debug_serial1(printf(s("LINE reset by watchdog\n")));
	// NB: watchdog useless without PPP or modem (SLIP direct link)
#ifdef SERIAL_MODEM
#ifdef SERIAL_PPP
	switch (modem_state)
	{
	case MODEM_STATE_CONNECTED:
	    ppp_terminate();
	    break;
	case MODEM_STATE_BREAK_WAITOK:
	    modem_state = MODEM_STATE_HANGUP;
	    break;
	default:
	    modem_state = MODEM_STATE_BREAK;
	    break;
	}
#else
	if (modem_state == MODEM_STATE_BREAK_WAITOK)
	{
	    modem_state = MODEM_STATE_HANGUP;
	}
	else
	{
	    modem_state = MODEM_STATE_BREAK;
	}
#endif
#else
	serial_init();
#endif
    }
}


/***************************************************************************
 * FUNCTION:	serial_recv
 *
 * Process a single incoming character on the serial line.
 * When receiving PPP the HDLC-line framing is stripped from the input,
 * when receiving SLIP the frame & escape codes are removed
 * If the modem is not online and it isn't the start of a SLIP/PPP packet
 * send it on for modem-command processing. Send all decoded data from the
 * SLIP packet on for IP processing, and from PPP frames to PPP processing
 */
void serial_recv(Uint8 c)
{
#ifdef SERIAL_MODEM
    if ((modem_state == MODEM_STATE_IDLE) || (modem_state == MODEM_STATE_CONNECTED))
    {
#endif
#ifdef SERIAL_PPP
	if ((buf_datastate == BUF_DATASTATE_UNKNOWN_PREVPPP) && (c == 0xFF))
	{
	    // presume start of PPP-packet without interframe fill (i.e. not a double PPP_FRAME)
	    buf_datastate = BUF_DATASTATE_PPP;
	    buf_timer = SERIAL_PACKETTIME_TICKS;
	}

	if ((buf_datastate == BUF_DATASTATE_PPP) || (buf_datastate == BUF_DATASTATE_PPPESC))
	{
	    switch (c)
	    {
	    case PPP_FRAME:
		// could be end of PPP-packet, or an empty frame (skip)
		if (buf_pos > 0)
		{
		    buf_state = BUF_STATE_PPPIN;
		}

		// deactivate watchdog
		buf_timer = 0;

		break;

	    case PPP_ESCAPE:
		buf_datastate = BUF_DATASTATE_PPPESC;
		break;

	    default:
		if (buf_datastate == BUF_DATASTATE_PPPESC)
		{
		    c ^= PPP_ESCAPE_MASK;
		    buf_datastate = BUF_DATASTATE_PPP;
		}

		if (buf_pos < SERIAL_BUFLEN)
		{
		    // store the received character in the buffer
		    buf[buf_pos++] = c;
		}
		else
		{
		    // buffer overrun, discard the complete packet
		    buf_reset(BUF_DATASTATE_UNKNOWN_PREVPPP);
		}

	    }

	    return;
	}
#endif // SERIAL_PPP

#ifdef SERIAL_SLIP
	if ((buf_datastate == BUF_DATASTATE_SLIP) || (buf_datastate == BUF_DATASTATE_SLIPESC))
	{
	    switch (c)
	    {
	    case SLIP_END:
		// end of SLIP-packet
		buf_state = BUF_STATE_SLIPIN;
		// deactivate watchdog
		buf_timer = 0;
		break;

	    case SLIP_ESC:
		buf_datastate = BUF_DATASTATE_SLIPESC;
		break;

	    default:
		if (buf_datastate == BUF_DATASTATE_SLIPESC)
		{
		    switch (c)
		    {
		    case SLIP_ESC_END:
			// received escaped END character
			c = SLIP_END;
			break;

		    case SLIP_ESC_ESC:
			// received escaped ESC character
			c = SLIP_ESC;
			break;

		    default:
			// ignoring illegal character, if it's a real problem
			// more intelligent layers should detect a checksum error
			return;

		    }

		    buf_datastate = BUF_DATASTATE_SLIP;
		}

		if (buf_pos < SERIAL_BUFLEN)
		{
		    // store the received character in the buffer
		    buf[buf_pos++] = c;
		}
		else
		{
		    // buffer overrun, discard the complete packet
		    buf_reset(BUF_DATASTATE_UNKNOWN);
		}
	    }

	    return;
	}
#endif // SERIAL_SLIP

#ifdef SERIAL_PPP
	if (c == PPP_FRAME)
	{
	    // start of PPP-packet
	    buf_datastate = BUF_DATASTATE_PPP;
	    buf_timer = SERIAL_PACKETTIME_TICKS;
#ifdef SERIAL_MODEM
	    if (modem_state == MODEM_STATE_IDLE)
#else
	    if (line_state == LINESTATE_IDLE)
#endif
	    {
		debug_serial1(printf(s("line received PPP_FRAME, presuming online or direct connection\n")));

		// pretend they are calling us, in that case we can select our own IP numbers
#ifdef SERIAL_MODEM
		modem_state = MODEM_STATE_CONNECTED;
#endif
		line_state = LINESTATE_CALLIN_PPPCONNECTED;
		debug_serial1(state_show(line_state));
		tcpip_settings.serial_statuschange(line_state);

		// in practice this is always negotiated
		ppp_fullescapemode = 0;
	    }

	    return;
	}
#endif // SERIAL_PPP

#ifdef SERIAL_SLIP
	if (c == SLIP_END)
	{
	    // start of SLIP-packet
	    buf_datastate = BUF_DATASTATE_SLIP;
	    buf_timer = SERIAL_PACKETTIME_TICKS;
#ifdef SERIAL_MODEM
	    if (modem_state == MODEM_STATE_IDLE)
#else
	    if (line_state == LINESTATE_IDLE)
#endif
	    {
		debug_serial1(printf(s("modem received SLIP_END, presuming online or direct connection\n")));

		// pretend they are calling us, in that case we can select our own IP numbers
#ifdef SERIAL_MODEM
		modem_state = MODEM_STATE_CONNECTED;
#endif
		line_state = LINESTATE_CALLIN;
		debug_serial1(state_show(line_state));
		tcpip_settings.serial_statuschange(line_state);
	    }
	    return;
	}
#endif // SERIAL_SLIP
#ifdef SERIAL_MODEM
    }
#endif

    // check for special case NT RAS direct connection caller
    ntras_recv(c);

#ifdef SERIAL_MODEM
    // if a modem is connected then presume it is in commandmode,
    modem_recv(c);
#else
    // otherwise ignore received character until next SLIP_END or PPP_FRAME
    debug_serial1(printf("line discarded char %02X='%c'\n", c, isprint(c) ? c : '.'));
#endif // SERIAL_MODEM
}


/***************************************************************************
 * FUNCTION:	framebuf_init_ip
 *
 * If the buffer is available (meaning empty) initialize it to send a
 * PPP-IP frame. A pointer to the start of the IP frame is returned,
 * or NULL if the buffer was not available
 */
char *framebuf_init_ip(void)
{
    if (buf_pos != 0)
    {
	// already busy writing
	return NULL;
    }

    if ((line_state == LINESTATE_CALLIN_PPPCONNECTED) || (line_state == LINESTATE_CALLOUT_PPPCONNECTED))
    {
	// send ppp
	buf_pos = 0;
	buf[buf_pos++] = 0xFF;
	buf[buf_pos++] = 0x03;
	buf[buf_pos++] = highbyte(PPP_PROTOCOL_IP);
	buf[buf_pos++] = lowbyte(PPP_PROTOCOL_IP);

    }
    else if ((line_state == LINESTATE_CALLIN) || (line_state == LINESTATE_CALLOUT))
    {
	// send slip
	buf_pos = 0;
    }
    else
    {
	// no output available
	debug_serial1(printf("LINE discarded request for buffer, connection down\n"));
	return NULL;
    }

    return IPBUF;
}


/***************************************************************************
 * FUNCTION:	buf_init_cpp_cp
 *
 * initialize the buffer to send a PPP-control frame with given
 * PPP protocol, ident and code
 */
static void buf_init_ppp_cp(Uint16 protocol, Uint8 ident, Uint8 code)
{
    buf_pos = 0;
    buf[buf_pos++] = 0xFF;
    buf[buf_pos++] = 0x03;
    buf[buf_pos++] = highbyte(protocol);
    buf[buf_pos++] = lowbyte(protocol);
    buf[buf_pos++] = code;
    buf[buf_pos++] = ident;
    buf[buf_pos++] = 0;
    buf_pos++;
}


/***************************************************************************
 * FUNCTION:	framebuf_send
 *
 * Send the current buffer plus a given number of bytes of the current
 * buffer over the line.
 */
void framebuf_send(Uint16 len)
{
    if ((line_state == LINESTATE_CALLIN_PPPCONNECTED) || (line_state == LINESTATE_CALLOUT_PPPCONNECTED))
    {
	// send ppp
	buf_pos += len;
	buf_state = BUF_STATE_PPPOUT;
    }
    else if ((line_state == LINESTATE_CALLIN) || (line_state == LINESTATE_CALLOUT))
    {
	// send slip
	buf_pos += len;
	buf_state = BUF_STATE_SLIPOUT;
    }
    else
    {
	// discard packet
	buf_pos = 0;
	debug_serial1(printf("LINE discarded transmissing, connection down\n"));
    }
}



/***************************************************************************
 * FUNCTION:	serial_action
 *
 * Execute any line related actions needed depending on the buf_state.
 * When sending PPP the HDLC-line framing is added, when sending SLIP
 * the frame & escape codes are added.
 * Returns 1 if buffer has been filled with new data to be send, 0 otherwise.
 */
Uint8 serial_action(void)
{
    Uint8 *p;
    Uint8 c;
#ifdef SERIAL_PPP
    Uint16 checksum;
#endif
#ifdef TEST_SEND_INJECT_ERRORS
    static testcount = 0;
#endif

    switch (buf_state)
    {
#ifdef SERIAL_PPP
    case BUF_STATE_PPPIN:
	debug_serial3((printf("PPP received "), printbytes((char*)buf, buf_pos)));
	buf_pos = ppp_process();
	if (buf_pos == 0)
	{
	    buf_reset(BUF_DATASTATE_UNKNOWN_PREVPPP);
	    return 0;
	}
	buf_state = BUF_STATE_PPPOUT;
	return 1;

    case BUF_STATE_PPPOUT:
#ifdef TEST_SEND_INJECT_ERRORS
	if ((++testcount & TEST_SEND_INJECT_ERRORS) == 0)
	{
	    printf("SIMULATING SEND ERROR - DROPPING OUTGOING PPP PACKET\n");
	    buf_reset(BUF_DATASTATE_UNKNOWN_PREVPPP);
	    return 0;
	}
#endif
	// write the checksum, send the buffer inside a PPP frame
	checksum = ~ppp_fcs16(PPP_FCS16_INIT, buf, buf_pos);
	buf[buf_pos++] = lowbyte(checksum);
	buf[buf_pos++] = highbyte(checksum);
	debug_serial3((printf("PPP send "), printbytes((char*)buf, buf_pos), printf("\n")));

	// send start of frame
	comm_put(PPP_FRAME);
	for (p = buf; buf_pos != 0; --buf_pos)
	{
	    c = *p++;
	    // send databyte, always escape the FRAME and ESCAPE characters,
	    // other bytes depending on the escape mode.
	    // Start is with all first 32 characters escaped, presume
	    // negotiotion always ends with none escaped. It does.
	    if ((c == PPP_FRAME) || (c == PPP_ESCAPE) || (ppp_fullescapemode && (c < PPP_ESCAPE_MASK)))
	    {
		comm_put(PPP_ESCAPE);
		comm_put((Uint8) (c ^ PPP_ESCAPE_MASK));
	    }
	    else
	    {
		comm_put(c);
	    }
	}

	// send end of frame (would not be needed when sending consecutive
	// frames, but the future is hard to detect at this point...)
	comm_put(PPP_FRAME);
	comm_flush();

	buf_reset(BUF_DATASTATE_UNKNOWN_PREVPPP);

	if (line_state & LINESTATE_TERMINATE)
	{
#ifdef SERIAL_MODEM
	    modem_state = MODEM_STATE_BREAK;
	    line_state = LINESTATE_TERMINATE_WAITFORMODEM;
#else
	    // we just transmitted a reset as a service to the other side,
	    // we move to idle state  (and hang up the modem if needed)
	    // without further communication
	    serial_init();
#endif
	}

	return 0;

#endif // SERIAL_PPP

#ifdef SERIAL_SLIP
    case BUF_STATE_SLIPIN:
	debug_serial3((printf("SLIP received "), printbytes(buf, buf_pos)));
#ifdef SERIAL_PPP
        // shift SLIP packet inside buffer to emulate a PPP encapsulated packet
        memmove(buf + PPP_BUF_OFFSET, buf, buf_pos);
#endif
	buf_pos = ip_process();
	if (buf_pos == 0)
	{
	    buf_reset(BUF_DATASTATE_UNKNOWN);
	    return 0;
	}
	buf_state = BUF_STATE_SLIPOUT;
	return 1;

    case BUF_STATE_SLIPOUT:
	// send the buffer inside a SLIP frame
	debug_serial3((printf("SLIP send "), printbytes(buf, buf_pos), printf("\n")));

	// send start of frame
	comm_put(SLIP_END);
	// send databyte, always escape the END and ESCAPE characters,
#ifdef SERIAL_PPP
        // SLIP packet is shifted inside buffer to emulate a PPP encapsulated packet
	p = buf + PPP_BUF_OFFSET;
#else
	p = buf;
#endif
        for (p; buf_pos != 0; --buf_pos)
	{
	    c = *p++;
	    switch (c)
	    {
	    case SLIP_END:
		comm_put(SLIP_ESC);
		comm_put(SLIP_ESC_END);
		break;
	    case SLIP_ESC:
		comm_put(SLIP_ESC);
		comm_put(SLIP_ESC_ESC);
		break;
	    default:
		comm_put(c);
		break;
	    }
	}
	// send end of frame
	comm_put(SLIP_END);
	comm_flush();

	buf_reset(BUF_DATASTATE_UNKNOWN);
	return 0;

#endif // SERIAL_SLIP

    default:
	// no action needed
	break;

    }

    return 0;
}


#ifdef SERIAL_PPP
#ifdef SERIAL_CALLOUT
/***************************************************************************
 * FUNCTION:	ppp_logon
 *
 * Start process to build a PPP connection, modem connection must be
 * established or a direct link must be present.
 */
void ppp_logon(void)
{
    line_state = LINESTATE_CALLOUT;
    line_timer = SERIAL_PPPCONNECTTIME_TICKS;
    debug_serial1(state_show(line_state));
}
#endif // SERIAL_CALLOUT


/***************************************************************************
 * FUNCTION:	ppp_terminate
 *
 * Start process to terminate a PPP connection, if modem connection
 * is present it will be broken
 */
void ppp_terminate(void)
{
    line_state = LINESTATE_TERMINATE;
    debug_serial1(state_show(line_state));
}

#endif // SERIAL_PPP


#ifdef SERIAL_PPP
/***************************************************************************
 * FUNCTION:	ppp_action
 *
 * Depending on the state of the PPP connection (kept in line_state)
 * create a CP packet in the line buffer to continue up the PPP handshaking.
 * Returns 1 if buffer has been filled with new data to be send, 0 otherwise.
 */
Uint8 ppp_action(void)
{
#ifdef SERIAL_CALLOUT
    static Uint8 delays;
#endif

    if (buf_pos != 0)
    {
	// busy receiving in the buffer, don't overwrite
	return 0;
    }

    switch (line_state)
    {
#ifdef SERIAL_CALLOUT
    case LINESTATE_CALLOUT | LINESTATE_LCP_OUT_SEND:	// retry
	// TODO: replace with real watchdog?
	if (delays != 0)
	{
	    sys_sleep(250);
	    --delays;
	    return 0;
	}

    case LINESTATE_CALLOUT:
	// build an empty LCP CONF_REQ
	buf_init_ppp_cp(PPP_PROTOCOL_LCP, ++ppp_ident, CP_CONF_REQ);

	line_state |= LINESTATE_LCP_OUT_SEND;
	debug_serial1(state_show(line_state));
	delays = 10;
	break;

    case LINESTATE_CALLOUT | LINESTATE_LCP_OK:
	// build a PAP CONF_REQ with our username & password
	buf_init_ppp_cp(PPP_PROTOCOL_PAP, ++ppp_ident, CP_CONF_REQ);

	buf[buf_pos++] = strlen(tcpip_settings.ppp_out_user);	// optionlength
	memcpy(buf + buf_pos, tcpip_settings.ppp_out_user, strlen(tcpip_settings.ppp_out_user));
	buf_pos += strlen(tcpip_settings.ppp_out_user);

	buf[buf_pos++] = strlen(tcpip_settings.ppp_out_password);	// optionlength
	memcpy(buf + buf_pos, tcpip_settings.ppp_out_password, strlen(tcpip_settings.ppp_out_password));
	buf_pos += strlen(tcpip_settings.ppp_out_password);

	line_state |= LINESTATE_PAP_OUT_SEND;
	debug_serial1(state_show(line_state));
	break;

    case LINESTATE_CALLOUT | LINESTATE_LCP_OK | LINESTATE_PAP_OUT_OK:
    case LINESTATE_CALLOUT | LINESTATE_LCP_OK | LINESTATE_PAP_OUT_OK | LINESTATE_IPCP_IN_OK:
	// build an IPCP CONF_REQ to ask our self IP and world DNS-IP address
	buf_init_ppp_cp(PPP_PROTOCOL_IPCP, ++ppp_ident, CP_CONF_REQ);

	buf[buf_pos++] = IPCP_OPTION_ADDRESS;
	buf[buf_pos++] = 6;	// optionlength
	memcpy(buf + buf_pos, tcpip_settings.self_ip, 4);
	buf_pos += 4;

#ifdef OPTION_DNS_CLIENT
	buf[buf_pos++] = IPCP_OPTION_DNS;
	buf[buf_pos++] = 6;	// optionlength
	memcpy(buf + buf_pos, tcpip_settings.dns_world_ip, 4);
	buf_pos += 4;
#endif
	line_state |= LINESTATE_IPCP_OUT_SEND;
	debug_serial1(state_show(line_state));
	break;

#endif // SERIAL_CALLOUT

    case LINESTATE_CALLIN | LINESTATE_LCP_IN_RECV:
    case LINESTATE_CALLIN | LINESTATE_LCP_IN_OK:
	// build an LCP CONF_REQ with request for PAP and clearing the escape map
	buf_init_ppp_cp(PPP_PROTOCOL_LCP, ++ppp_ident, CP_CONF_REQ);

	buf[buf_pos++] = LCP_OPTION_AUTHPROT;
	buf[buf_pos++] = 4;	// optionlength
	buf[buf_pos++] = highbyte(PPP_PROTOCOL_PAP);
	buf[buf_pos++] = lowbyte(PPP_PROTOCOL_PAP);

	buf[buf_pos++] = LCP_OPTION_ACCM;
	buf[buf_pos++] = 6;	// optionlength
	buf[buf_pos++] = 0;
	buf[buf_pos++] = 0;
	buf[buf_pos++] = 0;
	buf[buf_pos++] = 0;

	line_state |= LINESTATE_LCP_OUT_SEND;
	debug_serial1(state_show(line_state));
	break;

    case LINESTATE_CALLIN | LINESTATE_LCP_OK | LINESTATE_PAP_IN_OK | LINESTATE_IPCP_IN_RECV:
    case LINESTATE_CALLIN | LINESTATE_LCP_OK | LINESTATE_PAP_IN_OK | LINESTATE_IPCP_IN_OK:
	// build an IPCP CONF_REQ to tell our own IP
	buf_init_ppp_cp(PPP_PROTOCOL_IPCP, ++ppp_ident, CP_CONF_REQ);

	buf[buf_pos++] = IPCP_OPTION_ADDRESS;
	buf[buf_pos++] = 6;	// optionlength
	memcpy(buf + buf_pos, tcpip_settings.self_ip, 4);
	buf_pos += 4;

	line_state |= LINESTATE_IPCP_OUT_SEND;
	debug_serial1(state_show(line_state));
	break;

    case LINESTATE_TERMINATE:
	// build an empty LCP TERM_REQ
	buf_init_ppp_cp(PPP_PROTOCOL_LCP, ++ppp_ident, CP_TERM_REQ);

	// no watchdog needed, this is just a service to the other side,
	// we hang up after we transmitted the terminate
	break;

    default:
	// no action needed
	return 0;

    }

    buf[7] = buf_pos - 4;	// store length written
    debug_serial1(cp_show((Uint16) ((buf[2] << 8) + buf[3]), s("send"), buf + 4, (Uint16) (buf_pos - 4)));
    buf_state = BUF_STATE_PPPOUT;

    return 1;
}
#endif // SERIAL_PPP


// process a PPP frame in the incoming buffer
#ifdef SERIAL_PPP
/***************************************************************************
 * FUNCTION:	ppp_process
 *
 * Process an incoming PPP frame in the line buffer, send data on
 * for PPP-CP or IP processing, and response to unknown protocols with a
 * LCP protocol rejection.
 * Returns 1 if buffer has been filled with new data to be send, 0 otherwise.
 */
static Uint16 ppp_process(void)
{
    Uint8 *p = buf;
    Uint16 protocol;

#ifdef TEST_RECV_INJECT_ERRORS
    static testcount = 0;
    if ((++testcount & TEST_RECV_INJECT_ERRORS) == 0)
    {
	printf("SIMULATING RECV ERROR - DROPPING INCOMING PPP PACKET\n");
	return 0;
    }
#endif

    if (*p++ != 0xFF)
    {
	debug_serial1(printf(s("PPP: illegal address %02X (FF expected)\n"), *--p));
	return 0;
    }

    if (*p++ != 0x03)
    {
	debug_serial1(printf(s("PPP: illegal control %02X (03 expected)\n"), *--p));
	return 0;
    }

    // verify checksum
    if (ppp_fcs16(PPP_FCS16_INIT, buf, buf_pos) != PPP_FCS16_GOOD)
    {
	debug_serial1(printf(s("PPP: checksum error\n")));
	return 0;
    }

    // if protocol is odd, length is 2 bytes, is even then 1 Uint8
    // (first Uint8 always other way round, i.e. if first Uint8 even -> 2nd Uint8 follows)
    protocol = *p++;
    if (!(protocol & 0x01))
    {
	protocol = (protocol << 8) + *p++;
    }

    switch (protocol)
    {
    case PPP_PROTOCOL_LCP:
    case PPP_PROTOCOL_IPCP:
    case PPP_PROTOCOL_PAP:
	buf_pos = ppp_cp_process(protocol, p, (Uint16) (buf_pos - (p - buf) - 2));
	if (buf_pos != 0)
	{
	    buf_pos += (p - buf);
	}

	// detect if we finished building a connection
	if ((line_timer != 0) &&
	    ((line_state == LINESTATE_CALLIN_PPPCONNECTED) || (line_state == LINESTATE_CALLOUT_PPPCONNECTED)))
	{
	    line_timer = 0;	// deactivate watchdog
	    debug_serial1(printf(s("PPP connected\n")));
	    tcpip_settings.serial_statuschange(line_state);
	}
	break;

    case PPP_PROTOCOL_IP:
	buf_pos = ip_process();
	if (buf_pos != 0)
	{
	    buf_pos += (p - buf);
	}
	break;

    default:
	// unknown protocol, send an LCP packet to reject it
	debug_serial1(printf(s("PPP received protocol %04X, rejecting\n"), protocol));
	buf_init_ppp_cp(PPP_PROTOCOL_LCP, ++ppp_ident, CP_PROT_REJ);
	buf[buf_pos++] = highbyte(protocol);
	buf[buf_pos++] = lowbyte(protocol);
	buf[7] = buf_pos - 4;	// store length written
	debug_serial1(cp_show(PPP_PROTOCOL_LCP, s("send"), buf + 4, (Uint16) (buf_pos - 4)));
	break;
    }

    return buf_pos;
}
#endif // SERIAL_PPP


#ifdef SERIAL_PPP
/***************************************************************************
 * FUNCTION:	ppp_cp_process
 *
 * Process any incoming PPP CP packet. Supported are only a minimal
 * set of LCP, PAP and IPCP needed to build up a PPP IP connection
 * both as client and as server.
 * Returns 1 if buffer has been filled with new data to be send, 0 otherwise.
 */
static Uint16 ppp_cp_process(Uint16 protocol, Uint8 * cp, Sint16 maxlength)
{
    Sint16 length;
    Uint8 *readpos, *writepos;
    Uint8 reply, option, *optionpos, optionlength, action = 0;

    debug_serial1(cp_show(protocol, s("received"), cp, maxlength));

    if (cp[1] >= ppp_ident)
    {
	++ppp_ident;
    }

    length = (cp[2] << 8) + cp[3];
    if (length > maxlength)
    {
	// CP frame was send incomplete in PPP frame, discard silently
	return 0;
    }

    readpos = cp + 4;
    writepos = cp + length;	// trick: seems like we rewrote all options unchanged, can be changed later on
    length -= 4;
    reply = 0;

    switch (*cp)
    {
    case CP_TERM_REQ:
	if (protocol == PPP_PROTOCOL_LCP)
	{
	    // just acknowledge with all data unchanged, rely on the watchdog
	    // to break the link if the other side doesn't
	    line_timer = SERIAL_BREAKTIME_TICKS;
	    reply = CP_TERM_ACK;
	}
	else
	{
	    // illegal, just ignore
	    return 0;
	}
	break;

    case CP_CONF_ACK:
	// detect if we started a new incoming direct connection
	// (if started with an incoming modem the timer already started on the
	// modem CONNECT)
	if ((line_state == LINESTATE_IDLE) || (line_timer == 0))
	{
	    line_timer = SERIAL_PPPCONNECTTIME_TICKS;
	}

	switch (protocol)
	{
	case PPP_PROTOCOL_LCP:
	    if (line_state & LINESTATE_LCP_OUT_SEND)
	    {
		line_state |= LINESTATE_LCP_OUT_RECV;
		debug_serial1(state_show(line_state));
	    }
	    break;

#ifdef SERIAL_CALLOUT
	case PPP_PROTOCOL_PAP:
	    if (line_state & LINESTATE_PAP_OUT_SEND)
	    {
		line_state |= LINESTATE_PAP_OUT_RECV;
		debug_serial1(state_show(line_state));
	    }
	    break;
#endif

	case PPP_PROTOCOL_IPCP:
	    if (line_state & LINESTATE_IPCP_OUT_SEND)
	    {
		line_state |= LINESTATE_IPCP_OUT_RECV;
		debug_serial1(state_show(line_state));
	    }
	    break;

	}
	return 0;	// no response needed

    case CP_ECHO_REQ:
	if (protocol == PPP_PROTOCOL_LCP)
	{
	    // just reply with all data unchanged
	    reply = CP_ECHO_REP;
	}
	else
	{
	    // illegal, just ignore
	    return 0;
	}
	break;

    case CP_CONF_NAK:
	while (length > 0)
	{
	    // remember where the data belonging to this option started
	    optionpos = readpos;
	    option = *readpos++;
	    optionlength = *readpos++;
	    length -= 2;

	    // skip all optionbytes
	    readpos += (optionlength - 2);
	    length -= (optionlength - 2);

	    switch (protocol)
	    {
	    case PPP_PROTOCOL_IPCP:
		switch (option)
		{
		case IPCP_OPTION_ADDRESS:
		    // server suggest an IP address for us to use
		    memcpy(tcpip_settings.self_ip, optionpos + 2, 4);
		    break;

#ifdef OPTION_DNS_CLIENT
		case IPCP_OPTION_DNS:
		    // server suggests an IP address for a world DNS
		    memcpy(tcpip_settings.dns_world_ip, optionpos + 2, 4);
		    break;
#endif
		default:
		    // unknown NAK for IPCP protocol, abort negotiations
		    line_state = LINESTATE_TERMINATE;
		    debug_serial1(state_show(line_state));
		    return 0;

		}

		// resend out IPCP request
		line_state &= (Uint16) ~ LINESTATE_IPCP_OUT_SEND;
		debug_serial1(state_show(line_state));
		break;

	    default:
		// NAK for unexpected protocol, abort negotiations
		// (covers PAP-NAK of invalid username/passwords as well)
		line_state = LINESTATE_TERMINATE;
		debug_serial1(state_show(line_state));
		return 0;

	    }
	}

	// don't respond to a NAK, modification of the line_state
	// should be sufficient for further processing if needed
	return 0;

    case CP_CONF_REQ:
#ifdef SERIAL_CALLIN
	if (protocol == PPP_PROTOCOL_PAP)
	{
	    // password check
	    optionpos = readpos + *readpos + 1;	// start of password
	    optionlength = *optionpos;	// length of password
	    *optionpos = 0;	// null-terminate user
	    *(optionpos + optionlength + 1) = 0;	// null-terminate password

	    if (tcpip_settings.ppp_usercheck((char *) readpos + 1, (char *) optionpos + 1) == 0)
	    {
		// illegal password, terminate
		line_state = LINESTATE_TERMINATE;
		debug_serial1(state_show(line_state));
		return 0;
	    }

	    // password accepted
	    reply = CP_CONF_ACK;
	    writepos = cp + 4;
	    *writepos++ = 0;	// just reply with empty message
	}
	else
#endif
	{
	    reply = CP_CONF_ACK;
	    while (length > 0)
	    {
		// remember where the data belonging to this option started
		optionpos = readpos;
		option = *readpos++;
		optionlength = *readpos++;
		length -= 2;

		// skip all optionbytes
		readpos += (optionlength - 2);
		length -= (optionlength - 2);

		switch (protocol)
		{
		case PPP_PROTOCOL_LCP:
		    switch (option)
		    {
		    case LCP_OPTION_ACCM:
			// TODO: suppport FFFF & 0000, reject all others (for now presume 0000,
			// no known OS asks anything else for a normal callout anyway)
		    case LCP_OPTION_MAGIC:
			// accept but ignore
		    case LCP_OPTION_MRU:
			// accept but ignore (presume 1500)
		    case LCP_OPTION_QUALPROT:
			// accept but ignore
			action = CP_CONF_ACK;
			break;

		    case LCP_OPTION_AUTHPROT:
			// accept only PAP
			if ((optionpos[2] == highbyte(PPP_PROTOCOL_PAP)) && (optionpos[3] == lowbyte(PPP_PROTOCOL_PAP)))
			{
			    action = CP_CONF_ACK;
			}
			else
			{
			    // write back we want PAP...
			    optionpos[1] = 4;	// returning option could be 1 Uint8 shorter
			    optionpos[2] = highbyte(PPP_PROTOCOL_PAP);
			    optionpos[3] = lowbyte(PPP_PROTOCOL_PAP);
			    optionlength = 4;
			    action = CP_CONF_NAK;
			}
			break;

		    default:
			// reject
			action = CP_CONF_REJ;
			break;

		    }
		    break;

		case PPP_PROTOCOL_IPCP:
		    switch (option)
		    {
		    case IPCP_OPTION_ADDRESS:
			if ((optionpos[2] == 0) && (optionpos[3] == 0) && (optionpos[4] == 0) && (optionpos[5] == 0))
			{
			    // if server says IP=0, this means "tell me what you want"
			    // reply with the IP for our client in the NAK-ed option
			    memcpy(optionpos + 2, tcpip_settings.ppp_client_ip, 4);
			    action = CP_CONF_NAK;
			}
			else
			{
			    // accept all IP addresses they suggest for their side,
			    // DON'T store them as we really don't care
			    // who the client is, we just support this
			    // exchange to make PPP connections happy
			    action = CP_CONF_ACK;
			}
			break;

#ifdef OPTION_DNS_SERVER
		    case IPCP_OPTION_DNS:
			if ((optionpos[2] == 0) && (optionpos[3] == 0) && (optionpos[4] == 0) && (optionpos[5] == 0))
			{
			    // if server says DNS=0, this means "tell me what you want"
			    // reply with our own IP (we are the DNS) in the NAK-ed option
			    memcpy(optionpos + 2, tcpip_settings.self_ip, 4);
			    action = CP_CONF_NAK;
			}
			else
			{
			    // accept all DNS addresses they suggest for our DNS
			    // DON'T store them as this should not happen,
			    // we only end up here if they can't support our
			    // DNS-IP. And according to normal protocol they should
			    // unless it clashes with their own IP-space and they are
			    // smart enough to detect that (which Microsoft isn't).
			    // TODO: should we hangup if our DNS-IP is unacceptable,
			    // the connection is probably useless if they can't see our DNS?
			    action = CP_CONF_ACK;
			}
			break;
#endif
		    default:
			// reject
			action = CP_CONF_REJ;
			break;

		    }
		    break;

		}

		// matter of prority: CODE_REJ overrules CONF_REJ overrules CONF_NAK overrules CONF_ACK
		if (action > reply)
		{
		    reply = action;
		    writepos = cp + 4;	// start writing at the beginning of the optionspace again
		}

		// for CONF_ACK all options are kept unchanged,
		// for rejecting/nak-ing of values or codes we only send the ones we reject/nak
		//
		// we don't confirm to the RFC here: when sending a CodeReject
		// we don't repeat our received packet. Seems all known implementations
		// on the other side don't care anyway.
		if (((action == CP_CONF_REJ) || (action == CP_CONF_NAK)) && (reply == action))
		{
		    if (writepos != optionpos)
		    {
			// move our option tightly forward in the buffer
			memcpy(writepos, optionpos, optionlength);
		    }
		    writepos += optionlength;
		}
	    }
	}
	break;

    default:
	// unsupported CP packet type, DISCARD ?
	debug_serial1(printf(s("CP packet type %02X, discarded\n"), *cp));
	return 0;

    }

    // write back the reply-code and length
    *cp = reply;
    length = writepos - cp;
    cp[2] = highbyte(length);
    cp[3] = lowbyte(length);

    debug_serial1(cp_show(protocol, s("reply"), cp, length));

    // if we reply with conf-ack we can mark a layer as OK
    if (*cp == CP_CONF_ACK)
    {
	switch (protocol)
	{
	case PPP_PROTOCOL_LCP:
	    line_state |= LINESTATE_LCP_IN_OK;
	    break;

#ifdef SERIAL_CALLIN
	case PPP_PROTOCOL_PAP:
	    line_state |= LINESTATE_PAP_IN_OK;
	    break;
#endif

	case PPP_PROTOCOL_IPCP:
	    line_state |= LINESTATE_IPCP_IN_OK;
	    break;

	default:
	    // should never reach this line
	    break;
	}
	debug_serial1(state_show(line_state));
    }

    return length;
}
#endif // SERIAL_PPP


//******************************************************************************


#ifdef SERIAL_PPP
// FCS lookup table needed by ppp_fcs16()
static Uint16 ROMMEMSPEC fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
#endif // SERIAL_PPP


#ifdef SERIAL_PPP
/***************************************************************************
 * FUNCTION:	ppp_fcs16
 *
 * Calculate a new PPP fcs (checksum) given the current fcs and the new data,
 * returns the new fcs.
 * (Calculation & fcstab above based on reference implementation)
 */
static Uint16 ppp_fcs16(Uint16 fcs, Uint8 * p, Sint16 length)
{
    while (length--)
    {
	fcs = (fcs >> 8) ^ fcstab[(fcs ^ *p++) & 0xff];
    }

    return (fcs);
}
#endif // SERIAL_PPP

//******************************************************************************

#ifdef SERIAL_MODEM
/***************************************************************************
 * FUNCTION:	modem_init
 *
 * initialize modem (should contain no autoanswer!)
 * Should work even on a connected modem line or direct SLIP connection.
 */
void modem_init(void)
{
    comm_putstr_rom(tcpip_settings.modem_init);
    comm_flush();
    atpos = 0;
    modem_state = MODEM_STATE_IDLE;
}


#ifdef SERIAL_CALLOUT
#ifdef SERIAL_PPP
/***************************************************************************
 * FUNCTION:	modem_dialout_ppp
 *
 * Start process to build a modemlink and PPP connection to a provider.
 */
void modem_dialout_ppp(void)
{
    // TODO: check if modem & ppp idle?
    modem_state = MODEM_STATE_CALLOUT;
}
#endif
#endif


/***************************************************************************
 * FUNCTION:	modem_break
 *
 * Start process to break a connected modemlink.
 * Only use to break SLIP connections, for PPP callin/callout
 * use ppp_terminate, this breaks the modem link as well.
 */
void modem_break(void)
{
    modem_state = MODEM_STATE_BREAK;
}


/***************************************************************************
 * FUNCTION:	modem_action
 *
 * Excecute any modem related actions depending on the modem_state.
 * Returns 1 if any data was send to the modem, 0 if none.
 */
Uint8 modem_action(void)
{
    switch (modem_state)
    {
#ifdef SERIAL_CALLOUT
    case MODEM_STATE_CALLOUT:
	// dial a provider
	debug_serial1(printf(s("modem dialing out\n")));
	comm_putstr_rom("ATDT");
	comm_putstr(tcpip_settings.modem_dialoutnr);
	comm_put('\r');
	comm_flush();

	buf_reset(BUF_DATASTATE_UNKNOWN);	// modem_recv() gets modem response
	line_timer = SERIAL_MODEMCONNECTTIME_TICKS;
	break;
#endif // SERIAL_CALLOUT

    case MODEM_STATE_BREAK:
	debug_serial1(printf(s("modem break & hangup\n")));
	// break starts with 1 second delay
	sys_sleep(1000);
	break;

    case MODEM_STATE_BREAK_SEND:
	comm_putstr_rom("+++");
	comm_flush();
	buf_reset(BUF_DATASTATE_UNKNOWN);	// modem_recv() gets modem response
	line_timer = SERIAL_BREAKTIME_TICKS;
	break;

    case MODEM_STATE_HANGUP:
	comm_putstr_rom("ATH0\r\n");
	comm_flush();
	buf_reset(BUF_DATASTATE_UNKNOWN);	// modem_recv() gets modem response
	line_timer = SERIAL_BREAKTIME_TICKS;
	break;

    default:
	// no action needed
	return 0;

    }

    ++modem_state;
    return 1;
}



/***************************************************************************
 * FUNCTION:	modem_recv
 *
 * Process a character c received by the modem in commandmode.
 * Recognizes RING, OK, CONNECT, NO CARRIER. The modem is answered
 * if needed and the line_state and modem_state are adapted according
 * to changes indicated by those incoming strings.
 */
static void modem_recv(char c)
{
    debug_serial3(printf("modem char %02X='%c'\n", (Uint8) c, isprint(c) ? c : '.'));

    if ((c == '\r') | (c == '\n'))
    {
	if (atpos == 0)
	{
	    return;
	}

	// received AT info
	debug_serial1(atbuffer[atpos] = 0);
	debug_serial1(printf(s("modem received '%s'\n"), atbuffer));

	if (strncmp_rom(atbuffer, "RING", 4) == 0)
	{
	    debug_serial1(printf(s("modem answering\n")));
	    // why is this extra delay needed? Maybe because \n or \r still has be received
	    // first before sending our answer command?
	    sys_sleep(200);

	    comm_putstr_rom("ATA\r");
	    comm_flush();
	    modem_state = MODEM_STATE_WAITCONNECT;
	    line_timer = SERIAL_MODEMCONNECTTIME_TICKS;
	}
	else if (strncmp_rom(atbuffer, "CONNECT", 7) == 0)
	{
	    debug_serial1(printf(s("modem online\n")));
	    if (line_state == LINESTATE_IDLE)
	    {
		if (modem_state == MODEM_STATE_CALLOUT_WAITCONNECT)
		{
		    // outgoing connection succeded
		    line_state = LINESTATE_CALLOUT;
		}
		else
		{
		    // modemstate was probably IDLE, start a new incoming connection
		    line_state = LINESTATE_CALLIN;
		}
		line_timer = SERIAL_PPPCONNECTTIME_TICKS;
		debug_serial1(state_show(line_state));
		tcpip_settings.serial_statuschange(line_state);
	    }
	    modem_state = MODEM_STATE_CONNECTED;
	}
	else if (strncmp_rom(atbuffer, "BUSY", 10) == 0)
	{
	    modem_state = MODEM_STATE_IDLE;
	    debug_serial1(printf(s("modem busy\n")));
	    serial_init();
	}
	else if (strncmp_rom(atbuffer, "NO CARRIER", 10) == 0)
	{
	    modem_state = MODEM_STATE_IDLE;
	    debug_serial1(printf(s("modem offline\n")));
	    serial_init();
	}
	else if (strncmp_rom(atbuffer, "OK", 2) == 0)
	{
	    if (modem_state == MODEM_STATE_BREAK_WAITOK)
	    {
		++modem_state;
	    }
	    else if (modem_state == MODEM_STATE_HANGUP_WAITOK)
	    {
		modem_state = MODEM_STATE_IDLE;
		debug_serial1(printf(s("modem offline\n")));
		serial_init();
	    }
	}
	atpos = 0;
    }
    else
    {
	if (atpos >= AT_BUFLEN)
	{
	    // if atbuffer overflow, discard received data and start again with an empty buffer
	    atpos = 0;
	}

	atbuffer[atpos++] = c;
    }
}

#endif // SERIAL_MODEM


/***************************************************************************
 * FUNCTION:	ntras_recv	
 *
 * Process a character c received by the modem in commandmode.
 * This function only detects if NT RAS is trying to call us over a
 * "direct networking serial cable".
 * If NT sends "CLIENT", we return "CLIENTSERVER\r" and line_state and
 * modem_state are adapted to indicate an active connection.
 */
static void ntras_recv(char c)
{
    if (c == "CLIENT"[ntras_state])
    {
	if (++ntras_state == 6)
	{
	    debug_serial1(printf(s("Line detected NT RAS caller\n")));
#ifdef SERIAL_MODEM
	    modem_state = MODEM_STATE_CONNECTED;
#endif
	    comm_putstr_rom("CLIENTSERVER\r");
	    comm_flush();
	    ntras_state = 0;

	    // very basic: whenever we receive a new NT-RAS connection
	    // reset the PPP negotiation
	    line_state = LINESTATE_CALLIN;
#ifdef SERIAL_PPP
	    ppp_fullescapemode = 1;
#endif
	}
    }
    else
    {
	ntras_state = 0;
    }
}

//******************************************************************************

#endif
