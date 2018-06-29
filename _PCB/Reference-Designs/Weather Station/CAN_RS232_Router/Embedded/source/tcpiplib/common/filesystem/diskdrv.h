/*****************************************************************************\
|*
|*  VERSION CONTROL:	@(#)diskdrv.h	1.1	03/04/15
|*
|*  IN PACKAGE:
|*
|*  AUTHORS:
|*
|*  COPYRIGHT:		Copyright (c) 2002, Altium BV
|*
|*  DESCRIPTION:
|*
|*	Diskdrive interface description
|*
\*****************************************************************************/

#ifndef _DISKDRV_H
#define _DISKDRV_H

#include "tcpipset.h"
#include "../tcpip_global.h"

//**************************************************************************

extern Sint16 drv_write(Uint32 addr, void *buf, Uint16 size);
extern Sint16 drv_read(Uint32 addr, void *buf, Uint16 size);
extern void drv_sync(void);
extern void drv_clear(Uint32 addr, Uint32 size);

//**************************************************************************

#endif //************************************************************************** _DISKDRV_H
