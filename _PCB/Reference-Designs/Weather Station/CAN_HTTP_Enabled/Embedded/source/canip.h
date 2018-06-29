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

#ifndef _CANIP_H_
#define _CANIP_H_

#include "tcpipset.h"
#include "common/tcpip.h"

//**************************************************************************

#define IP_MAXLEN		1500
#define ETH_BUFLEN		IP_MAXLEN

#define CAN_RECV_ID_START      1000
#define CAN_SEND_ID_START      2000

//**************************************************************************

extern Uint16 canipbuf_aligned[];
#define IPBUF ((char*) canipbuf_aligned)

extern void tcpipstack_init(void);
extern Uint8 tcpipstack_main(void);
extern void tcpipstack_timertick(void);

extern void canip_init(void);
extern void canip_setstation(int stationnr);

extern Uint8 canip_action(void);

extern char *framebuf_init_ip(void);
extern void framebuf_send(Uint16 len);

//**************************************************************************

#endif
