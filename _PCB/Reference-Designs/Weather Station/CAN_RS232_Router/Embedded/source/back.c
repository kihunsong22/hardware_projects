/*************************************************************************
**
**  VERSION CONTROL:    %W%	%E%
**
**  IN PACKAGE:     	Weatherstation
**
**  COPYRIGHT:      	Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	CAN-RS232-Router
**                  	Backend connection to CAN stations
**
**************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "back.h"
#include "tcpipset.h"

#include "common/tcpip.h"

#include "tealib/can.h"

//**************************************************************************

#define CAN_BUF_SIZE          1500

#define CAN_SEND_ID_START     1001
#define CAN_RECV_ID_START     2001

typedef struct
{
    char recv_id;
    char recv_buf[CAN_BUF_SIZE];
    char send_id;
    char send_buf[CAN_BUF_SIZE];
} BACKEND;

static BACKEND backends[BACKEND_COUNT];


void back_init(void)
{
    BACKEND *b;
    int i;

    can_init(500000, 0, 0xFFFFFFFF);

    b = backends;
    for (i = 0; i < BACKEND_COUNT; ++i, ++b)
    {
        b->send_id = can_init_mo(CAN_SEND_ID_START + i, CAN_TX_MODE, CAN_BUF_SIZE, b->send_buf);
        b->recv_id = can_init_mo(CAN_RECV_ID_START + i, CAN_RX_MODE, CAN_BUF_SIZE, b->recv_buf);
    }
}


Uint8 back_main(void)
{
    BACKEND *b;
    int i;

    b = backends;
    for (i = 0; i < BACKEND_COUNT; ++i, ++b)
    {
        int recvsize = can_receive_status_mo(b->recv_id);

        if (recvsize >= 0)
        {
            Uint16 ip_length;
            char *front_ip_buf;

            // received message, try to route it
            ip_length = ntohw(((IPHEADER*) b->recv_buf)->packetlength);

            front_ip_buf = framebuf_init_ip();

            if (front_ip_buf)
            {
                char *p = b->recv_buf;

                while (ip_length--)
                {
                    *front_ip_buf++ = *p++;
                }
                framebuf_send(ntohw(IP->packetlength));

                can_clear_mo(b->recv_id);
                return 1;
            }
        }
        else if (recvsize == CAN_BUF_OVL)
        {
            // buffer overflow
            can_clear_mo(b->recv_id);
            return 1;
        }
    }

    return 0;
}


void back_framebuf_send(int stationnr, char *buf, Uint16 len)
{
    if ((stationnr >= 0) && (stationnr < BACKEND_COUNT))
    {
        BACKEND *b = &backends[stationnr];

        if (can_send_status_mo(b->send_id) == CAN_READY)
        {
            char *p = buf;
            char *q = b->send_buf;
            Uint16 l = len;

            while (l--)
            {
                *q++ = *p++;
            }

            can_send_mo(b->send_id, len, NULL);
        }
    }
}


//*****************************************************************************

