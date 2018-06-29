/* LWIP user configuration file
 *
 * This file consists of all configuration settings normally set in lwipopts.h
 * but as we cannot set individual options in this file a full copy is made and
 * where necessary settings are adjusted.
 */

#ifndef __LWIPOPTS_USER_H__
#define __LWIPOPTS_USER_H__   1

#include <string.h>

#ifdef NDEBUG
#define LWIP_NOASSERT 1
#undef LWIP_DEBUG
#endif /* NDEBUG */

/* ---------- NETIF options ---------- */
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1

/* ---------- Memory options ---------- */
#define mem_realloc(mem, size)  (mem)
extern void *memcpyl( void * restrict s, const void * restrict ct, size_t n );
#define MEMCPY(dst,src,len)     memcpyl(dst,src,len)

#define MEM_ALIGNMENT           4
#define MEM_SIZE                (32 * 1024)
#define MEMP_NUM_PBUF           16
#define MEMP_NUM_UDP_PCB        4
#define MEMP_NUM_TCP_PCB        5
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_TCP_SEG        32
#define MEMP_NUM_NETBUF         4                   /* was 2 */
#define MEMP_NUM_NETCONN        6                   /* was 6 */
#define MEMP_NUM_TCPIP_MSG_API   8
#define MEMP_NUM_TCPIP_MSG_INPKT 8
#define MEM_RECLAIM             1
#define MEMP_RECLAIM            1

/* ---------- Pbuf options ---------- */
#define PBUF_POOL_SIZE          100
#define PBUF_POOL_BUFSIZE       256
#define PBUF_LINK_HLEN          16

/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define TCP_TTL                 255
#define TCPIP_THREAD_PRIO       POSIX_THREADS_MAIN_PRIORITY
#define TCP_QUEUE_OOSEQ         1
#define TCP_MSS                 1460
#define TCP_SND_BUF             (16 * TCP_MSS)
#define TCP_SND_QUEUELEN        (4 * TCP_SND_BUF/TCP_MSS)
#define TCP_WND                 (16 * 1024)
#define TCP_MAXRTX              12
#define TCP_SYNMAXRTX           4
#define ARP_TABLE_SIZE          10
#define ARP_QUEUEING            1

/* ---------- IP options ---------- */
#define IP_FORWARD              0
#define IP_OPTIONS              1

/* ---------- ICMP options ---------- */
#define ICMP_TTL                255

/* ---------- DHCP options ---------- */
#define DHCP_DOES_ARP_CHECK     LWIP_DHCP

/* ---------- DNS options ---------- */
#define MEMP_NUM_SYS_TIMEOUT    (3 + LWIP_TCP + IP_REASSEMBLY + LWIP_ARP + (2*LWIP_DHCP) + LWIP_AUTOIP + LWIP_IGMP + LWIP_DNS + PPP_SUPPORT)

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255
#define LWIP_SO_RCVTIMEO        1

/* ------ Statistics options ------- */
#if LWIP_STATS
#define LINK_STATS 1
#define IP_STATS 1
#define ICMP_STATS 1
#define UDP_STATS 1
#define TCP_STATS 1
#define MEM_STATS 1
#define MEMP_STATS 1
#define PBUF_STATS 1
#define SYS_STATS 1
#else
#define LINK_STATS 0
#define IP_STATS 0
#define ICMP_STATS 0
#define UDP_STATS 0
#define TCP_STATS 0
#define MEM_STATS 0
#define MEMP_STATS 0
#define PBUF_STATS 0
#define SYS_STATS 0
#endif /* STATS */


/**
* LWIP_DBG_MIN_LEVEL: After masking, the value of the debug is
* compared against this value. If it is smaller, then debugging
* messages are written.  */
#ifndef LWIP_DBG_MIN_LEVEL
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_WARNING
#endif

/**  * LWIP_DBG_TYPES_ON: A mask that can be used to globally enable/disable
* debug messages of certain types.  */
#ifndef LWIP_DBG_TYPES_ON
#define LWIP_DBG_TYPES_ON               (~LWIP_DBG_TRACE)
#endif

/**  * ETHARP_DEBUG: Enable debugging in etharp.c.  */
#ifndef ETHARP_DEBUG
#define ETHARP_DEBUG                    LWIP_DBG_OFF
#endif

/**  * NETIF_DEBUG: Enable debugging in netif.c.  */
#ifndef NETIF_DEBUG
#define NETIF_DEBUG                     LWIP_DBG_OFF
#endif

/**  * PBUF_DEBUG: Enable debugging in pbuf.c.  */
#ifndef PBUF_DEBUG
#define PBUF_DEBUG                      LWIP_DBG_OFF
#endif

/**  * API_LIB_DEBUG: Enable debugging in api_lib.c.  */
#ifndef API_LIB_DEBUG
#define API_LIB_DEBUG                   LWIP_DBG_OFF
#endif

/**  * API_MSG_DEBUG: Enable debugging in api_msg.c.  */
#ifndef API_MSG_DEBUG
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#endif

/**  * SOCKETS_DEBUG: Enable debugging in sockets.c.  */
#ifndef SOCKETS_DEBUG
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#endif

/**  * ICMP_DEBUG: Enable debugging in icmp.c.  */
#ifndef ICMP_DEBUG
#define ICMP_DEBUG                      LWIP_DBG_OFF
#endif

/**  * IGMP_DEBUG: Enable debugging in igmp.c.  */
#ifndef IGMP_DEBUG
#define IGMP_DEBUG                      LWIP_DBG_OFF
#endif

/**  * INET_DEBUG: Enable debugging in inet.c.  */
#ifndef INET_DEBUG
#define INET_DEBUG                      LWIP_DBG_OFF
#endif

/**  * IP_DEBUG: Enable debugging for IP.  */
#ifndef IP_DEBUG
#define IP_DEBUG                        LWIP_DBG_OFF
#endif

/**  * IP_REASS_DEBUG: Enable debugging in ip_frag.c for both frag & reass.  */
#ifndef IP_REASS_DEBUG
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#endif

/**  * RAW_DEBUG: Enable debugging in raw.c.  */
#ifndef RAW_DEBUG
#define RAW_DEBUG                       LWIP_DBG_OFF
#endif

/**  * MEM_DEBUG: Enable debugging in mem.c.  */
#ifndef MEM_DEBUG
#define MEM_DEBUG                       LWIP_DBG_OFF
#endif

/**  * MEMP_DEBUG: Enable debugging in memp.c.  */
#ifndef MEMP_DEBUG
#define MEMP_DEBUG                      LWIP_DBG_OFF
#endif

/**  * SYS_DEBUG: Enable debugging in sys.c.  */
#ifndef SYS_DEBUG
#define SYS_DEBUG                       LWIP_DBG_OFF
#endif

/**  * TCP_DEBUG: Enable debugging for TCP.  */
#ifndef TCP_DEBUG
#define TCP_DEBUG                       LWIP_DBG_OFF
#endif

/**  * TCP_INPUT_DEBUG: Enable debugging in tcp_in.c for incoming debug.  */
#ifndef TCP_INPUT_DEBUG
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#endif

/**  * TCP_FR_DEBUG: Enable debugging in tcp_in.c for fast retransmit.  */
#ifndef TCP_FR_DEBUG
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#endif

/**  * TCP_RTO_DEBUG: Enable debugging in TCP for retransmit  * timeout.  */
#ifndef TCP_RTO_DEBUG
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#endif

/**  * TCP_CWND_DEBUG: Enable debugging for TCP congestion window.  */
#ifndef TCP_CWND_DEBUG
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#endif

/**  * TCP_WND_DEBUG: Enable debugging in tcp_in.c for window updating.  */
#ifndef TCP_WND_DEBUG
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#endif

/**  * TCP_OUTPUT_DEBUG: Enable debugging in tcp_out.c output functions.  */
#ifndef TCP_OUTPUT_DEBUG
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#endif

/**  * TCP_RST_DEBUG: Enable debugging for TCP with the RST message.  */
#ifndef TCP_RST_DEBUG
#define TCP_RST_DEBUG                   LWIP_DBG_OFF
#endif

/**  * TCP_QLEN_DEBUG: Enable debugging for TCP queue lengths.  */
#ifndef TCP_QLEN_DEBUG
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#endif

/**  * UDP_DEBUG: Enable debugging in UDP.  */
#ifndef UDP_DEBUG
#define UDP_DEBUG                       LWIP_DBG_OFF
#endif

/**  * TCPIP_DEBUG: Enable debugging in tcpip.c.  */
#ifndef TCPIP_DEBUG
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#endif

/**  * PPP_DEBUG: Enable debugging for PPP.  */
#ifndef PPP_DEBUG
#define PPP_DEBUG                       LWIP_DEBUG          // PPP_DEBUG doesn't use the LWIP debugging infrastructure
#endif

/**  * SLIP_DEBUG: Enable debugging in slipif.c.  */
#ifndef SLIP_DEBUG
#define SLIP_DEBUG                      LWIP_DBG_OFF
#endif

/**  * DHCP_DEBUG: Enable debugging in dhcp.c.  */
#ifndef DHCP_DEBUG
#define DHCP_DEBUG                      LWIP_DBG_OFF
#endif

/**  * AUTOIP_DEBUG: Enable debugging in autoip.c.  */
#ifndef AUTOIP_DEBUG
#define AUTOIP_DEBUG                    LWIP_DBG_OFF
#endif

/**  * SNMP_MSG_DEBUG: Enable debugging for SNMP messages.  */
#ifndef SNMP_MSG_DEBUG
#define SNMP_MSG_DEBUG                  LWIP_DBG_OFF
#endif

/**  * SNMP_MIB_DEBUG: Enable debugging for SNMP MIBs.  */
#ifndef SNMP_MIB_DEBUG
#define SNMP_MIB_DEBUG                  LWIP_DBG_OFF
#endif

/**  * DNS_DEBUG: Enable debugging for DNS.  */
#ifndef DNS_DEBUG
#define DNS_DEBUG                       LWIP_DBG_OFF
#endif

#endif /* __LWIPOPTS_USER_H__ */
