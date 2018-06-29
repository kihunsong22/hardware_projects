#include <string.h>
#include <ctype.h>

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include <per_ioport.h>

#include "devices.h"

#define LEDS (*IOPORT_BASE8(per_ioport_get_base_address(PRTIO)))


#include "httpdef_fixed.h"

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_html_error[] =
    "<html><head><title>Error</title></head><body><H2>HTTP 404 - File not found</H2></body></html>";

const static char http_html_begin[] = "<html><head><body>";
const static char http_html_end[] = "</body></html>";

char htmlbuf[1024];

typedef struct
{
    const char *url;
    const uint8_t *pagedata;
    int len;
}
httppage_t;

httppage_t httppages[] = HTTP_PAGES;

char *cgi_get_leds(char *data);
char *cgi_set_leds(char *data);

typedef struct
{
    char *url;
    char *(*pagedata)(char *data);
}
cgipage_t;

cgipage_t cgipages[] =
{
    { "/cgi/get?leds ", &cgi_get_leds },
    { "/cgi/set?leds=", &cgi_set_leds },
    { NULL, NULL }
};


int itoa(int num, char *buf)
{
#define ITOA_SIZE 5
    int pos;
    char tmpbuf[ITOA_SIZE + 1];
    pos = ITOA_SIZE;

    tmpbuf[pos] = 0;
    do
    {
        tmpbuf[--pos] = '0' + (num % 10);
        num = num / 10;
    }
    while (num != 0);

    strcpy(buf, tmpbuf + pos);

    return ITOA_SIZE - pos;
}


char *cgi_get_leds(char *data)
{
    static char cgibuf[6];

    data = data;

    itoa(LEDS, cgibuf);

    return cgibuf;
}


char *cgi_set_leds(char *data)
{
    LEDS = (uint8_t)atoi(data);

    return cgi_get_leds(data);
}


int cgicmp(const char *req, const char *url)
{
    int offset = 0;

    for (;; ++req, ++url, ++offset)
    {
        if (*url == 0)
        {
            return offset;
        }

        if (toupper(*url) != toupper(*req))
        {
            return 0;
        }
    }
}


int urlcmp(const char *req, const char *url)
{
    for (;; ++req, ++url)
    {
        if ((*url == 0) && (*req == ' '))
        {
            return 1;
        }

        if (toupper(*url) != toupper(*req))
        {
            return 0;
        }
    }
}


void http_server_serve(struct netconn *conn)
{
    struct netbuf *inbuf;
    char *buf;
    u16_t buflen;
    httppage_t *httppage;
    cgipage_t *cgipage;

    /* Read the data from the port, blocking if nothing yet there.
       We assume the request (the part we care about) is in one netbuf */
    // Set timeout to 5 Minutes
    conn->recv_timeout = 300000;
    netconn_recv(conn, &inbuf);

    if (netconn_err(conn) == ERR_OK)
    {
        netbuf_data(inbuf, (void **) &buf, &buflen);

        // check for HTTP GET (very primitive)
        if (buflen >= 5 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == ' ')
        {
            int found = 0;

            // send HTML header
            netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);

            for (httppage = httppages; httppage->url; ++httppage)
            {
                if (urlcmp(buf + 4, httppage->url))
                {
                    // our HTML page
                    netconn_write(conn, httppage->pagedata, httppage->len, NETCONN_NOCOPY);
                    found = 1;
                    break;
                }
            }

            if (found == 0)
            {
                for (cgipage = cgipages; cgipage->url; ++cgipage)
                {
                    int offset = cgicmp(buf + 4, cgipage->url);

                    if (offset)
                    {
                        // our generated HTML page body
                        char *cgibuf = cgipage->pagedata(buf + 4 + offset);

                        strcpy(htmlbuf, http_html_begin);
                        strcat(htmlbuf, cgibuf);
                        strcat(htmlbuf, http_html_end);

                        netconn_write(conn, cgibuf, strlen(cgibuf), NETCONN_NOCOPY);
                        found = 1;
                        break;
                    }
                }
            }

            if (found == 0)
            {
                // send "page not found" page
                netconn_write(conn, http_html_error, sizeof(http_html_error) - 1, NETCONN_NOCOPY);
            }

        }
    }

    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);

    /* Delete the buffer (netconn_recv gives us ownership,
       so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);
}


int http_server()
{
    struct netconn *conn, *newconn;

    /* Create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return -1; );

    /* Bind to port 80 (HTTP) with default IP address */
    netconn_bind(conn, NULL, 80);

    /* Put the connection into LISTEN state */
    netconn_listen(conn);

    while (1)
    {
        netconn_accept(conn, &newconn);
        http_server_serve(newconn);
        netconn_delete(newconn);
    }

    /* never reached */
}
