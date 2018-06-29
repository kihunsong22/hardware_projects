/*****************************************************************************\
|*
|*  VERSION CONTROL:	@(#)filesys.c	1.4	03/09/17
|*
|*  IN PACKAGE:
|*
|*  COPYRIGHT:      Copyright (c) 2002, Altium BV
|*
|*  DESCRIPTION:
|*
|*  File system support
|*
|*  TODO:
|*
|*  - Current directory support instead of seperate "root" and "path" arguments in fs_open and friends
|*
|*  CONTROLS:
|*
|*  FS_OPTIM_STRUCTCOPY
|*      Setting this will optimize FS_ADDR structure copied by copying the linear variable.
|*      Note: this requires volatile-like behavior on union elements
|*
|*  FS_OPTIM_STACK
|*      Minimizes stack usage by not caching some structures on the stack. The trade-off is
|*      in both, codesize and executionspeed (on most platforms, that is)
|*
|*  FS_READONLY
|*      Exclude write actions from compilation
|*
|*  FS_CURDIR
|*      All functions that take a path require a virtual root specification and a path spec.
|*      When this symbol is set, the system requires a cluster number as root spec. This
|*      cluster number can be retrieved by calling fs_setdir().
|*
|*  DEBUG_FILESYS
|*      Sets debugging options. Currently, this will enable extra runtime checks only.
|*
|*  FS_MULTITHREAD
|*      Add multithreading support for RTOS's
|*      Note: needs OS_ENTER_CRITICAL, OS_LEAVE_CRITICAL, OS_SUSPEND functions
|*            If OS_LEAVE_CRITICAL implicitly calls OS_SUSPEND, the latter may be left empty
|*
\*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesys.h"
#include "diskdrv.h"

#define FS_CLUSTERS (FS_DISKSIZE / FS_CLUSTERSIZE)
#define FS_ROOTSIZE (FS_ROOTENTRIES * sizeof( FS_DIRENTRY ))
#define FS_ATTR_WRMODE  FS_ATTR_RESERVED2
#define FS_ATTR_ROOT    FS_ATTR_RESERVED1

#define LINEAR( A ) ((Uint32)((A).cluster) * FS_CLUSTERSIZE + (A).offset )
#ifndef MIN
#define MIN( A, B ) (((A) < (B)) ? (A) : (B))
#endif

#ifdef FS_OPTIM_STRUCTCOPY
#define FS_COPY( A, B ) (A).linear = (B).linear
#else
#define FS_COPY( A, B ) (A) = (B)
#endif

FS_HANDLE fcblist = (FS_HANDLE) NULL;

/*
 * Lock values for critical regions
 */

#define FS_LOCK_ALL     0x03

#define FS_LOCK_LOOKUP      0x01
#define FS_LOCK_CLUSTER 0x02

#if defined( FS_MULTITHREAD ) && !defined( FS_READONLY )

#ifndef OS_SUSPEND
#define OS_SUSPEND()
#endif

#ifndef OS_ENTER_CRITICAL
#error  Definition of os_enter_critical() is missing
#endif

#ifndef OS_LEAVE_CRITICAL
#error  Definition of os_leave_critical() is missing
#endif

static volatile Uint8 sem = 0;
void FS_ENTER_CRITICAL(Uint8 lock)
{
	OS_ENTER_CRITICAL();
	while (sem & lock)
	{
		OS_LEAVE_CRITICAL();
		OS_SUSPEND();
		OS_ENTER_CRITICAL();
	}
	sem |= lock;
	OS_LEAVE_CRITICAL();
}

void FS_LEAVE_CRITICAL(Uint8 lock)
{
	sem &= (Uint8) ~ lock;
	OS_SUSPEND();
}

#else
#define FS_ENTER_CRITICAL( A )
#define FS_LEAVE_CRITICAL( A )
#endif

/*****************************************************************************\
|*
|*  FUNCTION:       fs_get_fat
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     cluster = cluster number to read
|*
|*  RETURN VALUE:   value from FAT[cluster]
|*
|*  DESCRIPTION:
|*
|*  Read the FAT at index "cluster"
|*
 */

static FS_CLUSTER fs_get_fat(FS_CLUSTER cluster)
{
	drv_read(cluster * sizeof(cluster), &cluster, sizeof(cluster));
	return cluster;
}

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_put_fat
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     cluster = cluster number to write
|*          value = value to write on FAT[cluster]
|*
|*  RETURN VALUE:   None
|*
|*  DESCRIPTION:
|*
|*  Write a new calue on the FAT at index "cluster"
|*
 */

static void fs_put_fat(FS_CLUSTER cluster, FS_CLUSTER value)
{
	drv_write(cluster * sizeof(FS_CLUSTER), &value, sizeof(FS_CLUSTER));
}
#endif // FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     addr = pointer to address of directory entry
|*
|*  RETURN VALUE:
|*
|*  DESCRIPTION:
|*
|*
|*
 */

static FS_HANDLE fs_findlock(FS_ADDR addr)
{
	FS_HANDLE handle;
	FS_ENTER_CRITICAL(FS_LOCK_LOOKUP);
	for (handle = fcblist; handle; handle = handle->next)
	{
		if (handle->direntry.linear == addr.linear)
			break;
	}
	FS_LEAVE_CRITICAL(FS_LOCK_LOOKUP);
	return handle;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_lock
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     handle = pointer to FCB to lock
|*
|*  RETURN VALUE:   0: success, -1 otherwise
|*
|*  DESCRIPTION:
|*
|*  Lock an FCB
|*
 */

static int fs_lock(FS_HANDLE handle)
{
	FS_HANDLE loop;
	FS_ENTER_CRITICAL(FS_LOCK_LOOKUP);
	for (loop = fcblist; loop; loop = loop->next)
	{
		if (loop == handle)
			return -1;
	}
	handle->next = fcblist;
	fcblist = handle;
	FS_LEAVE_CRITICAL(FS_LOCK_LOOKUP);
	return 0;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_unlock
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     addr = pointer to address of directory entry
|*
|*  RETURN VALUE:   None
|*
|*  DESCRIPTION:
|*
|*  Find handle and unlock it (remove it from the FCB list)
|*
 */

static void fs_unlock(FS_HANDLE handle)
{
	FS_HANDLE prev;
	FS_HANDLE tmp;
	FS_ENTER_CRITICAL(FS_LOCK_LOOKUP);
	for (prev = (FS_HANDLE) NULL, tmp = fcblist; tmp; tmp = tmp->next)
	{
		if (tmp == handle)
		{
			if (prev)
			{
				prev->next = tmp->next;
			}
			else
			{
				fcblist = tmp->next;
			}
			break;
		}
	}
	FS_LEAVE_CRITICAL(FS_LOCK_LOOKUP);
}

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_alloc_cluster
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     parent = Cluster to allocate new cluster from
|*
|*  RETURN VALUE:   Number of newly allocated cluster or 0 on error
|*
|*  DESCRIPTION:
|*
|*  Allocate a new cluster and add it to the linked list of clusters for current object
|*  Note: the FAT entry of the allocated cluster contains a pointer to itself on exit
|*
 */

static FS_CLUSTER fs_alloc_cluster(FS_CLUSTER parent)
{
	FS_CLUSTER cluster;
	FS_ENTER_CRITICAL(FS_LOCK_CLUSTER);
	for (cluster = FS_CLUSTERS - 1; cluster; cluster--)
	{
		if (!fs_get_fat(cluster))
		{
			fs_put_fat(cluster, cluster);
			if (parent)
				fs_put_fat(parent, cluster);
			break;
		}
	}
	FS_LEAVE_CRITICAL(FS_LOCK_CLUSTER);
	return cluster;
}
#endif // FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:       fs_get
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     addr = pointer to address to read from
|*          buf = pointer to memory to read into
|*          size = number of bytes to read
|*              if negative, addr will not be updated!
|*
|*  RETURN VALUE:   number of bytes that where NOT read because of cluster overflow
|*          thus: return value of 0 means everything worked
|*          on return, addr contains address just after the read bytes
|*
|*  DESCRIPTION:
|*
|*  Read from device, using FAT entries to determine continuation chain.
|*  A return value of 0 means everything worked.
|*  Note: this reads until the end of a cluster, even if the actual file is not that large!
|*  Possible errors that are checked for:
|*      buf is invalid
|*      addr is invalid
|*      addr->seg.cluster = 0 (== empty file)
|*      Trying to read pas logical end of file
 */

static Uint16 fs_get(FS_ADDR * addr, void *buf, Sint16 size)
{
	FS_OFFSET chunk;
	FS_ADDR cache;
	Uint1 update;

#ifdef DEBUG_FILESYS
	if (addr && buf && addr->seg.cluster)
#endif
	{
		if (size < 0)
		{
			update = 0;
			size = -size;
		}
		else
		{
			update = 1;
		}
		FS_COPY(cache, *addr);
		while (size > 0)
		{
			if (cache.seg.offset == FS_CLUSTERSIZE)
			{
				chunk = fs_get_fat(cache.seg.cluster);
				if (cache.seg.cluster == chunk)
					break;	// End of file reached!
				cache.seg.cluster = chunk;
				cache.seg.offset = 0;
			}
			chunk = MIN(FS_CLUSTERSIZE - cache.seg.offset, size);
			drv_read(LINEAR(cache.seg), buf, chunk);
			cache.seg.offset += chunk;
			buf = (Uint8 *) buf + chunk;
			size -= chunk;
		}
		if (update)
			FS_COPY(*addr, cache);
	}
	return size;
}

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_put
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     addr = address to start writing
|*          buf = pointer to data to write
|*          size = number of bytes to write (negative means: do not update write address pointers)
|*
|*  RETURN VALUE:   0 = okay, number of bytes NOT written otherwise
|*          on exit, addr points to new position
|*
|*  DESCRIPTION:
|*
|*  Write to a file. Allocates new clusters if at end of file.
|*
 */

static Uint16 fs_put(FS_ADDR * addr, void *buf, Sint16 size)
{
	FS_OFFSET chunk;
	FS_ADDR cache;

#ifdef DEBUG_FILESYS
	if (addr && buf)
#endif
	{
		FS_COPY(cache, *addr);
		if (size < 0)
		{
			size = -size;
			addr = NULL;
		}
		if (cache.seg.cluster == 0)	// Empty file
		{
			cache.seg.cluster = fs_alloc_cluster(0);
			cache.seg.offset = 0;
		}

		if (cache.seg.cluster)
		{

			while (size > 0)
			{
				if (cache.seg.offset == FS_CLUSTERSIZE)
				{
					chunk = fs_get_fat(cache.seg.cluster);
					if ((cache.seg.cluster == chunk) && (chunk = fs_alloc_cluster(chunk), !chunk))
						break;
					cache.seg.cluster = chunk;
					cache.seg.offset = 0;
				}
				chunk = MIN(FS_CLUSTERSIZE - cache.seg.offset, size);
				drv_write(LINEAR(cache.seg), buf, chunk);
				cache.seg.offset += chunk;
				buf = (Uint8 *) buf + chunk;
				size -= chunk;
			}
		}

		if (addr)
			FS_COPY(*addr, cache);
	}

	return size;
}
#endif // FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:       fs_findentry
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     *addr = address to start searching (offset is forced to zero before reading)
|*          name = name of item to be found
|*          *dir = return container. On entry, dir->filesize should contain
|*              the size of the directory we're searching through.
|*
|*  RETURN VALUE:   0 = okay, FS_ERR_NOTFOUND if the entry could not be found
|*          *addr = address where directory entry was found (valid if return value = 0 only)
|*          *dir = directory entry
|*
|*  DESCRIPTION:
|*
|*  Locate a directory entry in the current directory. The directory must start at the address
|*  pointed to by "addr.seg.cluster".
|*
|*  Note: we don't want to use dynamic memory or statically allocated return values. Thus, do NOT
|*      change the function interface to something like "FS_DIRENTRY * fs_findentry(...)".
|*
 */

static int fs_findentry(FS_ADDR * addr, char *name, FS_DIRENTRY * dir)
{
	Uint32 i;
#ifdef DEBUG_FILESYS
	if (!addr || !name)
		return FS_ERR_API;
#endif
	if (strlen(name) > FS_MAXFILENAMELEN)
		return FS_ERR_NOTFOUND;
	addr->seg.offset = 0;

	for (i = dir->filesize; i; i -= sizeof(FS_DIRENTRY))
	{
		FS_ADDR temp;
		FS_COPY(temp, *addr);
		fs_get(addr, dir, sizeof(*dir));
		if (dir->status && !strncmp(dir->name, name, FS_MAXFILENAMELEN))
		{
			FS_COPY(*addr, temp);
			return 0;
		}
	}
	return FS_ERR_NOTFOUND;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_findpath
|*
|*  AVAILABILITY:   LOCAL
|*
|*  PARAMETERS:     cluster = cluster number to start searching. If zero, assume searching from root directory
|*          path = name of path to search for
|*          dir = container to directory entry storage. dir.filesize should contain size of current directory.
|*
|*  RETURN VALUE:   Address of path when found. If not found, fs_findpath().cluster is 0.
|*
|*  DESCRIPTION:
|*
|*  Find the directory pointed to by "path" in the current directory which is starting at "cluster".
|*
 */

static const FS_DIRENTRY fs_root_dir = {
	FS_ROOTSIZE,	// Filesize
	0,	// Cluster (to be defined at runtime)
	FS_ATTR_DIR | FS_ATTR_ROOT,	// Status: root directory
	"/"	// Name is '/'
};

static FS_ADDR fs_findpath(FS_CLUSTER cluster, char *path, FS_DIRENTRY * dir)
{
	Uint16 i;
	FS_ADDR addr;
	char name[FS_MAXFILENAMELEN];

	addr.seg.cluster = cluster;
	addr.seg.offset = 0;

	if (cluster == 0)
	{
		// Start search from root directory
		*dir = fs_root_dir;
		dir->cluster = addr.seg.cluster = cluster = fs_get_fat(0);
	}

	if (path)
	{
		do
		{
			while (*path == '/')
				path++;	// Skip all preceeding directory seperators
			if (*path == '\0')
				break;	// Found it, we're done

			addr.seg.cluster = cluster;
			addr.seg.offset = 0;

			for (i = 0; i < FS_MAXFILENAMELEN - 1; i++)	// Search for directory seperator
			{
				if ((*path == '\0') || (*path == '/'))
					break;
				name[i] = *path++;
			}

			name[i] = '\0';	// Should contain name of file now
			if (name[0] == '.' && (i == 1))
				continue;	// Skip single dot entries

			if ((fs_findentry(&addr, name, dir) == FS_ERR_NOTFOUND) || ((dir->status & FS_ATTR_DIR) == 0))
			{
				// Entry not found or no subdirectory entry
				addr.seg.cluster = 0;
				break;
			}
			else if (dir->status & FS_ATTR_LINK)
			{
				drv_read(dir->filesize, dir, sizeof(*dir));
				cluster = dir->cluster;
			}
			else
			{
				cluster = dir->cluster;
			}
			while (*path && *path != '/')
				path++;
		}
		while (*path);
	}
	else
	{
		// No path or root signified, just copy the data and be done with it
		addr.seg.cluster = cluster;
		addr.seg.offset = 0;
	}
	return addr;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_open
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     handle = pointer to FCB used for future referencing this file
|*          root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where file should be, relative from application root
|*          name = name of file to search for
|*
|*  RETURN VALUE:   0: OK, error code otherwise. Handle->status updated to reflect the result
|*
|*  DESCRIPTION:
|*
|*  Open a file for reading and/or writing
|*
 */

#ifdef FS_CURDIR
int fs_open(FS_HANDLE handle, FS_CLUSTER root, char *path, char *name)
#else
int fs_open(FS_HANDLE handle, char *root, char *path, char *name)
#endif
{
	FS_ADDR addr;
	FS_DIRENTRY dir;

#ifdef DEBUG_FILESYS
	if (!handle)
		return FS_ERR_API;
#endif

	handle->status = 0;

#ifdef FS_CURDIR
	if (fs_findpath(root, path, &dir).seg.cluster)
#else
	if (fs_findpath(0, root, &dir).seg.cluster && fs_findpath(dir.cluster, path, &dir).seg.cluster)
#endif
	{
		addr.seg.cluster = dir.cluster;
		addr.seg.offset = 0;
		if ((fs_findentry(&addr, name, &dir) == FS_ERR_NOTFOUND) || (dir.status != FS_ATTR_FILE))
		{
			return FS_ERR_NOTFOUND;
		}
		if (fs_lock(handle))
		{
			return FS_ERR_DENIED;
		}

#ifdef FS_OPTIM_STACK
		FS_COPY(handle->direntry, addr);
		handle->diskpos.seg.cluster = dir.cluster;
		handle->diskpos.seg.offset = 0;
		handle->filepos = 0;
		handle->filesize = dir.filesize;
		handle->status = dir.status;
#else
		// Optimize by avoiding excessive pointer dereferencing
		{
			FS_FCB cache;
			FS_COPY(cache.direntry, addr);
			cache.diskpos.seg.cluster = dir.cluster;
			cache.diskpos.seg.offset = 0;
			cache.filepos = 0;
			cache.filesize = dir.filesize;
			cache.status = dir.status;
			cache.next = handle->next;
			*handle = cache;
		}
#endif
		return 0;
	}
	else
	{
		return FS_ERR_NOTFOUND;
	}
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_close
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     handle = pointer to FCB of file
|*
|*  RETURN VALUE:
|*
|*  DESCRIPTION:
|*
|*  Close the file by synchronizing the file system and closing (freeing) the handle
|*
 */

void fs_close(FS_HANDLE handle)
{
#ifndef FS_READONLY
	FS_DIRENTRY dir;
	Uint32 filesize;
#endif // FS_READONLY

#ifdef DEBUG_FILESYS
	if (handle)
#endif
	{
#ifndef FS_READONLY
		if (handle->status & FS_ATTR_WRMODE)
		{
			fs_get(&handle->direntry, &dir, -(Sint16) sizeof(dir));
			filesize = handle->filesize;
			if (filesize > dir.filesize)
			{
				dir.filesize = filesize;
				fs_put(&handle->direntry, &dir, sizeof(dir));
			}
		}
		drv_sync();
#endif // FS_READONLY
		fs_unlock(handle);
		handle->status = 0;
	}
}

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_create
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     handle = file handle to be used
|*          root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where file should be created, relative from application root
|*          name = name of file to create
|*
|*  RETURN VALUE:   0 = okay, error code otherwise
|*
|*  DESCRIPTION:
|*
|*  Create a new file. The following errors are possible:
|*      root directory not found
|*      path not found
|*      file or directory with same name already exists
|*      no room to expand directory or root directory full
|*
 */

#ifdef FS_CURDIR
int fs_create(FS_HANDLE handle, FS_CLUSTER root, char *path, char *name)
#else
int fs_create(FS_HANDLE handle, char *root, char *path, char *name)
#endif
{
	FS_ADDR parentaddr;
	FS_DIRENTRY parentdir;
	FS_ADDR addr;
	FS_DIRENTRY dir;
	Uint32 i;

#ifdef DEBUG_FILESYS
	if (!name)
		return FS_ERR_API;
#endif
	handle->status = 0;

	if ((strlen(name) <= (FS_MAXFILENAMELEN - 1)) && (*name != '.') &&
#ifdef FS_CURDIR
	    (FS_COPY(parentaddr, fs_findpath(root, path, &parentdir)), parent.seg.cluster))
#else
	    fs_findpath(0, root, &parentdir).seg.cluster &&
	    (FS_COPY(parentaddr, fs_findpath(parentdir.cluster, path, &parentdir)), parentaddr.seg.cluster))
#endif
	{
		// Path is found, check if file already exists
		addr.seg.cluster = parentdir.cluster;
		addr.seg.offset = 0;

		for (i = parentdir.filesize; i; i -= sizeof(dir))
		{
			fs_get(&addr, &dir, sizeof(dir));
			if ((dir.status & (FS_ATTR_FILE | FS_ATTR_DIR)) && !strncmp(name, dir.name, FS_MAXFILENAMELEN))
			{
				// Found entry with same name, do not create a new one
				handle = (FS_HANDLE) NULL;
				break;
			}
		}
		if (!i)
		{
			FS_ADDR tmp = { 0 };
			addr.seg.cluster = parentdir.cluster;
			addr.seg.offset = 0;
			// Unique file and we succeeded in allocating the handle. Go allocate directory entry
			for (i = parentdir.filesize; i; i -= sizeof(dir))
			{
				FS_COPY(tmp, addr);
				fs_get(&addr, &dir, sizeof(dir));
				if (dir.status == 0)
				{
					// Found an unused one, stop scanning
					FS_COPY(addr, tmp);
					break;
				}
			}

			// Prepare the directory entry to be written
			dir.cluster = fs_alloc_cluster(0);
			dir.filesize = 0;
			strncpy(dir.name, name, FS_MAXFILENAMELEN);
			dir.name[FS_MAXFILENAMELEN - 1] = '\0';
			dir.status = FS_ATTR_FILE;
			FS_COPY(tmp, addr);
			if (((parentdir.status & FS_ATTR_ROOT) && !i) || dir.cluster == 0
			    || fs_put(&addr, &dir, sizeof(dir)))
			{
				// Parentdirectory is root and root does not have free handles available...
				// ...or failed to write directory entry. Thus: free handle and quit
				handle = (FS_HANDLE) NULL;
			}
			else
			{
				// Directory entry written successfully, update the handle
				memset(handle, 0, sizeof(*handle));
				FS_COPY(handle->direntry, tmp);
				handle->diskpos.seg.cluster = dir.cluster;
				handle->status = dir.status;
				handle->next = fcblist;
				fcblist = handle;
				if (!i)
				{
					// No empty directory entry was found, but we where able to write one anyway
					// Thus, the directory is expanded: update parent information
					parentdir.filesize += sizeof(dir);
					fs_put(&parentaddr, &parentdir, sizeof(parentdir));
				}
			}
		}
	}
	drv_sync();
	return handle ? 0 : FS_ERR_NOTOPEN;
}
#endif // FS_READONLY

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_mkdir
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where directory should be created, relative from application root
|*          name = name of directory to create
|*
|*  RETURN VALUE:   0: success, error value otherwise
|*
|*  DESCRIPTION:
|*
|*  Create a new directory
|*
 */

#ifdef FS_CURDIR
int fs_mkdir(FS_CLUSTER root, char *path, char *name)
#else
int fs_mkdir(char *root, char *path, char *name)
#endif
{
	FS_DIRENTRY dir;
	FS_FCB temp;
	FS_ADDR addr;
	int retval;

#ifdef DEBUG_FILESYS
	if (!name)
		return FS_ERR_API;
#endif
#ifdef FS_CURDIR
	FS_COPY(addr, fs_findpath(root, path, &dir));
#else
	FS_COPY(addr, fs_findpath(fs_findpath(0, root, &dir).seg.cluster, path, &dir));
#endif
	if (retval = fs_create((FS_HANDLE) & temp, root, path, name), !retval)
	{
		// First, write the backlink entry
		dir.filesize = 0;
		dir.cluster = addr.seg.cluster;
		dir.status = FS_ATTR_DIR | FS_ATTR_LINK;
		dir.filesize = LINEAR(addr.seg);
		strcpy_rom(dir.name, "..");

		if (fs_put(&temp.diskpos, &dir, sizeof(dir)))
		{
			// ".." could not be written, failure!
			dir.filesize = 0;
			FS_COPY(addr, temp.direntry);
			fs_put(&addr, &dir, sizeof(dir));
			fs_unlock((FS_HANDLE) & temp);
			retval = FS_ERR_NOSPACE;
		}
		else
		{
			// Update new entry: it's a directory instead of a file
			FS_COPY(addr, temp.direntry);
			fs_get(&addr, &dir, -(Sint16) sizeof(dir));
			dir.status = FS_ATTR_DIR;
			dir.filesize = sizeof(dir);
			fs_put(&addr, &dir, sizeof(dir));
			fs_unlock((FS_HANDLE) & temp);
		}
	}

	drv_sync();
	return retval;
}
#endif // FS_READONLY

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_remove
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where file lives, relative from application root
|*          name = name of file to remove
|*
|*  RETURN VALUE:   0 = success, error value otherwise
|*
|*  DESCRIPTION:
|*
|*  Remove an existing file
|*
 */

#ifdef FS_CURDIR
int fs_remove(FS_CLUSTER root, char *path, char *name)
#else
int fs_remove(char *root, char *path, char *name)
#endif
{
	FS_DIRENTRY dir;
	FS_ADDR addr;
	FS_CLUSTER cluster;
	FS_CLUSTER next;

#ifdef DEBUG_FILESYS
	if (!name)
		return FS_ERR_API;
#endif

#ifdef FS_CURDIR
	if ((FS_COPY(addr, fs_findpath(root, path, &dir)), addr.seg.cluster) &&
#else
	if (fs_findpath(0, root, &dir).seg.cluster &&
	    (FS_COPY(addr, fs_findpath(dir.cluster, path, &dir)), addr.seg.cluster) &&
#endif
	    (addr.seg.cluster = dir.cluster, addr.seg.offset = 0, fs_findentry(&addr, name, &dir) == 0) &&
	    (dir.status & FS_ATTR_FILE))
	{
		dir.status = 0;
		drv_write(LINEAR(addr.seg), &dir, sizeof(dir));
		for (cluster = dir.cluster; cluster; cluster = next)
		{
			next = fs_get_fat(cluster);
			fs_put_fat(cluster, 0);
			if (cluster == next)
				break;
		}
		return 0;
	}
	else
	{
		return FS_ERR_NOTFOUND;
	}

}
#endif // FS_READONLY

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_rmdir
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where directory lives, relative from application root
|*          name = name of directory to remove
|*
|*  RETURN VALUE:   0 = success, error value otherwise
|*
|*  DESCRIPTION:
|*
|*  Remove an existing directory. This directory must be enpty or it will fail!
|*
 */


#ifdef FS_CURDIR
int fs_rmdir(FS_CLUSTER root, char *path, char *name)
#else
int fs_rmdir(char *root, char *path, char *name)
#endif
{
	FS_ADDR addr;
	FS_ADDR tmp;
	FS_DIRENTRY dir;
	FS_LINEAR i;
	FS_CLUSTER cluster, next;

#ifdef FS_CURDIR
	if (name && fs_findpath(root, path, &dir).seg.cluster)
#else
	if (name && fs_findpath(0, root, &dir).seg.cluster && fs_findpath(dir.cluster, path, &dir).seg.cluster)
#endif
	{
		addr.seg.cluster = dir.cluster;
		addr.seg.offset = 0;
		if (fs_findentry(&addr, name, &dir) == FS_ERR_NOTFOUND)
			return FS_ERR_NOTFOUND;
		if (dir.status != FS_ATTR_DIR)
			return FS_ERR_DENIED;

		FS_ENTER_CRITICAL(FS_LOCK_CLUSTER);

		tmp.seg.cluster = dir.cluster;
		tmp.seg.offset = sizeof(dir);
		for (i = dir.filesize - sizeof(dir); i; i -= sizeof(dir))
		{
			fs_get(&tmp, &dir, sizeof(dir));
			if (dir.status)
			{
				FS_LEAVE_CRITICAL(FS_LOCK_CLUSTER);
				return FS_ERR_NOTEMPTY;
			}
		}
		if (fs_findlock(addr))
		{
			FS_LEAVE_CRITICAL(FS_LOCK_CLUSTER);
			return FS_ERR_DENIED;
		}

		fs_get(&addr, &dir, -(Sint16) sizeof(dir));
		dir.status = 0;
		fs_put(&addr, &dir, sizeof(dir));
		for (next = 0, cluster = dir.cluster; cluster != next; cluster = next)
		{
			next = fs_get_fat(cluster);
			fs_put_fat(cluster, 0);
		}
		FS_LEAVE_CRITICAL(FS_LOCK_CLUSTER);
		return 0;
	}
	else
	{
		return FS_ERR_NOTFOUND;
	}
}
#endif // FS_READONLY

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_format
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:
|*
|*  RETURN VALUE:
|*
|*  DESCRIPTION:
|*
|*  Format filesystem (clear FAT, and setup the root directory)
|*
 */

void fs_format(char *vollabel)
{
	FS_CLUSTER i;
	FS_CLUSTER fat;
	FS_LINEAR addr;
	FS_DIRENTRY root = { 0, 0, FS_ATTR_VOL, "" };

	FS_ENTER_CRITICAL(FS_LOCK_ALL);

	/* The size of the FAT is CLUSTERS words. These first clusters must be marked as "unavailable" */
	addr = 0;

	for (i = 0; i <= (FS_CLUSTERS * sizeof(fat) - 1) / (FS_CLUSTERSIZE); i++)
	{
		fs_put_fat(i, 0xFFFF);
		addr += sizeof(fat);
	}

	/*
	 * We know that the very first cluster is always part of the FAT.
	 * We mis-use that first entry by having it pointing to the start of the root directory
	 */

	fat = i++;
	drv_write(0, &fat, sizeof(fat));
	drv_write(addr, &fat, sizeof(fat));
	root.cluster = fat;
	addr += sizeof(fat);

	/*
	 * The rest of the drive is free, mark as such:
	 */
	fat = 0;
	while (i++ < FS_CLUSTERS)
	{
		drv_write(addr, &fat, sizeof(fat));
		addr += sizeof(fat);
	}

	/*
	 * Initialize the root directory
	 */

	drv_clear(root.cluster * FS_CLUSTERSIZE, FS_ROOTSIZE);
	if (vollabel != NULL)
		strncpy(root.name, vollabel, FS_MAXFILENAMELEN);
	drv_write(root.cluster * FS_CLUSTERSIZE, &root, sizeof(root));

	/*
	 * And we're done
	 */
	drv_sync();
	FS_LEAVE_CRITICAL(FS_LOCK_ALL);
}
#endif // FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:       fs_seek
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     handle = pointer to FCB
|*          offset = file position to seek to
|*          whence = seek relative definition (START, CURRENT, END)
|*
|*  RETURN VALUE:   >= 0: new position in file, error otherwise
|*
|*  DESCRIPTION:
|*
|*  Move file pointer to a new position in the file
|*
 */

Uint32 fs_seek(FS_HANDLE handle, Sint32 offset, FS_SEEK whence)
{
	FS_DIRENTRY dir;

#ifdef DEBUG_FILESYS
	if (handle)
#endif
	{
		fs_get(&handle->direntry, &dir, -(Sint16) sizeof(dir));
#ifndef FS_READONLY
		if (handle->filepos > dir.filesize)
		{
			dir.filesize = handle->filepos;
			fs_put(&handle->direntry, &dir, -(Sint16) sizeof(dir));
			drv_sync();
		}
#endif // FS_READONLY
		switch (whence)
		{
		case FS_SEEK_START:
			break;
		case FS_SEEK_CURRENT:
			offset += handle->filepos;
			break;
		case FS_SEEK_END:
			offset += dir.filesize;
		}
		if (offset >= (Sint32) dir.filesize)
			offset = (Sint32) dir.filesize;
		if (offset < 0)
			offset = 0;
		if (handle->filepos != offset)
		{
			handle->filepos = offset;
			while (offset > FS_CLUSTERSIZE)
			{
				dir.cluster = fs_get_fat(dir.cluster);
				offset -= FS_CLUSTERSIZE;
			}
			handle->diskpos.seg.cluster = dir.cluster;
			handle->diskpos.seg.offset = (FS_OFFSET) offset;
		}
	}
#ifdef DEBUG_FILESYS
	else
	{
		offset = FS_ERR_NOTOPEN;
	}
#endif
	return offset;
}

#ifndef FS_READONLY
/*****************************************************************************\
|*
|*  FUNCTION:       fs_write
|*
|*  AVAILABILITY:   GLOBAL | LOCAL
|*
|*  PARAMETERS:     handle = pointer to handle of file
|*          buf = pointer to databuffer to be written
|*          size = number of bytes to write
|*
|*  RETURN VALUE:   Number of bytes actually written or error code if negative
|*
|*  DESCRIPTION:
|*
|*  Write data to a file
|*
 */

int fs_write(FS_HANDLE handle, void *buf, Sint16 size)
{
	int res;

#ifdef DEBUG_FILESYS
	if (!handle)
		return FS_ERR_API;
#endif

	if ((handle->status & (FS_ATTR_FILE | FS_ATTR_RO)) == FS_ATTR_FILE)
	{
		handle->status |= FS_ATTR_WRMODE;	// Mark that we must update the directory entry on close
		res = size - fs_put(&handle->diskpos, buf, (Uint16) size);
		handle->filepos += res;
		if (handle->filepos > handle->filesize)
		{
			handle->filesize = handle->filepos;
		}
		return res;
	}
	else
	{
		return FS_ERR_DENIED;
	}
}
#endif // FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:       fs_read
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     handle = pointer to handle of file
|*          buf = pointer to databuffer to be read in
|*          size = number of bytes to read
|*
|*  RETURN VALUE:   Number of bytes actually read
|*          Error code if negative!
|*
|*  DESCRIPTION:
|*
|*  Read data from a previously opened file
|*
 */

int fs_read(FS_HANDLE handle, void *buf, Sint16 size)
{
	Uint32 res;
	Uint32 filepos;
	Uint32 filesize;

#ifdef DEBUG_FILESYS
	if (!handle)
		return FS_ERR_API;
#endif

	filesize = handle->filesize;
	filepos = handle->filepos;
	res = filesize - filepos;
	if (size > res)
	{
		size = res;
	}

	if (handle->status & FS_ATTR_FILE)
	{
		size -= fs_get(&handle->diskpos, buf, size);
		handle->filepos += size;
		return size;
	}
	else
	{
		return FS_ERR_NOTOPEN;
	}
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_findfirst
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path of directory to read, relative from application root
|*          dir = directory entry storage
|*
|*  RETURN VALUE:   0 on error, 1 otherwise
|*
|*  DESCRIPTION:
|*
|*  Find first entry of a directory
|*
 */

#ifdef FS_CURDIR
int fs_findfirst(FS_HANDLE handle, FS_CLUSTER root, char *path, FS_DIRENTRY * dir)
#else
int fs_findfirst(FS_HANDLE handle, char *root, char *path, FS_DIRENTRY * dir)
#endif
{
	FS_ADDR addr;
#ifdef DEBUG_FILESYS
	if (!handle)
		return FS_ERR_API;
#endif

	handle->status = 0;	// No need to close
#ifdef FS_CURDIR
	if (FS_COPY(addr, fs_findpath(root, path, dir)), addr.seg.cluster)
#else
	if (fs_findpath(0, root, dir).seg.cluster
	    && (FS_COPY(addr, fs_findpath(dir->cluster, path, dir)), addr.seg.cluster))
#endif // FS_CURDIR
	{
		FS_COPY(handle->direntry, addr);
		addr.seg.cluster = dir->cluster;
		addr.seg.offset = 0;
		handle->filesize = dir->filesize;
		if (!fs_get(&addr, dir, sizeof(*dir)))
		{
			handle->filepos = sizeof(*dir);
			FS_COPY(handle->diskpos, addr);
			FS_ENTER_CRITICAL(FS_LOCK_LOOKUP);
			handle->next = fcblist;
			fcblist = handle;
			FS_LEAVE_CRITICAL(FS_LOCK_LOOKUP);
			return 1;
		}
	}
	return 0;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_findnext
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     bookmark = marker to tell where we're at
|*          dir = directory entry storage
|*
|*  RETURN VALUE:   0 if no more files where found
|*
|*  DESCRIPTION:
|*
|*  Find next directory entry
|*
 */

int fs_findnext(FS_HANDLE handle, FS_DIRENTRY * dir)
{
	int i = 0;

#ifdef DEBUG_FILESYS
	if (!handle)
		return FS_ERR_API;
#endif
	while (handle->filepos < handle->filesize)
	{
		if (fs_get(&handle->diskpos, dir, sizeof(*dir)))
		{
			break;
		}
		handle->filepos += sizeof(*dir);

		if (dir->status)
		{
			i = 1;
			break;
		}
	}
	return i;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_closedir
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:
|*
|*  RETURN VALUE:
|*
|*  DESCRIPTION:
|*
|*  Close a directory
|*
 */

void fs_closedir(FS_HANDLE handle)
{
	fs_unlock(handle);
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_stat
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where object lives, relative from application root
|*          name = name of object
|*
|*  RETURN VALUE:   Status from object or 0 if not found
|*
|*  DESCRIPTION:
|*
|*  Return status of a directory entry
|*
 */

#ifdef FS_CURDIR
Uint16 fs_stat(FS_CLUSTER root, char *path, char *name)
#else
Uint16 fs_stat(char *root, char *path, char *name)
#endif
{
	FS_ADDR addr;
	FS_DIRENTRY dir;

#ifdef FS_CURDIR
	if (name && fs_findpath(root, path, &dir).seg.cluster)
#else
	if (name && fs_findpath(0, root, &dir).seg.cluster && fs_findpath(dir.cluster, path, &dir).seg.cluster)
#endif
	{
		addr.seg.cluster = dir.cluster;
		addr.seg.offset = 0;
		if (fs_findentry(&addr, name, &dir) == FS_ERR_NOTFOUND)
		{
			dir.status = 0;
		}
	}
	else
	{
		dir.status = 0;
	}
	return dir.status;
}

/*****************************************************************************\
|*
|*  FUNCTION:       fs_setdir
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     path = absolute path to be set as current directory
|*
|*  RETURN VALUE:   Starting cluster of path or 0 on error
|*
|*  DESCRIPTION:
|*
|*  Return cluster for a start path
|*
 */


#ifdef FS_CURDIR
FS_CLUSTER fs_setdir(char *path)
{
	FS_DIRENTRY dir;
	return fs_findpath(0, path, &dir).seg.cluster;
}
#endif

#ifndef FS_READONLY

/*****************************************************************************\
|*
|*  FUNCTION:       fs_rename
|*
|*  AVAILABILITY:   GLOBAL
|*
|*  PARAMETERS:     root = name of absolute application root (f.e. "/htdocs" for the HTTP deamon)
|*          path = path where object lives, relative from application root
|*          oldname = current name of object
|*          newname = new name of object
|*
|*  RETURN VALUE:   0 if successfull, error code otherwise
|*
|*  DESCRIPTION:
|*
|*  Rename a file or directory
|*
 */


int fs_rename(char *root, char *path, char *oldname, char *newname)
{
	FS_ADDR addr;
	FS_DIRENTRY dir;

	if (
#ifdef DEBUG_FILESYS
		oldname && newname &&
#endif
		strcmp_rom(oldname, "..") && strcmp_rom(newname, "..") &&
		fs_findpath(0, root, &dir).seg.cluster && fs_findpath(dir.cluster, path, &dir).seg.cluster)
	{
		addr.seg.cluster = dir.cluster;
		addr.seg.offset = 0;
		if (fs_findentry(&addr, oldname, &dir) != FS_ERR_NOTFOUND)
		{
			strncpy(dir.name, newname, sizeof(dir.name));
			fs_put(&addr, &dir, sizeof(dir));
			return 0;
		}
	}
	return FS_ERR_NOTFOUND;

}

#endif // FS_READONLY
