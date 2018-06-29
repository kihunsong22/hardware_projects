/*************************************************************************
**
**  VERSION CONTROL:	@(#)http_server.h	1.2	03/10/31
**
**  IN PACKAGE:		Embedded TCPIP
**
**  COPYRIGHT:		Copyright (c) 2002 Altium
**
**  DESCRIPTION:	Demo application: universal HTTP server
** 			with support for pages and reading/writing of
**			variables.
**			See the httpdef_*demo.h for example server definitions.
**
**************************************************************************/

#include "tcpipset.h"
#include "common/tcpip_global.h"

#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#ifdef HTTP_FS_ROOT
#include "common/filesystem/filesys.h"
#endif

//**************************************************************************

typedef struct
{
    Uint16 skip;
#ifdef HTTP_FS_ROOT
    FS_FCB fcb;
#endif
    Uint16 urlcode;
}
HTTP_SERVER_CARGO;

// definition of cgi variables
typedef struct CGIVAR_
{
    Uint8 flags;
    void *var;
    char ROMMEMSPEC *name;
    void ROMMEMSPEC *info1;
    void ROMMEMSPEC *info2;
    CALLBACKMEMSPEC Uint8(*cgifunc) (Uint8 mode, struct CGIVAR_ ROMMEMSPEC * var, char *buf);
}
CGIVAR;

// modes for cgifunc_ functions
#define CGIMODE_SET	   1
#define CGIMODE_SETRESULT  2
#define CGIMODE_GET	   3
#define CGIMODE_SSIGET	   4
#define CGIMODE_INFOGET	   5

//**************************************************************************

extern Uint16 CALLBACKMEMSPEC http_server(Sint8 resend, TCP_SESSION * session,
                                          Uint16 datalength, Uint8 * tcp_flags);


// exported functions to let user create his own cgifunc_ functions
extern Uint8 http_bufwrite_byte(char c);
extern Uint8 http_bufwrite_byte_ssi(char c);
extern void http_bufwrite_word(Uint16 num);
extern void http_bufwrite(const Uint8 *b, Uint16 len);
extern void http_bufwrite_ssi(const Uint8 *b, Uint16 len);
extern void http_bufwrite_str(const char *sram);

#ifdef ROMMEMSPEC_AUTOCAST
// compiler/processor can read constant strings straight from ROM
// or the library startup code copies them from ROM to RAM at startup
#define http_bufwrite_rom(b, len) http_bufwrite(b, len)
#define http_bufwrite_rom_ssi(b, len) http_bufwrite_ssi(b, len)
#define http_bufwrite_romstr(str) http_bufwrite_str((char*) str)
#else
// compiler/processor uses different code to read from ROM and from RAM
extern void http_bufwrite_rom(ROMMEMSPEC Uint8 *b, Uint16 len);
extern void http_bufwrite_rom_ssi(ROMMEMSPEC Uint8 *b, Uint16 len);
extern void http_bufwrite_romstr(ROMMEMSPEC char *str);
#endif

//**************************************************************************

#endif
