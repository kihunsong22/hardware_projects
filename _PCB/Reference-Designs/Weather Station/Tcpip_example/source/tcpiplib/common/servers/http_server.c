/*************************************************************************
**
**  VERSION CONTROL:    @(#)http_server.c	1.5	03/12/18
**
**  IN PACKAGE:     Embedded TCPIP
**
**  COPYRIGHT:      Copyright (c) 2002 Altium
**
**  DESCRIPTION:    Demo application: universal HTTP server
**          with support for pages and reading/writing of
**          variables.
**          See the httpdef_*demo.h for example server definitions.
**
**************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "http_server.h"
#include "../tcpip.h"

#ifndef OPTION_HTTP_SERVER
#ifndef TCPIPDEBUG
#error OPTION_HTTP_SERVER is not defined, but http_server.c is included in project
#endif
#endif

#ifdef OPTION_HTTP_SERVER

//**************************************************************************

#ifdef HTTP_DEBUG
#if DEBUG_HTTP >= 1
#define debug_http(x) x
#else
#define debug_http(x)
#endif
#else
#define debug_http(x)
#endif

//**************************************************************************

#ifdef HTTP_FS_ROOT
#ifdef ROMSTR
static char http_fs_root[] = HTTP_FS_ROOT;
#endif
static Uint16 http_process_url(char *url, FS_FCB * fcb);
static Uint16 http_write_page(Uint16 urlcode, FS_FCB * fcb, Uint16 skip, Uint16 max);
#else
static Uint16 http_process_url(char *url);
static Uint16 http_write_page(Uint16 urlcode, Uint16 skip, Uint16 max);
#endif


//**************************************************************************

// definitions for webserver

// bitmasks for CGI definitions, if undefined turns off the code in the server
#define HTTP_CGI_INFOGET    0x01	// is included in interface description
#define HTTP_CGI_GET        0x02	// is readable with CGI interface
#define HTTP_CGI_SSIGET     0x04	// is readable with SSI interface
#define HTTP_CGI_SET        0x10	// is writable with CGI interface

// include the userdefined server definition,
// must be included here as it uses the prototypes above
// HTTP_DEF must be set to a valid server definition file
#include HTTP_DEF

// some defaults if not set in the server definition
#ifndef HTTP_CGIURL_GET
#define HTTP_CGIURL_GET "/cgi/get"
#endif
#ifndef HTTP_CGIURL_SET
#define HTTP_CGIURL_SET "/cgi/set"
#endif
#ifndef HTTP_CGIURL_INFO
#define HTTP_CGIURL_INFO "/cgi/info"
#endif

#ifndef HTTP_ERRORPAGE_NOTFOUND
#define HTTP_ERRORPAGE_NOTFOUND "<H2>HTTP 404 - File not found</H2>"
#endif
#ifndef HTTP_ERRORPAGE_INVALIDCALL
#define HTTP_ERRORPAGE_INVALIDCALL "<H2>HTTP 490 - Invalid CGI call</H2>"
#endif
#ifndef HTTP_ERRORPAGE_INVALIDVALUE
#define HTTP_ERRORPAGE_INVALIDVALUE "<H2>HTTP 491 - Invalid CGI value</H2>"
#endif

// all types used in userdefinition of server:

// definition of HTTP snippet (will be included based on urlcodemask)
typedef struct
{
    ROMMEMSPEC char *ext;
    ROMMEMSPEC char *mimetype;
}
HTTPMIME;
static ROMMEMSPEC HTTPMIME httpmimedef[] = HTTP_MIMETYPES;


// definition of constant pages
typedef struct
{
    ROMMEMSPEC char *url;
    ROMMEMSPEC Uint8 *pagedata;
    Uint16 len;
}
HTTPPAGE;
#ifdef HTTP_PAGES
static ROMMEMSPEC HTTPPAGE httppagedef[] = HTTP_PAGES;
#define HTTPPAGES_MAX (sizeof(httppagedef) / sizeof(HTTPPAGE))
#endif

// definition of custum url-recognition functions
typedef CALLBACKMEMSPEC Uint16(*HTTPCUSTOMURL) (char *url);
#ifdef HTTP_CUSTOMURLS
static ROMMEMSPEC HTTPCUSTOMURL httpcustomurldef[] = HTTP_CUSTOMURLS;
#endif

// definition of custom page generation functions
typedef CALLBACKMEMSPEC Uint8(*HTTPCUSTOMPAGE) (Uint16 urlcode);
#ifdef HTTP_CUSTOMPAGES
static ROMMEMSPEC HTTPCUSTOMPAGE httpcustompagedef[] = HTTP_CUSTOMPAGES;
#endif

#ifdef HTTP_CGIVARS
// CGI to C interface functions
#ifdef HTTP_CGIVAR_BOOL
CALLBACKMEMSPEC Uint8 cgifunc_bool(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url);
#endif
#ifdef HTTP_CGIVAR_WORD
CALLBACKMEMSPEC Uint8 cgifunc_word(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url);
#endif
#ifdef HTTP_CGIVAR_STRING
CALLBACKMEMSPEC Uint8 cgifunc_string(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url);
#endif
#ifdef HTTP_CGIVAR_IP
CALLBACKMEMSPEC Uint8 cgifunc_ip(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url);
#endif

static ROMMEMSPEC CGIVAR cgivardef[] = HTTP_CGIVARS;
#endif // HTTP_CGIVARS

// reserved mimetypes
#define MIMETYPE_INFO    ((Uint16) 0xFF)
#define MIMETYPE_CGIGET  ((Uint16) 0xFE)
#define MIMETYPE_CGISET  ((Uint16) 0xFD)

// reserved urlcodes
#define URLCODE_NOTFOUND    ((Uint16) ((MIMETYPE_INFO << 8) + 0xFF))
#define URLCODE_INVALIDCALL ((Uint16) ((MIMETYPE_INFO << 8) + 0xFE))
#define URLCODE_INVALIDVALUE    ((Uint16) ((MIMETYPE_INFO << 8) + 0xFD))
#define URLCODE_CGIINFOGET  ((Uint16) ((MIMETYPE_CGIGET << 8) + 0xFF))

//**************************************************************************

static char *http_buffer;	// pointer to buffer were current page is being written
static Uint16 http_pos;	// position in buffer were current page is being written
static Uint16 http_max;	// maximum number of characters in buffer
static Uint16 http_skip;	// number of characters left to skip while writing

#ifdef HTTP_CGI_SSIGET
static ROMMEMSPEC char http_ssitag[] = "<!--#echo var=\"";
static Uint8 http_ssitag_pos;
static char http_ssivar[HTTP_SSIGETVAR_MAXLEN + 1];
static Uint8 http_ssivar_pos;
#endif


/***************************************************************************
 * FUNCTION:    http_server
 *
 * TCP server application.
 * Process incoming HTTP request, http_process_url() is called to decode
 * the URL into an id, this id is fed into http_write_page(). This last
 * call can be repeated with a different page-offset in case the page
 * didn't fit into a single frame.
 *
 * resend   if non-zero the last packet has to be regenerated
 * session  active TCP session
 * datalength   number of incoming bytes
 * flags    pointer to flags in TCP-header
 *
 * returns number of bytes answered
 */
Uint16 CALLBACKMEMSPEC http_server(Sint8 resend, TCP_SESSION * session, Uint16 datalength, Uint8 * tcp_flags)
{
    HTTP_SERVER_CARGO *cargo = (HTTP_SERVER_CARGO *) & session->cargo.http_server;	// cast only needed to keep some compilers from complaining
    char *p;
    char *q;

    if (session->state == TCP_STATE_SYNACK)
    {
	// initialize sessioninfo
	// (everything is set to 0 by the stack)
    }
    else if (session->state != TCP_STATE_CONNECTED)
    {
#ifdef HTTP_FS_ROOT
	if (cargo->fcb.status)
	{
	    fs_close(&cargo->fcb);
	}
#endif
	return 0;
    }

    if (datalength > 0)
    {
	// decode HTTP request
	if (strncmp_rom(TCPDATA, "GET ", 4) == 0)
	{
	    // it's a request for a page, decode the url for later use
	    q = TCPDATA + 4;
	    TCPDATA[session->maxdatalength - 1] = '\0';	// keep the strchr() from crashing
	    // nul-terminate the url
	    if ((p = strpbrk_rom(q, " \r\n")) != NULL)
	    {
		*p = '\0';
	    }
	    else
	    {
		*q = '\0';
	    }
	    debug_http(printf(s("HTTP received 'GET %s' "), q));
#ifdef HTTP_FS_ROOT
	    cargo->urlcode = http_process_url(q, &cargo->fcb);
#else
	    cargo->urlcode = http_process_url(q);
#endif
	    debug_http(printf("(pagecode %04X)\n", cargo->urlcode));
	}
    }

    if ((cargo->urlcode != 0) && ((datalength == 0) || (*tcp_flags & TCP_FLAG_PUSH)))
    {
	if ((resend == 0) && ((*tcp_flags & TCP_FLAG_PUSH) == 0))
	{
	    cargo->skip += session->maxdatalength;
	}

	// send the requested data
#ifdef HTTP_FS_ROOT
	datalength = http_write_page(cargo->urlcode, &cargo->fcb, cargo->skip, session->maxdatalength);
#else
	datalength = http_write_page(cargo->urlcode, cargo->skip, session->maxdatalength);
#endif
	if (datalength > session->maxdatalength)
	{
	    // we need more frames, just send the data
	    datalength = session->maxdatalength;
	    *tcp_flags = TCP_FLAG_ACK;
	}
	else
	{
	    // data fitted in a single TCP frame,
	    // send & push the data, close the session
	    *tcp_flags = TCP_FLAG_ACK | TCP_FLAG_PUSH | TCP_FLAG_FIN;
	}
	debug_http(printf("HTTP send page %04X (start %u, %u bytes%s)\n", cargo->urlcode,
			  cargo->skip, datalength, (*tcp_flags & TCP_FLAG_FIN) ? "" : ", incomplete"));
	return datalength;
    }
    else
    {
	return 0;
    }
}


/***************************************************************************
 * FUNCTION:    http_process_url
 *
 * decode the given url into a unique code, process any results from
 * forms in the url, return the unique urlcode belonging to the page
 * to be served
 * if a file is opened the handle is stored in the fcb.
 */
#ifdef HTTP_FS_ROOT
static Uint16 http_process_url(char *url, FS_FCB * fcb)
#else
static Uint16 http_process_url(char *url)
#endif
{
    Uint8 i;
    char *q;
    union p_looppointer
    {
	HTTPMIME ROMMEMSPEC *mime;
	HTTPPAGE ROMMEMSPEC *page;
	HTTPCUSTOMURL ROMMEMSPEC *customurl;
	CGIVAR ROMMEMSPEC *var;
    }
    p;
    Uint16 mimetype;
#ifdef HTTP_CUSTOMURLS
    Uint16 urlcode;
#endif

    // strip optional leading servername
    // (don't care what it is, IP-address is leading)
    if ((q = strstr_rom(url, "//")) != NULL)
    {
	url = q + 2;
	if ((q = strchr(url, '/')) != NULL)
	{
	    url = q;
	}
    }

    // strip leftover leading multiple "/"
    while (url[1] == '/')
    {
	++url;
    }

#ifdef HTTP_DIRINDEX
    // default filename if none given
    if (url[strlen(url) - 1] == '/')
    {
	strcat_rom(url, HTTP_DIRINDEX);
    }
#endif

    // default mimetype is first one in the list
    mimetype = 0;

#ifdef HTTP_CGI_INFOGET
    // detect url of info-interface
    if (strcmp_rom(url, HTTP_CGIURL_INFO) == 0)
    {
	return URLCODE_CGIINFOGET;
    }
#endif

#ifdef HTTP_CGI_SET
    // detect invalid url: set without a parameter
    if (strcmp_rom(url, HTTP_CGIURL_SET) == 0)
    {
	return URLCODE_INVALIDVALUE;
    }

    // detect url of all individual cgi-set interfaces
    // for all cgi-vars with the CGISET flag, process both /cgiurl?var=value
    // and /cgiurl/returnpage.html?var=value (where returnpage.html will
    // be returned if the set was excepted instead of the normal response
    // generated by the cgivar-callback function for mode CGIMODE_SETRESULT)
    if (strncmp_rom(url, HTTP_CGIURL_SET, sizeof(HTTP_CGIURL_SET) - 1) == 0)
    {
	char *cgi = NULL;
	if (url[sizeof(HTTP_CGIURL_SET) - 1] == '/')
	{
	    // url to be returned starts after the '/'
	    url += sizeof(HTTP_CGIURL_SET) - 1;

	    // if it is an original cgi-set there should be a closing '?'
	    // after which the cgi command starts
	    cgi = strchr(url, '?');
	    if (cgi)
	    {
		// terminate the url to be returned after the '?' (start of the cgi command)
		*cgi++ = 0;
	    }
	}
	else if (url[sizeof(HTTP_CGIURL_SET) - 1] == '?')
	{
	    // cgi command starts rigth after the '?'
	    cgi = url + sizeof(HTTP_CGIURL_SET);

	    // don't process the url anymore after this CGISET
	    url = NULL;
	}
	else
	{
	    // not a valid CGISET url after all, continue processing of the url
	}

	if (cgi)
	{
	    for (p.var = cgivardef, i = 1; p.var->cgifunc; ++p.var, ++i)
	    {
		if ((p.var->flags & HTTP_CGI_SET) && (strncmp_rom(cgi, p.var->name, strlen_rom(p.var->name)) == 0) &&
		    (cgi[strlen_rom(p.var->name)] == '='))
		{
		    if (p.var->cgifunc(CGIMODE_SET, p.var, cgi + strlen_rom(p.var->name) + 1))
		    {
			if (url)
			{
			    break;
			}
			return (MIMETYPE_CGISET << 8) + i;
		    }
		    else
		    {
			return URLCODE_INVALIDVALUE;
		    }
		}
	    }

	    if (!p.var->cgifunc)
	    {
		// invalid url: set with unknown parameter
		return URLCODE_INVALIDCALL;
	    }

	    // if we are here the CGI set was processed correcetly,
	    // but we must return the page after the (HTTP_CGIURL_SET '/')
	    // instead of a default CGIMODE_SETRESULT response
	}
    }
#endif

#ifdef HTTP_CGI_GET
    // detect url of all individual cgi-get interfaces (get with parameter)
    // for all cgi-vars with the CGIGET flag
    if (strncmp_rom(url, HTTP_CGIURL_GET "?", sizeof(HTTP_CGIURL_GET)) == 0)
    {
	url += sizeof(HTTP_CGIURL_GET);
	for (p.var = cgivardef, i = 1; p.var->cgifunc; ++p.var, ++i)
	{
	    if ((p.var->flags & HTTP_CGI_GET) && (strcmp_rom(url, p.var->name) == 0))
	    {
		return (MIMETYPE_CGIGET << 8) + i;
	    }
	}

	// invalid url: get with unknown parameter
	return URLCODE_INVALIDCALL;
    }
#endif

    // ignore all unrecognized?xxxx parameters
    q = strrchr(url, '?');
    if (q)
    {
	*q = '\0';
    }

    // convert extension to mimetype-nr
    q = strrchr(url, '.');
    if (q)
    {
	++q;
    }
    else
    {
	q = url + strlen(url);
    }
    for (p.mime = httpmimedef, i = 0; p.mime->ext; ++p.mime, ++i)
    {
	if (strcmp_rom(q, p.mime->ext) == 0)
	{
	    mimetype = i << 8;
	    break;
	}
    }

#ifdef HTTP_CUSTOMURLS
    {
	HTTPCUSTOMURL ROMMEMSPEC *customurl;

	// detect url's of runtime detected/generated pages
	for (customurl = httpcustomurldef; *customurl; ++customurl)
	{
	    urlcode = (*customurl) (url);
	    if (urlcode != 0)
	    {
		return mimetype + urlcode;
	    }
	}
    }
#endif // HTTPCUSTOMURL_DEF

#ifdef HTTP_PAGES
    // detect url's of hardcoded pages
    for (p.page = httppagedef, i = 1; p.page->url; ++p.page, ++i)
    {
	if (strcmp_rom(url, p.page->url) == 0)
	{
	    return mimetype + i;
	}
    }
#endif // HTTPPAGE_DEF

#ifdef HTTP_FS_ROOT
    // detect if url is a "real" file on the filesystem

    // parse url into path & filename
    q = url;
    url = strrchr(url, '/');
    if (url)
    {
	*url++ = '\0';
    }
    else
    {
	url = q;
	q = NULL;
    }

#ifdef ROMSTR
    if (fs_open(fcb, http_fs_root, q, url) == 0)
#else
    if (fs_open(fcb, HTTP_FS_ROOT, q, url) == 0)
#endif
    {
	// return the default
	return mimetype + 0xFF;
    }
#endif // HTTP_FS_ROOT

    // url unrecognized
    return URLCODE_NOTFOUND;
}


/***************************************************************************
 * FUNCTION:    http_write_page
 *
 * Write a page in the buffer, skip number of characters before starting
 * to write, stop when buffer is full.
 *
 * urlcode  unique identifier of page to write
 * fcb      fcb , status != 0 if serving from a file (if not the page is hardcoded or dynamic)
 * skip     number of bytes to discard before writing in the buffer
 * max      maximum number of bytes to write inthe buffer
 *
 * returns number of bytes written, or max + 1 if max is written and there
 * is still more to send
 */
#ifdef HTTP_FS_ROOT
static Uint16 http_write_page(Uint16 urlcode, FS_FCB * fcb, Uint16 skip, Uint16 max)
#else
static Uint16 http_write_page(Uint16 urlcode, Uint16 skip, Uint16 max)
#endif
{
    Uint8 mimetype;
    Uint8 pagenr;
#ifdef HTTP_CGIVARS
    CGIVAR ROMMEMSPEC *var;
#endif
#ifdef HTTP_PAGES
    HTTPPAGE ROMMEMSPEC *page;
#endif
#ifdef HTTP_CUSTOMPAGES
    HTTPCUSTOMPAGE ROMMEMSPEC *custompage;
#endif
#ifdef HTTP_FS_ROOT
    Sint16 read;
    char dummy;
#endif

    http_buffer = TCPDATA;
    http_skip = skip;
    http_max = max;
    http_pos = 0;
#ifdef HTTP_CGI_SSIGET
    http_ssitag_pos = 0;
    http_ssivar_pos = 0;
#endif

    mimetype = (urlcode >> 8) & 0x00FF;
    pagenr = urlcode & 0x00FF;

    http_bufwrite_romstr("HTTP/1.0 ");

    switch (urlcode)
    {
    case URLCODE_NOTFOUND:
	http_bufwrite_romstr("404 FILE NOT FOUND");
	break;
#ifdef URLCODE_INVALIDCALL
    case URLCODE_INVALIDCALL:
	http_bufwrite_romstr("490 ERROR IN CGI CALL");
	break;
#endif
#ifdef URLCODE_INVALIDVALUE
    case URLCODE_INVALIDVALUE:
	http_bufwrite_romstr("491 ERROR IN CGI VALUE");
	break;
#endif
    default:
	http_bufwrite_romstr("200 OK");
    }

    http_bufwrite_romstr("\r\nContent-type: ");

    // write HTTP mimetype
    switch (mimetype)
    {
    case MIMETYPE_INFO:
	http_bufwrite_romstr("text/html");
	break;

    case MIMETYPE_CGIGET:
    case MIMETYPE_CGISET:
	http_bufwrite_romstr("text/plain");
	break;

    default:
	http_bufwrite_romstr(httpmimedef[urlcode >> 8].mimetype);
	break;
    }

    http_bufwrite_romstr("\r\n\r\n");

#if defined(HTTP_CGI_GET) || defined(HTTP_CGI_SET) || defined(HTTP_CGI_INFOGET)
    if ((mimetype == MIMETYPE_CGIGET) || (mimetype == MIMETYPE_CGISET))
    {
#ifdef HTTP_CGI_INFOGET
	if (urlcode == URLCODE_CGIINFOGET)
	{
	    // generate a info line for every cgi interface with INFOGET defined
	    for (var = cgivardef; var->cgifunc; ++var)
	    {
		if (var->flags & HTTP_CGI_INFOGET)
		{
		    http_bufwrite_romstr(var->name);
		    http_bufwrite_romstr(" ");
		    var->cgifunc(CGIMODE_INFOGET, var, NULL);
		    http_bufwrite_romstr(" INFOGET");
#ifdef HTTP_CGI_GET
		    if (var->flags & HTTP_CGI_GET)
		    {
			http_bufwrite_romstr(" CGIGET");
		    }
#endif
#ifdef HTTP_CGI_SSIGET
		    if (var->flags & HTTP_CGI_SSIGET)
		    {
			http_bufwrite_romstr(" SSIGET");
		    }
#endif
#ifdef HTTP_CGI_SET
		    if (var->flags & HTTP_CGI_SET)
		    {
			http_bufwrite_romstr(" CGISET");
		    }
#endif
		    http_bufwrite_romstr("\n");
		}
	    }
	}
	else
#endif // INFOGET
#if defined(HTTP_CGI_GET) | defined(HTTP_CGI_SET)
	if ((mimetype == MIMETYPE_CGIGET) || (mimetype == MIMETYPE_CGISET))
	{
	    // write a single line page for cgi-vars with the CGIGET flag
	    // or as reply on a SET action
	    var = &cgivardef[pagenr - 1];
	    var->cgifunc((Uint8) ((mimetype == MIMETYPE_CGIGET) ? CGIMODE_GET : CGIMODE_SETRESULT), var, NULL);
	}
#endif // defined(HTTP_CGI_GET) | defined(HTTP_CGI_SET)
    }
    else
#endif // defined(HTTP_CGI_GET) || defined(HTTP_CGI_SET) || defined(HTTP_CGI_INFOGET)
#ifdef HTTP_FS_ROOT
    if (fcb->status)
    {
	// TODO: SSI support

	// serve files from the filesystem
	fs_seek(fcb, http_skip, FS_SEEK_START);
	http_skip = 0;
	read = fs_read(fcb, http_buffer + http_pos, (short) (http_max - http_pos));

	if (read > 0)
	{
	    http_pos += read;
	}

	// verify if the file is any longer
	if (fs_read(fcb, &dummy, 1) > 0)
	{
	    ++http_pos;
	}
    }
    else
#endif // HTTP_FS_ROOT
#ifdef HTTP_PAGES
    if (pagenr <= HTTPPAGES_MAX)
    {
	// write a hardcoded page
	page = &httppagedef[pagenr - 1];
#ifdef HTTP_CGI_SSIGET
	http_bufwrite_rom_ssi(page->pagedata,
			      (Uint16) (page->len ? page->len : strlen_rom((ROMMEMSPEC char *) page->pagedata)));
#else
	http_bufwrite_rom(page->pagedata,
			  (Uint16) (page->len ? page->len : strlen_rom((ROMMEMSPEC char *) page->pagedata)));
#endif
    }
    else
#endif // HTTP_PAGES
#ifdef HTTP_CGIVARS
    if (urlcode == URLCODE_INVALIDCALL)
    {
	http_bufwrite_romstr(HTTP_ERRORPAGE_INVALIDCALL);
    }
    else
#endif // HTTP_CGIVARS
#ifdef HTTP_CGI_SET
    if (urlcode == URLCODE_INVALIDVALUE)
    {
	http_bufwrite_romstr(HTTP_ERRORPAGE_INVALIDVALUE);
    }
    else
#endif // HTTP_CGI_SET
    if (urlcode == URLCODE_NOTFOUND)
    {
	http_bufwrite_romstr(HTTP_ERRORPAGE_NOTFOUND);
    }
#ifdef HTTP_CUSTOMPAGES
    else
    {
	// write all custom pages
	// (urlfunc functions should both check urlcode and write page, return 1 is processed)
	for (custompage = httpcustompagedef; *custompage; ++custompage)
	{
	    if ((*custompage) (urlcode))
	    {
		break;
	    }
	}
    }
#endif // HTTP_CUSTOMPAGES

    return http_pos;
}


/***************************************************************************
 * FUNCTION:    http_bufwrite_byte
 *
 * write single character to http_buffer, if skip != 0 don't actually write the
 * character but decrement skip, if the buffer is full (http_max characters)
 * return 0 (otherwise 1)
 *
 * All write functions below call this function.
 */
Uint8 http_bufwrite_byte(char c)
{
    if (http_skip != 0)
    {
	--http_skip;
	return 1;
    }
    else if (http_pos < http_max)
    {
	http_buffer[http_pos++] = c;
	return 1;
    }
    else
    {
	++http_pos;	// signals we need at least another TCP-frame to send our page
	return 0;
    }
}


#ifdef HTTP_CGI_SSIGET
/***************************************************************************
 * FUNCTION:    http_bufwrite_byte_ssi
 *
 * write single character to http_buffer, while replacing any SSI tags
 * if the buffer is full (http_max characters) return 0 (otherwise 1)
 */
Uint8 http_bufwrite_byte_ssi(char c)
{
    volatile char copy_c = c;	// TODO: to be removed...

    if (!http_ssitag[http_ssitag_pos])
    {
	// busy collecting varname
	if (c == '"')
	{
	    // varname is complete
	    CGIVAR ROMMEMSPEC *vardef;

	    // terminate the varname
	    http_ssivar[http_ssivar_pos++] = 0;

	    // find out which var it is, just ignore unknown names
	    for (vardef = cgivardef; vardef->cgifunc; ++vardef)
	    {
		if ((vardef->flags & HTTP_CGI_SSIGET) && (strcmp_rom(http_ssivar, vardef->name) == 0))
		{
		    // call the CGI function to output its stuff
		    vardef->cgifunc(CGIMODE_SSIGET, vardef, NULL);
		}
	    }
	}
	else if (c == '>')
	{
	    // tag has ended, reset to find another match
	    http_ssitag_pos = 0;
	}
	else if (http_ssivar_pos < HTTP_SSIGETVAR_MAXLEN)
	{
	    http_ssivar[http_ssivar_pos++] = c;
	}
	return 1;
    }
    else if (c == http_ssitag[http_ssitag_pos])
    {
	if (!http_ssitag[++http_ssitag_pos])
	{
	    // recognized full SSI tag, varname from here
	    http_ssivar_pos = 0;
	}
	return 1;
    }

    if (http_ssitag_pos)
    {
	// we suppressed some characters while thinking they might be an SSI tag,
	// just write them out after all
	ROMMEMSPEC char *p = http_ssitag;
	while (http_ssitag_pos)
	{
	    --http_ssitag_pos;
	    http_bufwrite_byte(*p++);
	}
    }

    c = copy_c;	// TODO: to be removed...
    return http_bufwrite_byte(c);
}
#endif // HTTP_CGI_SSIGET


/***************************************************************************
 * FUNCTION:    http_bufwrite
 *
 * write given data (start at b for len bytes) in the http_buffer
 */
void http_bufwrite(const Uint8 * b, Uint16 len)
{
    while ((len--) && http_bufwrite_byte(*b++));
}


#ifdef HTTP_CGI_SSIGET
/***************************************************************************
 * FUNCTION:    http_bufwrite_ssi
 *
 * write given data (start at b for len bytes) in the http_buffer
 * while replacing any SSI tags
 */
void http_bufwrite_ssi(const Uint8 * b, Uint16 len)
{
    while ((len--) && http_bufwrite_byte_ssi(*b++));
}
#endif // HTTP_CGI_SSIGET


/***************************************************************************
 * FUNCTION:    http_bufwrite_str
 *
 * write given string in the http_buffer
 */
void http_bufwrite_str(const char *str)
{
    while ((*str) && http_bufwrite_byte(*str++));
}


/***************************************************************************
 * FUNCTION:    http_bufwrite_word
 *
 * write given Uint16 in the http_buffer, is in fact sort of itoa() but
 * avoids pulling in sprintf()
 */
void http_bufwrite_word(Uint16 num)
{
    Uint8 pos;
    char buf[5];
    pos = 5;
    do
    {
	buf[--pos] = '0' + (num % 10);
	num = num / 10;
    }
    while (num != 0);
    http_bufwrite((Uint8 *) buf + pos, (Uint16) (5 - pos));
}


#ifndef ROMMEMSPEC_AUTOCAST
// processor needs different instructions to read from ROM,

/***************************************************************************
 * FUNCTION:    http_bufwrite_rom
 *
 * write given ROM-data (start at b for len bytes) in the http_buffer
 */
void http_bufwrite_rom(ROMMEMSPEC Uint8 * b, Uint16 len)
{
    while ((len--) && http_bufwrite_byte(*b++));
}

#ifdef HTTP_CGI_SSIGET
/***************************************************************************
 * FUNCTION:    http_bufwrite_rom_ssi
 *
 * write given ROM-data (start at b for len bytes) in the http_buffer
 * while replacing any SSI tags
 */
void http_bufwrite_rom_ssi(ROMMEMSPEC Uint8 * b, Uint16 len)
{
    while ((len--) && http_bufwrite_byte_ssi(*b++));
}
#endif // HTTP_CGI_SSIGET


/***************************************************************************
 * FUNCTION:    http_bufwrite_romstr
 *
 * write given ROM-string in the http_buffer
 */
void http_bufwrite_romstr(ROMMEMSPEC char *str)
{
    while ((*str) && http_bufwrite_byte(*str++));
}

#endif // !ROMMEMSPEC_AUTOCAST


#ifdef HTTP_CGIVAR_BOOL
/***************************************************************************
 * FUNCTION:    cgifunc_bool
 *
 * Interface function between a boolean (Uint8) C-variable and it's
 * web-url-interface. Have a look at CGIVAR structures and the demo
 * server definitions to see how to use this function.
 *
 * cgimode  mode: set value, get value or show var-info
 * var      CGI definition of variable
 * url      value as present in the url
 *
 * for setting should return non-zero if everything went ok, otherwise 0.
 */
CALLBACKMEMSPEC Uint8 cgifunc_bool(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url)
{
    switch (cgimode)
    {
    case CGIMODE_SET:
	// update variable from url, accept both offtext/ontext and "0"/"1"
	if ((strcmp_rom(url, (ROMMEMSPEC char *) var->info1) == 0) || (strcmp_rom(url, "0") == 0))
	{
	    *((Uint8 *) var->var) = 0;
	    return 1;
	}
	if ((strcmp_rom(url, (ROMMEMSPEC char *) var->info2) == 0) || (strcmp_rom(url, "1") == 0))
	{
	    *((Uint8 *) var->var) = 1;
	    return 1;
	}
	break;

    case CGIMODE_SETRESULT:
    case CGIMODE_SSIGET:
    case CGIMODE_GET:
	// write ontext or offtext (depending on given value) in the static buffer
	if (*((Uint8 *) var->var))
	{
	    http_bufwrite_romstr(var->info2);
	}
	else
	{
	    http_bufwrite_romstr(var->info1);
	}
	break;

    case CGIMODE_INFOGET:
	http_bufwrite_romstr("boolean ");
	http_bufwrite_romstr(var->info1);
	http_bufwrite_romstr("/");
	http_bufwrite_romstr(var->info2);
	break;

    }

    return 0;
}
#endif // HTTP_CGIVAR_BOOL


#ifdef HTTP_CGIVAR_WORD
/***************************************************************************
 * FUNCTION:    cgifunc_word
 *
 * Equals cgifunc_bool, but this interfaces Uint16 variables
 */
CALLBACKMEMSPEC Uint8 cgifunc_word(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url)
{
    Uint16 val;
    switch (cgimode)
    {
    case CGIMODE_SET:
	// update variable from url if within limits
	val = (Uint16) atoi(url);
	if ((val >= (Uint16) (Uint32) var->info1) && (val <= (Uint16) (Uint32) var->info2))
	{
	    *(Uint16 *) var->var = val;
	    return 1;
	}
	break;

    case CGIMODE_SETRESULT:
    case CGIMODE_SSIGET:
    case CGIMODE_GET:
	// write unsigned value in human-readable form in the static buffer
	http_bufwrite_word(*(Uint16 *) var->var);
	break;

    case CGIMODE_INFOGET:
	http_bufwrite_romstr("Uint16 ");
	http_bufwrite_word((Uint16) (Uint32) var->info1);
	http_bufwrite_romstr("..");
	http_bufwrite_word((Uint16) (Uint32) var->info2);
	break;

    }

    return 0;
}
#endif // HTTP_CGIVAR_WORD


#ifdef HTTP_CGIVAR_STRING
/***************************************************************************
 * FUNCTION:    cgifunc_string
 *
 * Equals cgifunc_bool, but this interfaces char* variables
 */
CALLBACKMEMSPEC Uint8 cgifunc_string(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url)
{
    switch (cgimode)
    {
    case CGIMODE_SET:
	// update variable from url if length within limits
	if ((strlen(url) >= (Uint32) var->info1) && (strlen(url) <= (Uint32) var->info2))
	{
	    strcpy((char *) var->var, url);
	    return 1;
	}
	break;

    case CGIMODE_SETRESULT:
    case CGIMODE_SSIGET:
    case CGIMODE_GET:
	// write unsigned value in human-readable form in the static buffer
	http_bufwrite_str((char *) var->var);
	break;

    case CGIMODE_INFOGET:
	http_bufwrite_romstr("string ");
	http_bufwrite_word((Uint16) (Uint32) var->info1);
	http_bufwrite_romstr("..");
	http_bufwrite_word((Uint16) (Uint32) var->info2);
	break;

    }

    return 0;
}
#endif // HTTP_CGIVAR_STRING


#ifdef HTTP_CGIVAR_IP
/***************************************************************************
 * FUNCTION:    cgifunc_ip
 *
 * Equals cgifunc_bool, but this interfaces IP-address variables
 * (Uint8[4] internaly, xxx.xxx.xxx.xxx on the web)
 */
CALLBACKMEMSPEC Uint8 cgifunc_ip(Uint8 cgimode, CGIVAR ROMMEMSPEC * var, char *url)
{
    url = url;	// not used

    switch (cgimode)
    {
    case CGIMODE_SSIGET:
    case CGIMODE_GET:
	// write unsigned value in human-readable form in the static buffer
	http_bufwrite_word((Uint16) ((Uint8 *) var->var)[0]);
	http_bufwrite_byte('.');
	http_bufwrite_word((Uint16) ((Uint8 *) var->var)[1]);
	http_bufwrite_byte('.');
	http_bufwrite_word((Uint16) ((Uint8 *) var->var)[2]);
	http_bufwrite_byte('.');
	http_bufwrite_word((Uint16) ((Uint8 *) var->var)[3]);
	break;

    case CGIMODE_INFOGET:
	http_bufwrite_romstr("ip");
	break;

    }

    return 0;
}
#endif // HTTP_CGIVAR_IP

//*****************************************************************************

#endif // OPTION_HTTP_SERVER
