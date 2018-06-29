/*************************************************************************
**
**  VERSION CONTROL:    %W%	%E%
**
**  IN PACKAGE:     	Weatherstation
**
**  COPYRIGHT:      	Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	CAN-RS232-Router
**             	    	System settings for TCPIP stack
**
**************************************************************************/

#ifndef _TCPIPSYSSET_H_
#define _TCPIPSYSSET_H_

// include routing code
#define OPTION_IP_ROUTING

// define if using "keep strings in ROM"
#define ROMSTR

// hardware specific header
#include "fpga/sys_8051fpga.h"

// used by the SFR file
#undef IP

#endif
