/*  NB3000 Webcam Server
 *
 *  This example will show the video stream captures by a USB connected
 *  webcam over an internet connection. To view the stream, point your
 *  browser to the following address:
 *
 *      http://<ip-address>:8080/?action=stream
 *
 *  where <ip-address> is displayed in the terminal window. The IP address
 *  is assigned through DHCP. A static IP address can be set in the network
 *  configuration of the Software Platform.
 */

#include "generic_devices.h"
#include "devices.h"

#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <pthread.h>

#include "swplatform.h"

#include "control.h"
#include "utils.h"

/* default port to connect to */
#define DEFAULT_PORT 8080

/* unique string for separation of images by the HTTP server */
#define BOUNDARY "arflebarfle"

/* capture frame size */
#define FRAME_WIDTH     320
#define FRAME_HEIGHT    240

/* initial frame rate */
#define DEFAULT_FPS     10

/* maximum allowed simultanious connections
 *
 * set to 2 as more connections aren't possible because of lack of SRAM
 * memory. current memory usage is:
 *
 * - USB video camera uses a buffer for 3 full images
 * - camera thread uses an intermediate image buffer
 * - client thread uses an intermediate image buffer
 *
 * this means that initially 4 image buffers are allocated (4x 150K) and
 * for each connection an additional image buffer is allocated (150K). The
 * heap size is 1M so we can only allocate a total of 6 image buffers (900K)
 * before we run out of memory.
 */
#define MAX_CONNECTIONS_ALLOWED 2

/* do we take a snapshot or stream images */
typedef enum {
    SNAPSHOT,
    STREAM
} answer_t;

/* globals */
int                      stop = 0;

/* HTML request strings */
const static char        html_req_snapshot[] = "/?action=snapshot";
const static char        html_req_stream[]   = "/?action=stream";

/* HTML templates */
const static char        http_html_hdr[]   = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char        http_html_error[] = "<html><head><title>Error</title></head><body><H2>HTTP 404 - File not found</H2></body></html>";
const static char        http_html_limit[] = "<html><head><title>Error</title></head><body><H2>Sorry, connection limit reached</H2></body></html>";

/* signal fresh frames */
pthread_mutex_t          db        = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t           db_update = PTHREAD_COND_INITIALIZER;

/* connection count */
pthread_mutex_t          con_count   = PTHREAD_MUTEX_INITIALIZER;
int                      connections = 0;

/* global JPG frame, this is more or less the "database" */
unsigned char           *g_buf = NULL;
int                      g_size = 0;

/* pointers to drivers */
usbhost_uvc_t           *usbuvc;
usbhost_t               *usbhost;

static void webcam_init(void)
{
    printf("Initializing USB video camera...");
    usbuvc  = usbhost_uvc_open(USBHOST_UVC_1);
    if (usbuvc == NULL)
    {
        printf("\nFailed usbhost_uvc_open(USBHOST_UVC_1)\n");
        exit(1);
    }

    usbhost  = usbhost_open(USBHOST_1);
    if (usbhost == NULL)
    {
        printf("\nFailed usbhost_open(USBHOST_1)\n");
        exit(1);
    }
    printf(" done\n");
}

static void wait_for_ip_address(lwip_t* lwip)
{
    struct ip_addr   ip_addr;

    while (true)
    {
        ip_addr = lwip_get_local_addr(lwip);
        if (ip_addr.addr != 0)
        {
            break;
        }
        SLEEP(1, 0);
    }
}

static void print_ip_address(lwip_t *lwip)
{
    struct ip_addr   ip_addr;

    ip_addr = lwip_get_local_addr(lwip);
    ip_addr_print(&ip_addr);
}

static void set_unique_mac (void)
{
    spi_t       *spi;
    bool         got_bus;
    int          i;
    ethernet_t  *ethernet;
    uint8_t      mac[6] = {0};

    spi = spi_open ( DRV_SPI_1 );
    got_bus = spi_get_bus ( spi, DEVICE_1WIRE_NB );
    while ( !got_bus )
        got_bus = spi_get_bus ( spi, DEVICE_1WIRE_NB );

    spi_cs_lo ( spi );
    spi_transmit8 ( spi, 0x03 ); // select memories
    spi_transmit8 ( spi, 0x00 ); // select first device - the NanoBoard

    // MAC is a custom internal mac
    mac[0] = 0x02;
    // Discard first 5 bytes of ID
    for ( i=1; i<7; i++ )
    {
        mac[i] = spi_transceive8 ( spi, 0 );
    }

    spi_cs_hi ( spi );

    spi_release_bus ( spi );


    printf("\nUsing MAC: ");
    for(i=0;i<7;i++){
        printf("%02X", mac[i]);
    }
    printf("\n");

    ethernet = ethernet_open ( ETHERNET_1 );
    ethernet_setmac ( ethernet, mac );

}

static void net_init(void)
{
    lwip_t     *lwip;

    printf("Initializing network...");
    lwip = lwip_open(LWIP_1);
    if (!lwip)
    {
        printf("\nLWIP open failed.\n");
        exit(1);
    }

    set_unique_mac();

    if (lwip_start(lwip) != ERR_OK)
    {
        printf("\nLWIP start failed.\n");
        exit(1);
    }

    wait_for_ip_address(lwip);
    printf(" done\n\n");
    printf("Stream URL: http://");
    print_ip_address(lwip);
    printf(":%d/?action=stream\n", DEFAULT_PORT);
}

/* compare URL */
static int urlcmp(const char *req, const char *url)
{
    for (;; ++req, ++url)
    {
        if ((*url == 0) && (*req == ' ' || *req == '\n'))
        {
            return 1;
        }

        if (toupper(*url) != toupper(*req))
        {
            return 0;
        }
    }
}

/* thread for clients that connected to this server */
static void *client_thread(void *arg)
{
    struct netconn     *conn;
    struct netbuf      *inbuf;
    char               *buf;
    u16_t               buflen;
    char                buffer[1024] = {0};
    unsigned char      *frame;
    err_t               err;
    int                 frame_size = 0;
    answer_t            answer = STREAM;
    int                 limits_reached  = 0;

    if (!arg)
        exit(1);

    /* get connection */
    conn = (struct netconn *)arg;

    /* set timeout to 5 seconds (in msec) */
    conn->recv_timeout = 5 * 1000;

    /* Read the data from the port, blocking if nothing yet there.
       We assume the request (the part we care about) is in one netbuf */
    netconn_recv(conn, &inbuf);

    if (netconn_err(conn) == ERR_OK)
    {
        netbuf_data(inbuf, (void **) &buf, &buflen);

        // check for HTTP GET (very primitive)
        if (buflen >= 5 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == ' ')
        {
            int    found = 0;

            if (urlcmp(buf + 4, html_req_snapshot))
            {
                answer = SNAPSHOT;
                found = 1;
                sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
                                "Server: UVC Streamer\r\n" \
                                "Content-type: image/jpeg\r\n"
                                "\r\n");
            }
            else if (urlcmp(buf + 4, html_req_stream))
            {
                answer = STREAM;
                found = 1;
                sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
                                "Server: UVC Streamer\r\n" \
                                "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
                                "Cache-Control: no-cache\r\n" \
                                "Cache-Control: private\r\n" \
                                "Pragma: no-cache\r\n" \
                                "\r\n" \
                                "--" BOUNDARY "\n");
            }

            if (found)
            {
                /* check if connection limit is reached */
                pthread_mutex_lock(&con_count);
                if (connections >= MAX_CONNECTIONS_ALLOWED)
                {
                    limits_reached = 1;
                    found = 0;
                }
                else
                {
                    connections++;
                }
                pthread_mutex_unlock(&con_count);
            }

            if (found)
            {
                /* allocate image frame buffer */
                frame = (unsigned char *)calloc(1, FRAME_WIDTH * FRAME_HEIGHT * sizeof(uint16_t));

                if (frame != NULL)
                {
                    /* send HTML header */
                    err = netconn_write(conn, buffer, strlen(buffer), NETCONN_NOCOPY);
                    while ((err == ERR_OK) && !stop)
                    {
                        /* wait for fresh frames */
                        pthread_mutex_lock(&db);
                        pthread_cond_wait(&db_update, &db);

                        /* read buffer */
                        frame_size = g_size;
                        memcpy(frame, g_buf, frame_size);

                        /* remove mutex lock */
                        pthread_mutex_unlock(&db);

                        /* output the frame */
                        if (answer == STREAM)
                        {
                            sprintf(buffer, "Content-type: image/jpeg\n\n");
                            err = netconn_write(conn, buffer, strlen(buffer), NETCONN_COPY);
                            if (err != ERR_OK) break;
                        }

                        err = print_picture(conn, frame, frame_size);
                        if (err != ERR_OK || answer == SNAPSHOT) break;

                        sprintf(buffer, "\n--" BOUNDARY "\n");
                        err = netconn_write(conn, buffer, strlen(buffer), NETCONN_COPY);
                        if (err != ERR_OK) break;
                    }

                    /* deallocate the frame */
                    free(frame);
                }
                else
                {
                    netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);
                    netconn_write(conn, http_html_error, sizeof(http_html_error) - 1, NETCONN_NOCOPY);
                }

                /* update the connection count */
                pthread_mutex_lock(&con_count);
                if (connections > 0)
                {
                    connections--;
                }
                pthread_mutex_unlock(&con_count);
            }
            else
            {
                // not found or connection limit reached
                netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);
                if (limits_reached)
                {
                    netconn_write(conn, http_html_limit, sizeof(http_html_limit) - 1, NETCONN_NOCOPY);
                }
                else
                {
                    netconn_write(conn, http_html_error, sizeof(http_html_error) - 1, NETCONN_NOCOPY);
                }
            }
        }
    }

    /* Close the connection (server closes in HTTP) */
    netconn_close(conn);

    /* Delete the buffer (netconn_recv gives us ownership,
       so we have to make sure to deallocate the buffer) */
    netbuf_delete(inbuf);

    /* Delete the connection */
    netconn_delete(conn);

    return NULL;
}

/* image retrieval thread */
static void *webcam_thread(void *arg)
{
    uint8_t           *buf;
    uint32_t           bufsize;
    int                i;
    int                fps = DEFAULT_FPS;
    int32_t            err;

    i = 0;
    while (!stop)
    {
        /* process USB host requests */
        usbhost_process(usbhost);
        if (usbhost_uvc_getstate(usbuvc) == USBHOST_UVC_STATE_CONNECTED)
        {
            fps = usbhost_uvc_init(usbuvc, DEFAULT_FPS);
            printf("Device framerate initialised to %d fps\n", fps);
            err = usbhost_uvc_start(usbuvc);
            if (err < 0)
            {
                printf("Error %d\n", err);
                exit(1);
            }
        }

        /* grab a frame */
        buf = usbhost_uvc_get_frame(usbuvc, &bufsize);
        if (!buf)
        {
            SLEEP(0, 1000*1000*1000/(fps * 2));
            continue;
        }

        /* copy frame to global buffer */
        pthread_mutex_lock(&db);

        g_size = bufsize;;
        memcpy(g_buf, buf, bufsize);

        /* signal fresh_frame */
        pthread_cond_broadcast(&db_update);
        pthread_mutex_unlock(&db);

        /* only use usleep if the fps is below 5, otherwise the overhead is too long */
        if (fps < 5)
        {
            SLEEP(0, 1000*1000*1000/fps);
        }
    }

    return NULL;
}

static void signal_handler(int sigm)
{
     /* signal "stop" to threads */
     stop = 1;

     /* cleanup most important structures */
     fprintf(stderr, "shutdown...\n");
     SLEEP(1, 0);
     usbhost_uvc_stop(usbuvc);
     pthread_cond_destroy(&db_update);
     pthread_mutex_destroy(&db);
     pthread_mutex_destroy(&con_count);

     exit(0);
     return;
}

static int webcam_server(void)
{
    int                   on = 1;
    pthread_t             client;
    pthread_t             cam;
    struct netconn       *conn, *newconn;

    /* ignore SIGPIPE (send if transmitting to closed sockets) */
    signal(SIGPIPE, SIG_IGN);
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        fprintf(stderr, "could not register signal handler\n");
        exit(1);
    }

    /* create a new TCP connection handle */
    conn = netconn_new(NETCONN_TCP);
    LWIP_ERROR("http_server: invalid conn", (conn != NULL), return -1;);

    /* Bind to port 8080 with default IP address */
    netconn_bind(conn, NULL, DEFAULT_PORT);

    /* Put the connection into LISTEN state */
    netconn_listen(conn);

    /* start to read the camera, push picture buffers into global buffer */
    g_buf = (unsigned char *) calloc(1, FRAME_WIDTH * FRAME_HEIGHT * sizeof(uint16_t));

    pthread_create(&cam, NULL, webcam_thread, NULL);
    pthread_detach(cam);

    /* create a child for every client that connects */
    while (true)
    {
        netconn_accept(conn, &newconn);
        if (newconn != NULL)
        {
            pthread_create(&client, NULL, &client_thread, newconn);
            pthread_detach(client);
        }
    }

    /* never reached */
    return 0;
}


void main(void)
{
    printf("Starting USB webcam server...\n");

    webcam_init();
    net_init();

    webcam_server();

    /* never reached */
}


