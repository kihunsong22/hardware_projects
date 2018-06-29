/*************************************************************************
**
**  VERSION CONTROL:	@(#)ftp_server.h	1.2	03/10/31
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Demo application: simple FTP server
**
**************************************************************************/

#include "tcpipset.h"
#include "common/tcpip_global.h"

#ifndef _FTP_SERVER_H_
#define _FTP_SERVER_H_

#include "common/filesystem/filesys.h"

//**************************************************************************

#ifndef FTP_SUBDIR_MAXDEPTH
#define FTP_SUBDIR_MAXDEPTH 5
#endif

#ifndef FTP_SESSIONS
#define FTP_SESSIONS 2
#endif

#define FTP_CWD_MAXLENGTH (FTP_SUBDIR_MAXDEPTH * (FS_MAXFILENAMELEN + 1))

typedef struct
{
    TCP_SESSION *ctrl_session;
    TCP_SESSION *data_session;
    Uint16 ctrl_lastmsg;
    Uint8 data_ip[4];
    Uint16 data_port;
    FS_FCB fcb;
    FS_DIRENTRY data_direntry;
    Uint32 data_pos;
    Uint32 data_lastpos;
    char rootcwd[FTP_CWD_MAXLENGTH];
    char *cwd;
    char rnfr[FS_MAXFILENAMELEN + 1];
    Uint8 state;
    Uint8 rootlevel;
    Uint8 level;
}
FTP_SERVER_SESSION;

typedef struct
{
    FTP_SERVER_SESSION *ftpsession;
}
FTP_SERVER_CARGO;

//**************************************************************************

extern void ftp_init(void);
extern Uint16 CALLBACKMEMSPEC ftp_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags);

//**************************************************************************

#endif
