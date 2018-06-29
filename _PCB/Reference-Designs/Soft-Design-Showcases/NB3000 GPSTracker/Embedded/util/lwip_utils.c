#include "lwip/sockets.h"
#include "lwip_utils.h"

void print_ip_address(bool pad, struct ip_addr* ip_address)
{
    int32_t addr0 = (ip_address->addr & 0xFF000000) >> 24;
    int32_t addr1 = (ip_address->addr & 0x00FF0000) >> 16;
    int32_t addr2 = (ip_address->addr & 0x0000FF00) >>  8;
    int32_t addr3 =  ip_address->addr & 0x000000FF       ;
    if (pad)
        printf("%3d.%3d.%3d.%3d", addr0, addr1, addr2, addr3);
    else
        printf("%d.%d.%d.%d", addr0, addr1, addr2, addr3);
}

void sprint_ip_address(char* dest, bool pad, struct ip_addr* ip_address)
{
    int32_t addr0 = (ip_address->addr & 0xFF000000) >> 24;
    int32_t addr1 = (ip_address->addr & 0x00FF0000) >> 16;
    int32_t addr2 = (ip_address->addr & 0x0000FF00) >>  8;
    int32_t addr3 =  ip_address->addr & 0x000000FF       ;
    if (pad)
        sprintf(dest, "%3d.%3d.%3d.%3d", addr0, addr1, addr2, addr3);
    else
        sprintf(dest, "%d.%d.%d.%d", addr0, addr1, addr2, addr3);
}

int socket_read_line (int socket, char* data, int max_length)
{
    int bytes = 0;
    while (bytes < max_length-1)
    {
        if (read ( socket, data, 1 ) != 1)
        {
            return -1;
        }
        bytes++;
        if (*data == '\n') break;
        data++;
    }
    if ( data[bytes-2] == '\r' )
    {
        data--;
        bytes--;
    }
    *data = 0;
    return bytes;
}

int socket_read_count(int socket, char* data, int max_count)
{
    int total=0;
    int this_read=0;

    while (total < max_count)
    {
        this_read = read ( socket, data, max_count-total );
        if (this_read <= 0) break;
        data  += this_read;
        total += this_read;
    }
    return total;
}

