////////////////////////////////////////////////////////////////////////////////
// server_stats.h

#ifndef __SERVER_STATS_H
#define __SERVER_STATS_H

#include <stdint.h>
#include <lwip/api.h>

#if LWIP_STATS
////////////////////////////////////////////////////////////////////////////////
extern void server_stats_init(void);
extern uint32_t http_serve_stats(struct netconn *conn, int8_t *url);
#else
#define server_stats_init(...)
#define http_serve_stats(...) (-1)

#endif


#endif // __SERVER_STATS_H
