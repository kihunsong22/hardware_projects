/*************************************************************************
**
**  VERSION CONTROL:	@(#)serial_driver.h	1.1	03/04/15
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	low-level serial API
**
**************************************************************************/

#ifndef _SERIAL_DRIVER_H_
#define _SERIAL_DRIVER_H_

#include "tcpipset.h"

//**************************************************************************

extern void comm_init(void);
extern void comm_flush(void);
extern void comm_putstr(char *s);
extern void comm_put(Uint8 c);
extern Uint8 comm_get(Uint8 * c);

#ifdef ROMMEMSPEC_AUTOCAST
#define comm_putstr_rom(s) comm_putstr((char*) s)
#else
extern void comm_putstr_rom(char ROMMEMSPEC * s);
#endif

//**************************************************************************

#endif
