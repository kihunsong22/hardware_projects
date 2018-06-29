/*************************************************************************
**
**  VERSION CONTROL:    @(#)ftp_server.c	1.5	03/10/31
**
**  IN PACKAGE:     Embedded TCPIP
**
**  COPYRIGHT:      Copyright (c) 2002 Altium
**
**  DESCRIPTION:    Demo application: simple FTP server
**
**************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ftp_server.h"
#include "common/tcpip.h"

#ifndef OPTION_FTP_SERVER
#ifndef TCPIPDEBUG
#error OPTION_FTP_SERVER is not defined, but ftp_server.c is included in project
#endif
#endif

#ifdef OPTION_FTP_SERVER

//**************************************************************************

#ifdef DEBUG_FTP
#define debug_ftp(x) x
#else
#define debug_ftp(x)
#endif

//**************************************************************************

static ROMMEMSPEC char *ftpcommands[] = {
    "USER",
    "PASS",
    "PORT",
    "QUIT",
    "PWD",
    "CDUP",
    "RNFR",
    "RNTO",
    "TYPE",
    "MODE",
    "PASV",
    "RETR",
    "STOR",
    "DELE",
    "MKD",
    "RMD",
    "CWD",
    "NLST",
    "LIST",
    "XMKD",
    "XRMD",
    "XPWD",
    "XCUP",
    "XCWD",
    NULL
};

#define CMD_USER    1
#define CMD_PASS    2
#define CMD_PORT    3
#define CMD_QUIT    4
#define CMD_PWD     5
#define CMD_CDUP    6
#define CMD_RNFR    7
#define CMD_RNTO    8
#define CMD_TYPE    9
#define CMD_MODE    10
#define CMD_PASV    11
#define CMD_RETR    12
#define CMD_STOR    13
#define CMD_DELE    14
#define CMD_MKD     15
#define CMD_RMD     16
#define CMD_CWD     17
#define CMD_NLST    18
#define CMD_LIST    19
#define CMD_XMKD    20
#define CMD_XRMD    21
#define CMD_XPWD    22
#define CMD_XCUP    23
#define CMD_XCWD    24

#define CMD_WITH_PATH(cmd) ((cmd >= CMD_RETR) && (cmd <= CMD_LIST))
#define CMD_WITH_PATH_ONLY(cmd) ((cmd >= CMD_CWD) && (cmd <= CMD_LIST))

#define FTPSTATE_WAITUSER   1
#define FTPSTATE_WAITPASS   2
#define FTPSTATE_IDLE       3
#define FTPSTATE_LIST       4
#define FTPSTATE_NLST       5
#define FTPSTATE_RETR       6
#define FTPSTATE_STOR       7
#define FTPSTATE_MSGREADY   8
#define FTPSTATE_MSGABORT   9

static FTP_SERVER_SESSION ftpsessions[FTP_SESSIONS];

//**************************************************************************

static Uint16 CALLBACKMEMSPEC ftpdata_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags);
static char *uint32toa(Uint32 num, Uint8 pad);

/***************************************************************************
 * FUNCTION:    ftp_init
 *
 * Initialize the FTP module
 */
void ftp_init(void)
{
    memset(ftpsessions, 0, sizeof(ftpsessions));
}

/***************************************************************************
 * FUNCTION:    ftp_server
 *
 * FTP server application.
 *
 * resend   if > 0 the last packet has to be regenerated, -1 is external trigger
 * session  active TCP session
 * datalength   number of incoming bytes
 * tcp_flags    pointer to flags in TCP-header
 *
 * returns number of bytes answered
 */
Uint16 CALLBACKMEMSPEC ftp_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags)
{
    // TODO: optimize, i/q/cwd initial values are just to make the compiling warningfree
    FTP_SERVER_SESSION *ftpsession = session->cargo.ftp_server.ftpsession;
    static char rootcwd[FTP_CWD_MAXLENGTH + 1];	// not needed as static, but we don't want it on stack
    char *p;
    char *q = NULL;
    Uint16 msg = 0;
    ROMMEMSPEC char **cmd;
    Sint16 i = 0;
    char *cwd = NULL;

    if (resend > 0)
    {
	// resend the previous message
	// (-1 is not really a resend but a remote trigger from the data session)
	msg = ftpsession->ctrl_lastmsg;
    }
    else
    {
	if (session->state == TCP_STATE_SYNACK)
	{
	    // Find a free slot in our list of FTP sessions, in our TCP cargo we will
	    // just keep a pointer to the FTP session.
	    // The reason for doing our housekeeping in ftpsessions[] and not
	    // directly in the TCP cargo is the enormous amount of stuff we
	    // need to remember for our sessions
	    for (i = 0; i < FTP_SESSIONS; ++i)
	    {
		if (!ftpsessions[i].ctrl_session)
		{
		    ftpsession = session->cargo.ftp_server.ftpsession = &ftpsessions[i];

		    // reset FTP sesion info and mark it as used
		    memset(ftpsession, 0, sizeof(FTP_SERVER_SESSION));
		    ftpsession->ctrl_session = session;

		    msg = 220;
		    break;
		}
	    }

	    if (!msg)
	    {
		// no free FTP sessions
		msg = 421;
	    }
	}
	else if (session->state != TCP_STATE_CONNECTED)
	{
	    if (ftpsession)
	    {
		// mark this session as free
		ftpsession->ctrl_session = NULL;

		if (ftpsession->data_session)
		{
		    // reset data session if still open
		    debug_ftp(printf("freeing FTP session\n"));
		    ftpsession->data_session->state = TCP_STATE_RESET;
		    ftpsession->data_session->retries = -1;
		    ftpsession->data_session->timer = 0;
		}
	    }
	}
	else if (datalength)
	{
	    // this avoids all buffer overruns as long as we always test for '\0'
	    TCPDATA[datalength] = '\0';

	    if (p = strchr(TCPDATA, '\r'), p)
	    {
		*p = '\0';
	    }
	    if (p = strchr(TCPDATA, '\n'), p)
	    {
		*p = '\0';
	    }

	    debug_ftp(printf("FTP received: %s\n", TCPDATA));
	    for (cmd = ftpcommands, msg = 1; *cmd; ++cmd, ++msg)
	    {
		if (strncmp_rom(TCPDATA, *cmd, strlen_rom(*cmd)) == 0)
		{
		    break;
		}
	    }

	    // backwards compatible support for 'experimental' commands
	    switch (msg)
	    {
	    case CMD_XMKD:
		msg = CMD_MKD;
		break;
	    case CMD_XRMD:
		msg = CMD_RMD;
		break;
	    case CMD_XPWD:
		msg = CMD_PWD;
		break;
	    case CMD_XCUP:
		msg = CMD_CDUP;
		break;
	    case CMD_XCWD:
		msg = CMD_CWD;
		break;
	    }

	    if (*cmd)
	    {
		if (msg == CMD_CDUP)
		{
		    // convert 'CDUP' to 'CWD .."
		    msg = CMD_CWD;
		    strcpy_rom(TCPDATA, "CWD ..");
		}

		q = TCPDATA + strlen_rom(*cmd);
		while (*q == ' ')
		{
		    ++q;
		}
		// q points to any parameter after the command or to the closing 0
	    }

	    switch (msg)
	    {
	    case CMD_USER:
		// temporaraly store the username until we get a password
		// the strange lengthcheck is used because we use rootcwd[]
		// to store the username while checking in
		// this way /home/<username> will be the default if nothing
		// is filled in later by ftp_server_usercheck()
		if (strlen(q) < FTP_CWD_MAXLENGTH)
		{
		    strcpy_rom(ftpsession->rootcwd, "/home/");
		    strcat(ftpsession->rootcwd, q);
		    ftpsession->state = FTPSTATE_WAITPASS;
		}
		else
		{
		    msg = 530;
		    ftpsession->state = FTPSTATE_WAITUSER;
		}
		break;

	    case CMD_PASS:
		// check if user/password is valid (user was stored after "/home/" in rootcwd...)
		if ((ftpsession->state == FTPSTATE_WAITPASS)
		    && (tcpip_settings.ftp_server_usercheck(ftpsession->rootcwd + 6, q, ftpsession->rootcwd)))
		{
		    // just count '/' to know the subdirlevel
		    // (no special support for "//" or "..")
		    cwd = ftpsession->rootcwd;
		    ftpsession->rootlevel = 0;
		    while (cwd = strchr(cwd, '/'), cwd)
		    {
			++cwd;
			if (*cwd)
			{
			    ++ftpsession->rootlevel;
			}
		    }
		    ftpsession->level = ftpsession->rootlevel;

		    debug_ftp(printf("setting rootcwd='%s' level=%u\n", ftpsession->rootcwd, ftpsession->rootlevel));

		    // set the cwd to the end of the rootcwd
		    if (strcmp_rom(ftpsession->rootcwd, "/"))
		    {
			ftpsession->cwd = strchr(ftpsession->rootcwd, '\0');
			strcpy_rom(ftpsession->cwd, "/");
		    }
		    else
		    {
			ftpsession->cwd = ftpsession->rootcwd;
		    }
		    ftpsession->state = FTPSTATE_IDLE;
		}
		else
		{
		    ftpsession->state = FTPSTATE_WAITUSER;
		    msg = 530;
		}
		break;

	    case CMD_PORT:
		// parse command: PORT <ip0>,<ip1>,<ip2>,<ip3>,<port0>,<port1>
		// this command is always accepted as ftpclients sometimes don't
		// repeat a PORT even if it wasn't answered with 200 the first time
		ftpsession->data_ip[0] = atoi(q);
		q = strchr(q, ',');
		if (q && *q)
		{
		    ftpsession->data_ip[1] = atoi(++q);
		    q = strchr(q, ',');
		}
		if (q && *q)
		{
		    ftpsession->data_ip[2] = atoi(++q);
		    q = strchr(q, ',');
		}
		if (q && *q)
		{
		    ftpsession->data_ip[3] = atoi(++q);
		    q = strchr(q, ',');
		}
		if (q && *q)
		{
		    ftpsession->data_port = atoi(++q);
		    q = strchr(q, ',');
		}
		if (q && *q)
		{
		    ftpsession->data_port = (ftpsession->data_port << 8) + atoi(++q);
		}

		if (!q || !*q)
		{
		    *(Uint32 *) ftpsession->data_ip = 0;
		    ftpsession->data_port = 0;
		    msg = 501;
		}
		break;

	    default:
		if (ftpsession->state < FTPSTATE_IDLE)
		{
		    // first log in before doing anything else
		    msg = 530;
		}
		break;
	    }

	    if (CMD_WITH_PATH(msg))
	    {
		// parse & check subdir for command
		strcpy(rootcwd, ftpsession->rootcwd);
		cwd = rootcwd + (ftpsession->cwd - ftpsession->rootcwd);
		i = ftpsession->level;
		if (*q == '/')
		{
		    // ignore cwd, start from (virtual) root
		    strcpy_rom(cwd, "/");
		    i = ftpsession->rootlevel;
		    ++q;
		}

		while (*q)
		{
		    p = strchr(q, '/');
		    if (p)
		    {
			*p = '\0';
		    }

		    if (p || CMD_WITH_PATH_ONLY(msg))
		    {
			if (!*q || !strcmp_rom(q, "."))
			{
			    // double slash or single dot, just ignore
			}
			else if (!strcmp_rom(q, ".."))
			{
			    // verify dir up to here, we only allow .. from an existing directory
			    if (!(fs_stat(NULL, rootcwd, q) & FS_ATTR_DIR))
			    {
				debug_ftp(printf("invalid directory: '%s' file: '%s'\n", rootcwd, q));
				msg = 550;
				break;
			    }

			    // double dot: one up
			    --i;
			    if (cwd[1])
			    {
				*strrchr(cwd, '/') = '\0';
			    }
			    if (!*cwd)
			    {
				strcpy_rom(cwd, "/");
			    }
			}
			else
			{
			    // descend into a single level subdir
			    if (++i > FTP_SUBDIR_MAXDEPTH)
			    {
				debug_ftp(printf("directory too deep: '%s'\n", rootcwd));
				msg = 550;
				break;
			    }
			    if (cwd[1])
			    {
				strcat_rom(cwd, "/");
			    }
			    strcat(cwd, q);
			}
		    }

		    if (p)
		    {
			q = p + 1;
		    }
		    else
		    {
			break;
		    }
		}
		debug_ftp(printf("rootcwd='%s' level=%u cwd='%s' file='%s'\n", rootcwd, i, cwd, q));
	    }

	    switch (msg)
	    {
	    case CMD_CWD:
		{
#ifdef ROMSTR
		    char doubledot[] = "..";
		    if (!strcmp_rom(rootcwd, "/") || fs_stat(NULL, rootcwd, doubledot))
#else
		    if (!strcmp_rom(rootcwd, "/") || fs_stat(NULL, rootcwd, ".."))
#endif
		    {
			strcpy(ftpsession->cwd, cwd);
			ftpsession->level = (Uint8) i;
		    }
		    else
		    {
			msg = 550;
		    }
		}
		break;

	    case CMD_RETR:
		ftpsession->data_pos = 0;
		fs_open(&ftpsession->fcb, NULL, rootcwd, q);
		if (ftpsession->fcb.status)
		{
		    ftpsession->state = FTPSTATE_RETR;
		}
		else
		{
		    msg = 550;
		}
		break;

	    case CMD_STOR:
		ftpsession->data_pos = 0;
		fs_create(&ftpsession->fcb, NULL, rootcwd, q);
		if (ftpsession->fcb.status)
		{
		    ftpsession->state = FTPSTATE_STOR;
		}
		else
		{
		    msg = 550;
		}
		break;

	    case CMD_DELE:
		if (fs_remove(NULL, rootcwd, q))
		{
		    msg = 550;
		}

		break;


	    case CMD_MKD:
		if ((i >= FTP_SUBDIR_MAXDEPTH) || fs_mkdir(NULL, rootcwd, q))
		{
		    msg = 521;
		}

		break;

	    case CMD_RMD:
		if (fs_rmdir(NULL, rootcwd, q))
		{
		    msg = 551;
		}

		break;

	    case CMD_RNFR:
		if (strchr(q, '/'))
		{
		    // no paths supported for rename
		    msg = 501;
		}
		else if (!fs_stat(NULL, ftpsession->rootcwd, q))
		{
		    msg = 550;
		}
		else
		{
		    strcpy(ftpsession->rnfr, q);
		}

		break;

	    case CMD_RNTO:
		if (ftpsession->ctrl_lastmsg != CMD_RNFR)
		{
		    // Rename-TO must directly follow Rename-FROM
		    msg = 500;
		}
		else if (strchr(q, '/'))
		{
		    // no paths or move supported for rename
		    msg = 501;
		}
		else if (fs_rename(NULL, ftpsession->rootcwd, ftpsession->rnfr, q))
		{
		    msg = 550;
		}

		break;

	    case CMD_LIST:
	    case CMD_NLST:
		if (strlen(TCPDATA) < 5)
		{
		    q = NULL;
		}
		else if (*q == '/')
		{
		    *cwd = '\0';
		}

		// in listmode data_pos is used to indicate
		// if any more entries are available
		ftpsession->data_pos = fs_findfirst(&ftpsession->fcb, NULL, rootcwd, &ftpsession->data_direntry);
		if (ftpsession->data_pos)
		{
		    ftpsession->state = (msg == CMD_LIST) ? FTPSTATE_LIST : FTPSTATE_NLST;
		}
		else
		{
		    msg = 550;
		}
		break;

	    case CMD_TYPE:
		if (strcmp_rom(q, "I"))
		{
		    msg = 504;
		}
		break;

	    case CMD_MODE:
		if (strcmp_rom(q, "S"))
		{
		    msg = 504;
		}
		break;

	    }

	    if (CMD_WITH_PATH(msg) && (ftpsession->state != FTPSTATE_IDLE) && ftpsession->data_port)
	    {
		// open a data session
		ftpsession->data_session =
		    tcp_create(TCP_PORT_FTPDATA, *(Uint32 *) ftpsession->data_ip,
			       ftpsession->data_port, &ftpdata_server);
		if (ftpsession->data_session)
		{
		    ftpsession->data_session->cargo.ftp_server.ftpsession = ftpsession;
		}
		else
		{
		    // we probably ran out of sessions
		    switch (ftpsession->state)
		    {
		    case FTPSTATE_RETR:
		    case FTPSTATE_STOR:
			fs_close(&ftpsession->fcb);
			break;
		    case FTPSTATE_NLST:
		    case FTPSTATE_LIST:
			fs_closedir(&ftpsession->fcb);
			break;
		    }
		    ftpsession->state = FTPSTATE_IDLE;
		    msg = 226;
		}
	    }
	}
	else
	{
	    switch (ftpsession->state)
	    {
	    case FTPSTATE_MSGREADY:
		msg = 226;
		ftpsession->state = FTPSTATE_IDLE;
		break;
	    case FTPSTATE_MSGABORT:
		msg = 426;
		ftpsession->state = FTPSTATE_IDLE;
		break;
	    }
	}
    }

    if (!msg)
    {
	return 0;
    }

#ifdef FTP_VERBOSE
#define v(a, b) b
#else
#define v(a, b) a
#endif

    // build return answer
    switch (msg)
    {
    case CMD_LIST:
    case CMD_NLST:
    case CMD_RETR:
	strcpy_rom(TCPDATA, "150 " v("ok", "file request OK, about to open data connection"));
	break;
    case CMD_STOR:
	strcpy_rom(TCPDATA, "150 " v("ok", "ready to receive BINARY."));
	break;
    case CMD_PORT:
    case CMD_TYPE:
    case CMD_MODE:
	strcpy_rom(TCPDATA, "200 " v("ok", "command OK"));
	break;
    case CMD_QUIT:
	strcpy_rom(TCPDATA, "221 " v("ok", "goodbye"));
	*tcp_flags |= TCP_FLAG_FIN;
	break;
    case CMD_PASS:
	strcpy_rom(TCPDATA, "230 " v("ok", "user logged in"));
	break;
    case CMD_CWD:
    case CMD_DELE:
    case CMD_RMD:
	strcpy_rom(TCPDATA, "250 " v("ok", "command successful"));
	break;
    case CMD_PWD:
	strcpy_rom(TCPDATA, "257 \"");
	strcat(TCPDATA, ftpsession->cwd);
	strcat_rom(TCPDATA, "\"" v("", " is current directory"));
	break;
    case CMD_MKD:
	strcpy_rom(TCPDATA, "257 " v("ok", "new directory created"));
	break;
    case 220:
	strcpy_rom(TCPDATA, "220 " v("ok", "server ready"));
	break;
    case 226:
	strcpy_rom(TCPDATA, "226 " v("err", "closing data connection"));
	break;
    case CMD_RNTO:
	strcpy_rom(TCPDATA, "250 " v("ok", "rename succesfull"));
	break;
    case CMD_USER:
	strcpy_rom(TCPDATA, "331 " v("ok", "user name OK"));
	break;
    case CMD_RNFR:
	strcpy_rom(TCPDATA, "350 " v("ok", "file found, waiting for new name"));
	break;
    case 421:
	strcpy_rom(TCPDATA, "421 " v("err", "service not available, too many users"));
	*tcp_flags |= TCP_FLAG_FIN;
	break;
    case 426:
	strcpy_rom(TCPDATA, "426 " v("err", "connection closed; transfer aborted (filesystem full)"));
	break;
    case 501:
	strcpy_rom(TCPDATA, "501 " v("err", "syntax error in arguments"));
	break;
    case CMD_PASV:
	strcpy_rom(TCPDATA, "502 " v("err", "command not implemented"));
	break;
    case 504:
	strcpy_rom(TCPDATA, "504 " v("err", "command not implemented for that parameter"));
	break;
    case 521:
	strcpy_rom(TCPDATA, "521 " v("err", "cannot create directory"));
	break;
    case 530:
	strcpy_rom(TCPDATA, "530 " v("err", "not logged in"));
	break;
    case 550:
	strcpy_rom(TCPDATA, "550 " v("err", "file or directory not available"));
	break;
    case 551:
	strcpy_rom(TCPDATA, "550 " v("err", "cannot remove directory"));
	break;
    default:
	strcpy_rom(TCPDATA, "500 " v("err", "syntax error"));
	break;
    }

    strcat_rom(TCPDATA, "\r\n");
    *tcp_flags |= (TCP_FLAG_ACK | TCP_FLAG_PUSH);

    if (ftpsession)
    {
	// remember what we send this time in case a resend is needed
	ftpsession->ctrl_lastmsg = msg;
    }

    debug_ftp(printf("FTP send: %s", TCPDATA));
    return strlen((char *) TCPDATA);
}


/***************************************************************************
 * FUNCTION:    ftpdata_server
 *
 * FTP data server application.
 *
 * This function is used by the FTP server to create a data connection to the client
 * resend   if non-zero the last packet has to be regenerated
 * session  active TCP session
 * datalength   number of incoming bytes
 * flags    pointer to flags in TCP-header
 *
 * returns number of bytes answered
 */
static Uint16 CALLBACKMEMSPEC ftpdata_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags)
{
    FTP_SERVER_SESSION *ftpsession = session->cargo.ftp_server.ftpsession;
    Uint16 length = 0;
    Uint16 lengthfree;
    Uint16 lengthline;
    char dummy;
    Uint8 newstate = 0;
    Uint8 i;
    char *p;
    char *q;

    if (resend)
    {
	// Restart from the last position.
	// There is NO support to really resend the original frame, instead the file
	// is read again. But as the file on disk could  be modified
	// anyway while we are sending it, this doesn't matter.

	// TODO (?): the resend doesn't support directory listings, so any
	// listing where a TCP frame is resend will be corrupted.
	// This has to be considered a 'feature' as implementing
	// it would be too much effort with the current filesystem API.
	ftpsession->data_pos = ftpsession->data_lastpos;
    }
    else
    {
	// remember out starting position his time in case a resend is needed
	ftpsession->data_lastpos = ftpsession->data_pos;
    }

    switch (ftpsession->state)
    {
    case FTPSTATE_NLST:
    case FTPSTATE_LIST:
	lengthfree = session->maxdatalength;
	if (ftpsession->state == FTPSTATE_NLST)
	{
	    lengthline = FS_MAXFILENAMELEN + 2 + 10;
	}
	else
	{
	    lengthline = 29 + 10 + 13 + FS_MAXFILENAMELEN;
	}

	q = TCPDATA;
	// just write complete lines in the buffer, easier to position if another frame is needed
	while (ftpsession->data_pos && (lengthfree > lengthline))
	{
	    // only show dirs and files
	    if (ftpsession->data_direntry.status & (FS_ATTR_DIR | FS_ATTR_FILE))
	    {

		// a bit of a mess, but avoids pulling in sprintf...
		if (ftpsession->state == FTPSTATE_NLST)
		{
		    // write a single 'nlist' line
		    p = ftpsession->data_direntry.name;
		    for (i = 0; i < FS_MAXFILENAMELEN; ++i)
		    {
			if (*p)
			{
			    q[i] = *p++;
			}
			else
			{
			    q[i] = ' ';
			}
		    }
		    strcpy_rom(q + FS_MAXFILENAMELEN, " ");
		    if (ftpsession->data_direntry.status & FS_ATTR_DIR)
		    {
			strcat_rom(q, "<dir>");
		    }
		    else
		    {
			strcat(q, uint32toa(ftpsession->data_direntry.filesize, 0));
		    }
		}
		else
		{
		    // write a single 'list' line, this is mostly bogus,
		    // but makes automated (graphical) clients happy
		    // who parse the result and expect an exact UNIX layout
		    *q = (ftpsession->data_direntry.status & FS_ATTR_DIR) ? 'd' : '-';
		    strcpy_rom(q + 1, "rw-rw-rw-   1 user     ftp  ");
		    strcat(q, uint32toa(ftpsession->data_direntry.filesize, 1));
		    strcat_rom(q, " Jan 01 2000 ");
		    p = ftpsession->data_direntry.name;
		    for (i = 0; (i < FS_MAXFILENAMELEN) && *p; ++i)
		    {
			q[29 + 10 + 13 + i] = *p++;
		    }
		    q[29 + 10 + 13 + i] = '\0';
		}
		strcat_rom(q, "\r\n");
		length += strlen(q);
		q += strlen(q);
	    }

	    // get next direntry
	    ftpsession->data_pos = (Uint32) fs_findnext(&ftpsession->fcb, &ftpsession->data_direntry);
	}

	if (!ftpsession->data_pos)
	{
	    // no more data available
	    *tcp_flags |= (TCP_FLAG_PUSH | TCP_FLAG_FIN);
	}

	break;

    case FTPSTATE_RETR:
	// send a full frame of filedata, starting were we left off the previous frame
	fs_seek(&ftpsession->fcb, ftpsession->data_pos, FS_SEEK_START);
	length = fs_read(&ftpsession->fcb, TCPDATA, session->maxdatalength);
	debug_ftp(printf
		  ("FTP reading from filesystem %u bytes (filepos %lu), %u read\n",
		   session->maxdatalength, ftpsession->data_pos, length));
	ftpsession->data_pos += length;

	if (!fs_read(&ftpsession->fcb, &dummy, 1))
	{
	    // no more data available
	    *tcp_flags |= (TCP_FLAG_PUSH | TCP_FLAG_FIN);
	}

	break;

    case FTPSTATE_STOR:
	// write the receive data to the filesystem, starting were we left off the previous frame
	if (session->state == TCP_STATE_FINACK)
	{
	    *tcp_flags = TCP_FLAG_ACK | TCP_FLAG_FIN;
	    newstate = FTPSTATE_MSGREADY;
	}
	else
	{
	    *tcp_flags = TCP_FLAG_ACK;
	}

	if (datalength > 0)
	{
	    fs_seek(&ftpsession->fcb, ftpsession->data_pos, FS_SEEK_START);
	    length = fs_write(&ftpsession->fcb, TCPDATA, datalength);
	    debug_ftp(printf
		      ("FTP writing to filesystem %u bytes (filepos %lu), %u written\n",
		       datalength, ftpsession->data_pos, length));

	    if (length != datalength)
	    {
		fs_close(&ftpsession->fcb);
		debug_ftp(printf("FTP filesystem full, aborted writing\n"));
		*tcp_flags |= TCP_FLAG_RESET;

		// send "we abort" on the control channel
		newstate = FTPSTATE_MSGABORT;
	    }
	    ftpsession->data_pos += length;
	    length = 0;
	}

	break;
    }

    if ((session->state != TCP_STATE_SYN) && (session->state != TCP_STATE_CONNECTED))
    {
	// close datafile & mark data session invalid
	if (ftpsession)
	{
	    switch (ftpsession->state)
	    {
	    case FTPSTATE_RETR:
	    case FTPSTATE_STOR:
		fs_close(&ftpsession->fcb);
		break;
	    case FTPSTATE_NLST:
	    case FTPSTATE_LIST:
		fs_closedir(&ftpsession->fcb);
		break;
	    }

	    ftpsession->data_session = NULL;	// remove pointer to this datasession
	    ftpsession->data_port = 0;	// mark ip/port settings invalid

	    if (session->state == TCP_STATE_RESET)
	    {
		// send "we were aborted" on the control channel
		newstate = FTPSTATE_MSGABORT;
	    }
	    else if ((*tcp_flags & TCP_FLAG_FIN) && (ftpsession->state != FTPSTATE_IDLE))
	    {
		// send "we are finished" on the control channel
		newstate = FTPSTATE_MSGREADY;
	    }
	}
	else
	{
	    return 0;
	}
    }

    if (newstate && ftpsession->ctrl_session)
    {
	// move the control connection into the new state
	ftpsession->state = newstate;

	// problem is:
	// our servers don't get called without any incoming TCP packet,
	// if the CTRL-session has no more outstanding ACK's it will
	// never be called again, and will never be able to see the new state
	//
	// If a ACK is pending depends on the FTP-client implementation and
	// the duration of this DATA connection, which are both beyond our control
	//
	// if the sessions internal sendnext and acknext sequencenumbers
	// are equal it means no more ACK (to be received) is outstanding
	if (ftpsession->ctrl_session->sendnext == ftpsession->ctrl_session->acknext)
	{
	    // manipulate the retry-settings so the CTRL-session will be
	    // activated as soon as the TCP stack has time available (retries=-1 is magic value)
	    ftpsession->ctrl_session->retries = -1;
	    ftpsession->ctrl_session->timer = 0;
	}
    }

    return length;
}


/***************************************************************************
 * FUNCTION:    uint32toa
 *
 * write given Uint32 in a stringbuffer in humanreadable form,
 * avoids pulling in sprintf()
 */
static char *uint32toa(Uint32 num, Uint8 pad)
{
    Uint8 pos;
    static char strbuf[11];

    pos = 10;
    memset(strbuf, ' ', 11);
    strbuf[pos] = '\0';
    do
    {
	strbuf[--pos] = '0' + (Uint8) (num % 10);
	num = num / 10;
    }
    while (num != 0);

    return pad ? strbuf : strbuf + pos;
}


//**************************************************************************

#endif // OPTION_FTP_SERVER
