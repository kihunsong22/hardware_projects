#ifndef _LWIP_UTILS_H_
#define _LWIP_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <lwip/api.h>

extern void print_ip_address(bool pad, struct ip_addr* ip_address);
extern void sprint_ip_address(char* dest, bool pad, struct ip_addr* ip_address);

extern int socket_read_line (int socket, char* data, int max_length);
extern int socket_read_count(int socket, char* data, int max_count);

#endif
