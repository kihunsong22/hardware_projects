////////////////////////////////////////////////////////////////////////////////
// server_stats.c

#include "../devices.h"
#include "lwip/opt.h"
#include <string.h>

#if LWIP_STATS /* don't build if not configured for use in lwipopts.h */

#include <lwip.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/api.h>
#include <lwip/def.h>
#include <lwip/stats.h>
#include <lwip/mem.h>

#include <rtc.h>

////////////////////////////////////////////////////////////////////////////////
#define HTTP_RESP_TEMPLATE  "HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-Length: %i\r\n\r\n"
#define MIME_TYPE_HTML      "text/html"
#define MIME_TYPE_JPEG      "image/jpeg"
#define MIME_TYPE_PNG       "image/x-png"
#define MIME_TYPE_GIF       "image/gif"
#define MIME_TYPE_CSS       "text/css"
#define MIME_TYPE_UNKW      "*/\*"

////////////////////////////////////////////////////////////////////////////////
#define SS_STAT_DECLARE_OUTBUF                      int8_t *p = bufp;
#define SS_STAT_TABLE_VALUE_INT(rname, rvalue)      do{bufp += sprintf(bufp,"<tr><td>"#rname"</td><td class='value'>%u</td></tr>", rvalue);}while(0);
#define SS_STAT_TABLE_VALUE_STRING(rname, rvalue)   do{bufp += sprintf(bufp,"<tr><td>"#rname"</td><td class='value'>%s</td></tr>", rvalue);}while(0);
#define SS_STAT_TABLE_START(tname)                  do{bufp += sprintf(bufp,"<h2>%s</h2>",tname);bufp += sprintf(bufp,"<table>");}while(0);
#define SS_STAT_TABLE_END                           do{bufp += sprintf(bufp,"</table>");}while(0);
#define SS_STAT_BUFLEN                              (bufp - p)

////////////////////////////////////////////////////////////////////////////////
typedef struct __static_file_item_t {
    int8_t     *filename;
    int8_t     *bufStart;
    int8_t     *bufEnd;
    int8_t     *mime;
    uint32_t    len;
}static_file_item_t;

////////////////////////////////////////////////////////////////////////////////
extern int8_t _lc_ub_stats_css[];
extern int8_t _lc_ue_stats_css[];

extern int8_t _lc_ub_logo_png[];
extern int8_t _lc_ue_logo_png[];

////////////////////////////////////////////////////////////////////////////////
static static_file_item_t static_file_items[] = {
    {"altium_stats_css.css", _lc_ub_stats_css, _lc_ue_stats_css, MIME_TYPE_CSS, 0},
    {"altium_stats_logo.png",  _lc_ub_logo_png,  _lc_ue_logo_png,  MIME_TYPE_PNG, 0}
                                                };

#define STATIC_FILE_COUNT (sizeof(static_file_items)/sizeof(static_file_item_t))

////////////////////////////////////////////////////////////////////////////////
rtc_t      *myClock;
struct tm   startTime;

////////////////////////////////////////////////////////////////////////////////
static uint32_t http_serve_buf(struct netconn *conn, int8_t *buf, uint32_t bufLen, int8_t *mime)
{
    uint32_t offset;
    uint32_t sndLen;
    //uint8_t sndBuf[1460];
    int8_t sndBuf[256];


    offset = snprintf(sndBuf, sizeof(sndBuf), HTTP_RESP_TEMPLATE, mime, bufLen);

    do{

        if((sizeof(sndBuf) - offset) < bufLen){
            memcpy(sndBuf + offset, buf, (sizeof(sndBuf) - offset));
            bufLen  -= (sizeof(sndBuf) - offset);
            buf     += (sizeof(sndBuf) - offset);
            sndLen = sizeof(sndBuf);
        }else{
            bufLen += offset;
            memcpy(sndBuf + offset, buf, bufLen);
            sndLen  = bufLen;
            buf     += bufLen;
            bufLen  = 0;
        }

        // We must use NETCONN_COPY as the data buf may be retransmitted
        if(netconn_write(conn, sndBuf, sndLen, NETCONN_COPY) != ERR_OK ){
            break;
        }
        offset = 0;

    }while(bufLen);

    return 0;

}

////////////////////////////////////////////////////////////////////////////////
static uint32_t http_server_static_file(struct netconn *conn, int8_t *url)
{
    int32_t i;

    for(i=0;i<STATIC_FILE_COUNT;i++){
        if(strstr(url, static_file_items[i].filename)){
            // Calculate(and cache) static file sizes if not specified
            if(static_file_items[i].len == 0){
                static_file_items[i].len = static_file_items[i].bufEnd - static_file_items[i].bufStart;
            }
            http_serve_buf(conn, static_file_items[i].bufStart, static_file_items[i].len, static_file_items[i].mime);
            //printf("http_server_static_file - FOUND[%s]\n", url);
            return 0;
        }
    }
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
static int server_stats_proto(int8_t *bufp, struct stats_proto *proto, int8_t *name)
{
    SS_STAT_DECLARE_OUTBUF

    SS_STAT_TABLE_START(name);
    SS_STAT_TABLE_VALUE_INT(xmit    , proto->xmit     );
    SS_STAT_TABLE_VALUE_INT(recv    , proto->recv     );
    SS_STAT_TABLE_VALUE_INT(fw      , proto->fw       );
    SS_STAT_TABLE_VALUE_INT(drop    , proto->drop     );
    SS_STAT_TABLE_VALUE_INT(chkerr  , proto->chkerr   );
    SS_STAT_TABLE_VALUE_INT(lenerr  , proto->lenerr   );
    SS_STAT_TABLE_VALUE_INT(memerr  , proto->memerr   );
    SS_STAT_TABLE_VALUE_INT(rterr   , proto->rterr    );
    SS_STAT_TABLE_VALUE_INT(proterr , proto->proterr  );
    SS_STAT_TABLE_VALUE_INT(opterr  , proto->opterr   );
    SS_STAT_TABLE_VALUE_INT(err     , proto->err      );
    SS_STAT_TABLE_VALUE_INT(cachehit, proto->cachehit );
    SS_STAT_TABLE_END

    return SS_STAT_BUFLEN;
}

#if IGMP_STATS
////////////////////////////////////////////////////////////////////////////////
static int server_stats_igmp(int8_t *bufp, struct stats_igmp *igmp)
{
    SS_STAT_DECLARE_OUTBUF

    SS_STAT_TABLE_START("IGMP");
    SS_STAT_TABLE_VALUE_INT(xmit,       igmp->xmit       );
    SS_STAT_TABLE_VALUE_INT(recv,       igmp->recv       );
    SS_STAT_TABLE_VALUE_INT(drop,       igmp->drop       );
    SS_STAT_TABLE_VALUE_INT(chkerr,     igmp->chkerr     );
    SS_STAT_TABLE_VALUE_INT(lenerr,     igmp->lenerr     );
    SS_STAT_TABLE_VALUE_INT(memerr,     igmp->memerr     );
    SS_STAT_TABLE_VALUE_INT(proterr,    igmp->proterr    );
    SS_STAT_TABLE_VALUE_INT(rx_v1,      igmp->rx_v1      );
    SS_STAT_TABLE_VALUE_INT(rx_group,   igmp->rx_group   );
    SS_STAT_TABLE_VALUE_INT(rx_general, igmp->rx_general );
    SS_STAT_TABLE_VALUE_INT(rx_report,  igmp->rx_report  );
    SS_STAT_TABLE_VALUE_INT(tx_join,    igmp->tx_join    );
    SS_STAT_TABLE_VALUE_INT(tx_leave,   igmp->tx_leave   );
    SS_STAT_TABLE_VALUE_INT(tx_report,  igmp->tx_report  );
    SS_STAT_TABLE_END

    return SS_STAT_BUFLEN;
}
#endif /* IGMP_STATS */

#if MEM_STATS || MEMP_STATS
////////////////////////////////////////////////////////////////////////////////
static int server_stats_mem(int8_t *bufp, struct stats_mem *mem, int8_t *name)
{
    SS_STAT_DECLARE_OUTBUF

    bufp += sprintf(bufp, "<h2>MEM %s</h2>", name);
    bufp += sprintf(bufp,"<table>");
    SS_STAT_TABLE_VALUE_INT(avail , (u32_t)mem->avail );
    SS_STAT_TABLE_VALUE_INT(used  , (u32_t)mem->used  );
    SS_STAT_TABLE_VALUE_INT(max   , (u32_t)mem->max   );
    SS_STAT_TABLE_VALUE_INT(err   , (u32_t)mem->err   );
    SS_STAT_TABLE_END

    return SS_STAT_BUFLEN;
}

#if MEMP_STATS
////////////////////////////////////////////////////////////////////////////////
static int server_stats_memp(int8_t *bufp, struct stats_mem *mem, int index)
{
    char * memp_names[] = {
#define LWIP_MEMPOOL(name,num,size,desc) desc,
#include "lwip/memp_std.h"
    };
    if(index < MEMP_MAX) {
        return server_stats_mem(bufp, mem, memp_names[index]);
    }
    return 0;
}
#endif /* MEMP_STATS */
#endif /* MEM_STATS || MEMP_STATS */

#if SYS_STATS
////////////////////////////////////////////////////////////////////////////////
static int server_stats_sys(int8_t *bufp, struct stats_sys *sys)
{
    SS_STAT_DECLARE_OUTBUF

    SS_STAT_TABLE_START("SYS");
    SS_STAT_TABLE_VALUE_INT(sem.used   ,  (u32_t)sys->sem.used  );
    SS_STAT_TABLE_VALUE_INT(sem.max    ,  (u32_t)sys->sem.max   );
    SS_STAT_TABLE_VALUE_INT(sem.err    ,  (u32_t)sys->sem.err   );
  //SS_STAT_TABLE_VALUE_INT(mutex.used , (u32_t)sys->mutex.used );
  //SS_STAT_TABLE_VALUE_INT(mutex.max  , (u32_t)sys->mutex.max  );
  //SS_STAT_TABLE_VALUE_INT(mutex.err  , (u32_t)sys->mutex.err  );
    SS_STAT_TABLE_VALUE_INT(mbox.used  , (u32_t)sys->mbox.used  );
    SS_STAT_TABLE_VALUE_INT(mbox.max   , (u32_t)sys->mbox.max   );
    SS_STAT_TABLE_VALUE_INT(mbox.err   , (u32_t)sys->mbox.err   );
    SS_STAT_TABLE_END

    return SS_STAT_BUFLEN;
}
#endif /* SYS_STATS */

////////////////////////////////////////////////////////////////////////////////
static int server_stats_uptime(int8_t *bufp)
{
    int8_t *p = bufp;

    struct tm   currentTime;
    uint32_t    timeDiff;
    uint32_t    upDays;
    uint32_t    upHours;
    uint32_t    upMinutes;
    uint32_t    upSeconds;

    rtc_get_time(myClock, &currentTime);

    timeDiff = difftime(mktime(&currentTime), mktime(&startTime));

    upDays      = timeDiff / 86400;
    upHours     = (timeDiff / 3600) - (upDays * 24);
    upMinutes   = (timeDiff / 60) - (upDays * 1440) - (upHours * 60);
    upSeconds   = timeDiff % 60;

    SS_STAT_TABLE_START("UPTIME");
    SS_STAT_TABLE_VALUE_STRING  (Start   , asctime(&startTime) );
    SS_STAT_TABLE_VALUE_INT     (Days    , upDays              );
    SS_STAT_TABLE_VALUE_INT     (Hours   , upHours             );
    SS_STAT_TABLE_VALUE_INT     (Mins    , upMinutes           );
    SS_STAT_TABLE_VALUE_INT     (Seconds , upSeconds           );
    SS_STAT_TABLE_END

    return SS_STAT_BUFLEN;
}

////////////////////////////////////////////////////////////////////////////////
void display_server_stats(struct netconn *conn)
{
    int8_t  *buf;
    s16_t    i;
    int8_t  *p;

    // Grab a chuck of memory big enough to hold entire stat page
    buf = (int8_t *)malloc(10*1024);

    do{

    p = buf;

    p += sprintf(p,
        "<!DOCTYPE html>"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
        "<head>"
        "<title>Altium Demo Web Server</title>"
        "<script type=\"text/javascript\"></script>"
        "<style type=\"text/css\" title=\"currentStyle\" media=\"screen\">"
        "@import \"altium_stats_css.css\";"
        "</style>"
        "<meta http-equiv=\"refresh\" content=\"5\" >"
        "</head>"
        "<div class=\"logo\">"
        "<p>&nbsp;</p>"
        "</div>"
        "<div class=\"line\"></div>"
        "<h1>Altium Demo Web Server</h1>"
        "<p>This page lists some statistics for the running system.</p>"
    );

    p += server_stats_uptime(p);
#if LINK_STATS
    p += server_stats_proto(p, &lwip_stats.link, "LINK");
#endif

#if ETHARP_STATS
    p += server_stats_proto(p, &lwip_stats.etharp, "ETHARP");
#endif

#if IPFRAG_STATS
    p += server_stats_proto(p, &lwip_stats.ip_frag, "IP_FRAG");
#endif

#if IP_STATS
    p += server_stats_proto(p, &lwip_stats.ip, "IP");
#endif

#if ICMP_STATS
    p += server_stats_proto(p, &lwip_stats.icmp, "ICMP");
#endif

#if IGMP_STATS
    p += server_stats_igmp(p, &lwip_stats.igmp);
#endif

#if UDP_STATS
    p += server_stats_proto(p, &lwip_stats.udp, "UDP");
#endif

#if TCP_STATS
    p += server_stats_proto(p, &lwip_stats.tcp, "TCP");
#endif

#if MEM_STATS
    p += server_stats_mem(p, &lwip_stats.mem, "HEAP");
#endif

#if MEMP_STATS
    for (i = 0; i < MEMP_MAX; i++) {
        p += server_stats_memp(p, &lwip_stats.memp[i], i);
    }
#endif

#if SYS_STATS
    p += server_stats_sys(p, &lwip_stats.sys);
#endif

    p += sprintf(p, "</html>");

#define MIME_TYPE_HTML      "text/html"

    // Send the page
    http_serve_buf(conn, buf, p-buf, MIME_TYPE_HTML);

    }while(0);

    free(buf);

}

////////////////////////////////////////////////////////////////////////////////
void server_stats_init(void)
{
    myClock = rtc_open(RTC_1);
    rtc_get_time(myClock, &startTime);
    printf("Start Time: %s\n", asctime(&startTime));
}

////////////////////////////////////////////////////////////////////////////////
uint32_t http_serve_stats(struct netconn *conn, int8_t *url)
{

    if(http_server_static_file(conn, url)==0) return 0;
    if(strstr(url, "?action=network_stats")){
        display_server_stats(conn);
        return 0;
    }
    return 1;
}
#endif /* LWIP_STATS */

