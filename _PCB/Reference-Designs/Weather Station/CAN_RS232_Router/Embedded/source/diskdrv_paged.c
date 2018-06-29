/*****************************************************************************\
|*
|*  VERSION CONTROL:    %W% %E%
|*
|*  IN PACKAGE:         Altium TCP/IP software
|*
|*  COPYRIGHT:          Copyright (c) 2002, Altium BV
|*
|*  DESCRIPTION:
|*
|*  RAMdisk driver for filesystem - 32k paged variant for weatherstation
|*
\*****************************************************************************/

#include <limits.h>
#include <string.h>
#include "common/filesystem/filesys.h"
#include "common/filesystem/diskdrv.h"

#define diskimg ((__xdata Uint8 *)0x8000)
#define banksel (*((__sfr Uint8 *) 0xD1))

#ifndef MIN
#define MIN( A, B ) ( ((A)<(B))) ? (A) : (B) )
#endif

/*****************************************************************************\
|*
|*  FUNCTION:       drv_clear
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     addr = Physical address to start writing
|*                  size = number of bytes to write
|*
|*  RETURN VALUE:   0 = none
|*
|*  DESCRIPTION:
|*
|*  Clear (part of) the drive
|*
*/

void drv_clear(Uint32 addr, Uint32 size)
{
    Uint8  bank;
    Uint16 offset;

    if (addr < FS_DISKSIZE)
    {
        if (addr + size >= FS_DISKSIZE)
        {
            size = FS_DISKSIZE - addr;
        }
        offset = addr & 0x7FFF;
        bank = (addr >> 15);
        banksel = bank;

        while (size--)
        {
            if (offset & 0x8000)
            {
                offset &= 0x7FFF;
                banksel = ++bank;
            }
            diskimg[offset++] = 0;
        }
    }
}

/*****************************************************************************\
|*
|*  FUNCTION:       drv_write
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     addr = Physical address to start writing
|*                  buf = Contents to write
|*                  size = number of bytes to write
|*
|*  RETURN VALUE:   0 = success, -1 otherwise
|*
|*  DESCRIPTION:
|*
|*  Write data to the drive
|*
*/

Sint16 drv_write(Uint32 addr, void * buf, Uint16 size)
{
    Uint8    bank;
    Uint16   offset;
    Uint8  * ptr = buf;
    if (addr >= FS_DISKSIZE)
    {
        return - 1;
    }
    else
    {
        if ((addr + size) > FS_DISKSIZE)
        {
            size = (Uint16)(FS_DISKSIZE - addr);
        }
        offset = addr & 0x7FFF;
        bank = (addr >> 15);
        banksel = bank;
        while (size--)
        {
            if (offset & 0x8000)
            {
                offset &= 0x7FFF;
                banksel = ++bank;
            }
            diskimg[offset++] = * ptr++;
        }
    }
    return size;
}

/*****************************************************************************\
|*
|*  FUNCTION:       drv_read
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     addr = Physical address to start reading
|*                  buf = Buffer to place data in, large enough to hold "size" bytes
|*                  size = number of bytes to read
|*
|*  RETURN VALUE:   0 = success, -1 otherwise
|*
|*  DESCRIPTION:
|*
|*  Read data from the drive
|*
*/

Sint16 drv_read(Uint32 addr, void * buf, Uint16 size)
{
    Uint8    bank;
    Uint16   offset;
    Uint8  * ptr = buf;
    if (addr >= FS_DISKSIZE)
    {
        return - 1;
    }
    else
    {
        if ((addr + size) > FS_DISKSIZE)
        {
            size = (Uint16)(FS_DISKSIZE - addr);
        }
        offset = addr & 0x7FFF;
        bank = (addr >> 15);
        banksel = bank;

        while (size--)
        {
            if (offset & 0x8000)
            {
                offset &= 0x7FFF;
                banksel = ++bank;
            }
            * ptr++ = diskimg[offset++];
        }
    }
    return 0;
}

/*****************************************************************************\
|*
|*  FUNCTION:       drv_sync
|*
|*  AVAILABILITY:   GLOBAL | LOCAL
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



