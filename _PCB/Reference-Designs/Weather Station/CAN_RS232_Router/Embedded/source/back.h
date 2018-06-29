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

#ifndef _BACK_H_
#define _BACK_H_

#define BACKEND_COUNT	4

#include "tcpipset.h"

extern void back_init(void);
extern Uint8 back_main(void);

extern void back_framebuf_send(int stationnr, char *buf, Uint16 len);

#endif
