/*************************************************************************
**
**  VERSION CONTROL:    %W%	%E%
**
**  IN PACKAGE:     	Weatherstation Demo
**
**  COPYRIGHT:      	Copyright (c) 2003 Altium
**
**  DESCRIPTION:    	IP-over-CAN enabled weather station
**                  	System settings for TCPIP stack
**
**************************************************************************/

#ifndef _TCPIPSYSSET_H_
#define _TCPIPSYSSET_H_

//**************************************************************************

// because we are using "keep strings in ROM"
#define ROMSTR

// hardware specific header
#include "fpga/sys_8051fpga.h"

// define is used by the SFR file but needed by the TCPIP library
#undef IP

//**************************************************************************

#endif
