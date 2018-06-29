/*****************************************************************************\
|*
|*  VERSION CONTROL:	@(#)filesys.h	1.2	03/10/31
|*
|*  IN PACKAGE:		TCP/IP
|*
|*  COPYRIGHT:		Copyright (c) 2002, Altium BV
|*
|*  DESCRIPTION:
|*
|*	File system interface
|*
\*****************************************************************************/

#include "tcpipset.h"

#ifndef _FILESYS_H
#define _FILESYS_H

//**************************************************************************
/*
 * Disk definition
 */

#ifndef FS_DISKSIZE
#define FS_DISKSIZE	8192	// Default disk size = 8192 bytes
#endif

#ifndef FS_CLUSTERSIZE
#define FS_CLUSTERSIZE	256	// Default clustersize = 256 bytes
#endif

#ifndef FS_ROOTENTRIES
#define FS_ROOTENTRIES	(FS_CLUSTERSIZE / sizeof( FS_DIRENTRY ))
#endif

#ifndef FS_MAXFILENAMELEN
#define FS_MAXFILENAMELEN	9	// Default maximum file name length (any arbitrary number, must be less than 1 cluster for a FSREC)
#endif

/*
 * Optimizations and debug flags
 */

#ifndef RAM
#define RAM	// Define to _near for example to get better optimization
#endif

//#define DEBUG_FILESYS

/*
 * Type definitions
 */

// FAT16: a cluster contains max. 32768 bytes, maximal 65536 clusters

typedef Uint16 FS_CLUSTER;
typedef Uint16 FS_OFFSET;
typedef Uint32 FS_LINEAR;

// Directory entry

typedef struct FS_DIRENTRY
{
    Uint32 filesize;	// Size of file
    FS_CLUSTER cluster;	// Start cluster
    Uint8 status;	// Status byte...
    char name[FS_MAXFILENAMELEN];	// File or directory name
}
FS_DIRENTRY;

/*
 * Contents of the status byte
 */

#define FS_ATTR_RO		0x01	// ReadOnly
#define FS_ATTR_DIR		0x02	// Directory
#define FS_ATTR_FILE		0x04	// Regular file
#define FS_ATTR_LINK		0x08	// Just a soft link
#define FS_ATTR_VOL		0x10	// Volume label (per definition 1st entry of root only)
#define FS_ATTR_RESERVED1	0x40	// Reserved for internal use
#define FS_ATTR_RESERVED2	0x80	// Reserved for internal use

// Logical disk addresses

typedef union FS_ADDR
{
    FS_LINEAR linear;	// Assuming a 32-bit int and copying this field is more efficient than a structure copy
    struct
    {
	FS_OFFSET offset;
	FS_CLUSTER cluster;
    }
    seg;
}
FS_ADDR;

// File control block, must be provided by callee
typedef struct FS_FCB FS_FCB;
typedef RAM FS_FCB *FS_HANDLE;
struct FS_FCB
{
    FS_ADDR direntry;
    FS_ADDR diskpos;
    Uint32 filepos;
    Uint32 filesize;
    FS_HANDLE next;
    Uint8 status;
};


/*
 * Values for whence argument in fs_seek:
 */

typedef enum
{ FS_SEEK_START, FS_SEEK_CURRENT, FS_SEEK_END }
FS_SEEK;

/*
 * Error indication return values
 */

#define FS_ERR_API	-1	// Error in parameters or other API clash
#define FS_ERR_NOTOPEN	-2	// File not open
#define FS_ERR_NOSPACE	-3	// Out of disk space
#define FS_ERR_NOTFOUND	-4	// File not found
#define FS_ERR_DENIED	-5	// Access denied
#define FS_ERR_NOTEMPTY	-6	// Directory not empty while trying to remove

/*
 * User API
 */

#ifndef FS_READONLY
extern int fs_create(FS_HANDLE handle, char *root, char *path, char *name);	// Create a file for writing
extern int fs_mkdir(char *root, char *path, char *name);	// Create a directory
#endif // FS_READONLY
extern int fs_open(FS_HANDLE handle, char *root, char *path, char *name);	// Open a file
extern void fs_close(FS_HANDLE handle);	// Close an open file
#ifndef FS_READONLY
extern int fs_remove(char *root, char *path, char *name);	// Remove a file
extern void fs_format(char *vollabel);	// Format the drive
extern int fs_rmdir(char *root, char *path, char *name);	// Remove a directory
extern int fs_rename(char *root, char *path, char *oldname, char *newname);	// Rename a file
#endif // FS_READONLY

extern Uint32 fs_seek(FS_HANDLE handle, Sint32 offset, FS_SEEK whence);	// Seek to a specified position in a file
#ifndef FS_READONLY
extern int fs_write(FS_HANDLE handle, void *buf, Sint16 size);	// Write a block of data to the file
#endif // FS_READONLY
extern int fs_read(FS_HANDLE handle, void *buf, Sint16 size);	// Read a block of data from the file

extern int fs_findfirst(FS_HANDLE handle, char *root, char *path, FS_DIRENTRY * dir);	// Find first entry of a directory
extern int fs_findnext(FS_HANDLE handle, FS_DIRENTRY * dir);	// Find next entry of the directory
extern void fs_closedir(FS_HANDLE handle);	// Close directory after searching
extern Uint16 fs_stat(char *root, char *path, char *name);	// Get file status

#endif // _FILESYS_H
