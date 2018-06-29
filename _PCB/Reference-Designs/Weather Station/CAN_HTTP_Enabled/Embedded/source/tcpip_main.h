/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	IP-over-CAN enabled weather station
**                 	Interface between main program and TCPIP library
**
**************************************************************************/

#ifndef _TCPIP_MAIN_H_
#define _TCPIP_MAIN_H_

//**************************************************************************

#define SELF_IP                 {192, 168, 100, 1}  // our IP

//**************************************************************************

void tcpipmain_init(void);
int tcpipmain(void);

void tcpipmain_setstation(int stationnr);

//**************************************************************************

#endif
