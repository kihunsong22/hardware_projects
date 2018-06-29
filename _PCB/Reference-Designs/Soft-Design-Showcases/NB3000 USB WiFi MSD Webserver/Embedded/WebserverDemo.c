////////////////////////////////////////////////////////////////////////////////
// WebServerDemo.c

#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>

#include <lwip.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/api.h>

#include <webstats/server_stats.h>
#include <util_string.h>

#include "WebServerDemo.h"

////////////////////////////////////////////////////////////////////////////////
const static int8_t http_html_error[] ="HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n"
                                        "<html><head><title>Error</title></head><body>"
                                        "<H2>HTTP 404 - File not found</H2>"
                                        "</body></html>";

////////////////////////////////////////////////////////////////////////////////
#define WEBSERV_ROOTDIR         "/usb%i/%s"
#define WEBSERV_TOKEN_GET       "GET /"
#define WEBSERV_TOKEN_GET_LEN   5
#define WEBSERV_TOKEN_HTTP      " HTTP"
#define WEBSERV_TOKEN_HTTP_LEN  5

#define HTTP_DEFAULT_PAGE   "index.html"
#define HTTP_RESP_TEMPLATE  "HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-Length: %i\r\n\r\n"
#define MIME_TYPE_HTML      "text/html"
#define MIME_TYPE_JPEG      "image/jpeg"
#define MIME_TYPE_PNG       "image/x-png"
#define MIME_TYPE_GIF       "image/gif"
#define MIME_TYPE_UNKW      "*/\\*"

#define EXTN_HTML           ".htm"
#define EXTN_JPEG           ".jpg"
#define EXTN_GIF            ".gif"
#define EXTN_PNG            ".png"

////////////////////////////////////////////////////////////////////////////////
static int8_t filePath[256];
static int8_t sndBuf[1460];

////////////////////////////////////////////////////////////////////////////////
static usbhost_t  *USBHost;

////////////////////////////////////////////////////////////////////////////////
static uint32_t http_serve_file(struct netconn *conn, int8_t *filename)
{
    int32_t retVal;
    FILE    *fp=NULL;
    int8_t *mime;
    int32_t offset;
    int32_t i;
    struct stat fileStat;

    if(chdir("/usb0") && chdir("/usb1") && chdir("/usb2")){
        //printf("No MSD Found!\n");
        return -1;
    }

    if(!*filename){
        filename = HTTP_DEFAULT_PAGE;
    }

    for(i=0;i<3;i++){
        snprintf(filePath, sizeof(filePath), WEBSERV_ROOTDIR, i, filename);
        fp = fopen(filename, "rb");
        if(fp){
            //printf("HTTP[Found]: %s\n", filePath);
            break;
        }
    }

    if (fp == NULL) {
        //printf("HTTP[File Not Found]: %s\n", filename);
        goto badfile;
    }

    stat(filePath, &fileStat);

    //printf("FileSize: %i\n", fileStat.st_size);

    // Basic MIME support
    if(strstr(filename, EXTN_HTML)){
        mime = MIME_TYPE_HTML;
    } else if(strstr(filename, EXTN_JPEG)){
        mime = MIME_TYPE_JPEG;
    } else if(strstr(filename, EXTN_PNG)){
        mime = MIME_TYPE_PNG;
    } else if(strstr(filename, EXTN_GIF)){
        mime = MIME_TYPE_GIF;
    }  else {
        mime = MIME_TYPE_UNKW;
    }

    offset = snprintf(sndBuf, sizeof(sndBuf), HTTP_RESP_TEMPLATE, mime, (unsigned int)(fileStat.st_size));

    do{
        retVal = fread(sndBuf + offset, 1, sizeof(sndBuf) - offset, fp);
        if(retVal>0){
            // We must use NETCONN_COPY as the data buf may be retransmitted
            if(netconn_write(conn, sndBuf, retVal + offset, NETCONN_COPY /*NETCONN_NOCOPY*/) != ERR_OK ){
                break;
            }
            offset = 0;
        } else {
            break;
        }
    }while(1);

    fclose(fp);
    return 0;

badfile:
    if(fp)
        fclose(fp);
    return -1;
}

static int netbuf_data_error=0;
static int strstr_error=0;
static int recv_timeouts=0;
////////////////////////////////////////////////////////////////////////////////
static void http_server_serve(struct netconn *conn)
{
    struct netbuf   *inbuf;
    int8_t    *buf;
    u16_t      buflen;
    int8_t    *pathEnd;

    /* Read the data from the port, blocking if nothing yet there.
       We assume the request (the part we care about) is in one(the first) netbuf */

    // Set timeout to 10 Seconds
    conn->recv_timeout = 10000;
    netconn_recv(conn, &inbuf);

    if (netconn_err(conn) == ERR_TIMEOUT)
    {
        recv_timeouts ++;
        printf("recv_timeouts: %d\n", recv_timeouts);
    }

    if (netconn_err(conn) == ERR_OK)
    {
        if (netbuf_data(inbuf, (void **) &buf, &buflen) == ERR_OK)
        {
            // check for HTTP GET (very primitive)
            if ((buflen >= (WEBSERV_TOKEN_GET_LEN + 1)) && (strncmp(buf, WEBSERV_TOKEN_GET, WEBSERV_TOKEN_GET_LEN) == 0))
            {
                pathEnd = strnstr(buf, WEBSERV_TOKEN_HTTP, buflen);
                if (pathEnd != NULL)
                {
                    *pathEnd = 0;

                    do {
                        if (http_serve_file(conn, buf + WEBSERV_TOKEN_GET_LEN)==0) break;
                        if (http_serve_stats(conn, buf + WEBSERV_TOKEN_GET_LEN)==0) break;
                        // page not found
                        netconn_write(conn, http_html_error, sizeof(http_html_error) - 1, NETCONN_NOCOPY);
                    }while(0);
                }
                else
                {
                    strstr_error++;
                }
            }
        }
        else
        {
            netbuf_data_error++;
        }
    }

    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);

    /* Delete the buffer (netconn_recv gives us ownership,
       so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);
}

////////////////////////////////////////////////////////////////////////////////
int http_server(usbhost_t  *usbHost)
{
    struct netconn *conn, *newconn;

    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return -1; );

    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, NULL, 80);

    /* Put the connection into LISTEN state */
    netconn_listen(conn);

    /* get a reference to the underlying USB host */
    USBHost = usbHost;

    /* Initialize stats */
    server_stats_init();

    while (1)
    {
        /* process USB connection changes */
        usbhost_process(USBHost);
        /* Accept incomming connection */
        netconn_accept(conn, &newconn);
        http_server_serve(newconn);
        netconn_delete(newconn);
    }

    /* never reached */
}

