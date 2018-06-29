/*****************************************************************************\
|*
|*  VERSION CONTROL:	@(#)diskdrv.c	1.1	03/04/15
|*
|*  IN PACKAGE:		Altium TCP/IP software
|*
|*  COPYRIGHT:		Copyright (c) 2002, Altium BV
|*
|*  DESCRIPTION:
|*
|*	RAMdisk driver for filesystem
|*
\*****************************************************************************/

#include <limits.h>
#include <string.h>
#include "filesys.h"
#include "diskdrv.h"

extern Uint8 diskimg[];

#ifndef MIN
#define MIN( A, B ) ( ((A)<(B))) ? (A) : (B) )
#endif

void drv_clear(Uint32 addr, Uint32 size)
{
    if (addr < FS_DISKSIZE)
    {
	if (addr + size >= FS_DISKSIZE)
	{
	    size = FS_DISKSIZE - addr;
	}
	while (size--)
	{
	    diskimg[addr++] = 0;
	}
//              memset( diskimg + addr, 0, size );
    }

}

/*****************************************************************************\
|*
|*  FUNCTION:		drv_write
|*
|*  AVAILABILITY:	GLOBAL
|*
|*  PARAMETERS:		addr = Physical address to start writing
|*			buf = Contents to write
|*			size = number of bytes to write
|*
|*  RETURN VALUE:	0 = success, -1 otherwise
|*
|*  DESCRIPTION:
|*
|*	Write data to the drive
|*
 */

Sint16 drv_write(Uint32 addr, void *buf, Uint16 size)
{
    Uint8 *ptr = buf;
    if (addr >= FS_DISKSIZE)
    {
	return -1;
    }
    else
    {
	if ((addr + size) > FS_DISKSIZE)
	{
	    size = (Uint16) (FS_DISKSIZE - addr);
	}
	while (size--)
	{
	    diskimg[addr++] = *ptr++;
	}
    }
    return size;
}

/*****************************************************************************\
|*
|*  FUNCTION:		drv_read
|*
|*  AVAILABILITY:	GLOBAL
|*
|*  PARAMETERS:		addr = Physical address to start reading
|*			buf = Buffer to place data in, large enough to hold "size" bytes
|*			size = number of bytes to read
|*
|*  RETURN VALUE:	0 = success, -1 otherwise
|*
|*  DESCRIPTION:
|*
|*	Read data from the drive
|*
 */

Sint16 drv_read(Uint32 addr, void *buf, Uint16 size)
{
    Uint8 *ptr = buf;
    if (addr >= FS_DISKSIZE)
    {
	return -1;
    }
    else
    {
	if ((addr + size) > FS_DISKSIZE)
	{
	    size = (Uint16) (FS_DISKSIZE - addr);
	}
	while (size--)
	{
	    *ptr++ = diskimg[addr++];
	}
    }
    return 0;
}

/*****************************************************************************\
|*
|*  FUNCTION:		drv_sync
|*
|*  AVAILABILITY:	GLOBAL | LOCAL
|*
|*  PARAMETERS:
|*
|*  RETURN VALUE:
|*
|*  DESCRIPTION:
|*
|*
|*
 */

void drv_sync(void)
{
    /* Nothing to do here */
}
