/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	IP-over-CAN enabled weather station
**                      CAN interface layer for TCPIP stack
**
**************************************************************************/

#include <string.h>

#include "canip.h"
#include "tealib/can.h"

//**************************************************************************

#if !defined(OPTION_CANIP)
#ifndef TCPIPDEBUG
#error OPTION_CANIP not defined, but canip.c is included in project
#endif
#endif

#if defined(OPTION_CANIP)

//**************************************************************************

// dirty: HTTP needs to be sure it can cat a default filename to the buffer
#if defined(OPTION_HTTP) && defined(HTTP_FILESYS_DEFAULT) && defined(HTTP_DIRINDEX)
#define HTTP_SLACK sizeof(HTTP_DIRINDEX)
#else
#define HTTP_SLACK 0
#endif

// single (aligned) global buffer for TCPIP stack
Uint16 canipbuf_aligned[(ETH_BUFLEN + HTTP_SLACK) / 2];

#define CAN_BUF_SIZE  ETH_BUFLEN

char can_recv_id = 0;
char can_recv_buf[CAN_BUF_SIZE];

char can_send_id = 0;
char can_send_buf[CAN_BUF_SIZE];

/***************************************************************************
 * FUNCTION: tcpipstack_init
 *
 * called one to initialize all modules,
 * global tcpip_settings must be filled beforehand
 */
void tcpipstack_init(void)
{
    canip_init();

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
}

/***************************************************************************
 * FUNCTION: tcpipstack_main
 *
 * In case of using this linedriver this function must be called by the
 * user application on a regular basis. Returns 1 if any action was taken,
 * or 0 if none (can be used to sleep if multitasking)
 */
Uint8 tcpipstack_main(void)
{
    if (canip_action())
    {
    }
    else if (tcp_retries())
    {
    }
    else
    {
       return 0;
    }

    return 1;
}

/***************************************************************************
 * FUNCTION: tcpipstack_timertick
 *
 * In case of using this linedriver this function must be called every
 * retry-timer unit. Retry-values default in TCP module are
 * based on one tick each second.
 */
void tcpipstack_timertick(void)
{
    tcp_timertick();
}


/***************************************************************************
 * FUNCTION: canip_init
 *
 * Initialize the CANIP module
 */
void canip_init(void)
{
        can_init(500000, 0, 0xFFFFFFFF);
}


/***************************************************************************
 * FUNCTION: canip_setstation
 *
 * (Re)Create CAN message objects for the given CAN id
 */
void canip_setstation(int stationnr)
{
    if (can_recv_id)
    {
        can_delete_mo(can_recv_id);
    }
    can_recv_id = can_init_mo(CAN_RECV_ID_START + stationnr, CAN_RX_MODE, CAN_BUF_SIZE, can_recv_buf);

    if (can_send_id)
    {
        can_delete_mo(can_send_id);
    }
    can_send_id = can_init_mo(CAN_SEND_ID_START + stationnr, CAN_TX_MODE, CAN_BUF_SIZE, can_send_buf);
}



/***************************************************************************
 * FUNCTION: canip_action
 *
 * Called from TCPIP stack, executes all pending CAN-layer related processing
 */
Uint8 canip_action(void)
{
    int size = can_receive_status_mo(can_recv_id);

    if (size >= 0)
    {
        // received message, copy it to the IP-buffer
        // and have the TCPIP stack process it
        memcpy(IPBUF, can_recv_buf, size);
        can_clear_mo(can_recv_id);
        size = ip_process();
        if (size > 0)
        {
            if (can_send_status_mo(can_send_id) == CAN_READY)
            {
                // put the reply in the can-send-buffer and transmit it
                memcpy(can_send_buf, IPBUF, size);

                can_send_mo(can_send_id, size, NULL);
            }
        }

        return 1;
    }
    else if (size == CAN_BUF_OVL)
    {
        // buffer overflow
        P1_5 = 0;
        can_clear_mo(can_recv_id);
    }

    return 0;
}


/***************************************************************************
 * FUNCTION: framebuf_init_ip
 *
 * return pointer to global TPCIP buffer to send an IP frame,
 * returns NULL if no buffer available
 */
char *framebuf_init_ip(void)
{
    if (can_send_status_mo(can_send_id) == CAN_READY)
    {
        return can_send_buf;
    }

    return NULL;
}


/***************************************************************************
 * FUNCTION: framebuf_send
 *
 * Send an IP frame from the global TPCIP buffer with the given length
 */
void framebuf_send(Uint16 len)
{
    // copy the message to send in the can-send-buffer and transmit it
    memcpy(can_send_buf, IPBUF, len);

    can_send_mo(can_send_id, len, NULL);
}

//**************************************************************************

#endif
