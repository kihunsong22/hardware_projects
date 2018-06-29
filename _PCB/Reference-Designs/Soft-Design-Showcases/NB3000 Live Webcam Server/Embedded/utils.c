#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "huffman.h"

static int is_huffman(unsigned char *buf)
{
    unsigned char *ptbuf;
    int i = 0;
    ptbuf = buf;
    while (((ptbuf[0] << 8) | ptbuf[1]) != 0xffda)
    {
       if (i++ > 2048)
          return 0;
       if (((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
          return 1;
       ptbuf++;
    }
    return 0;
}

err_t print_picture(struct netconn *conn, unsigned char *buf, int size)
{
    unsigned char *ptdeb, *ptcur = buf;
    int            sizein;
    err_t          err;

    if (!is_huffman(buf))
    {
        ptdeb = ptcur = buf;
        while (((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
            ptcur++;
        sizein = ptcur - ptdeb;

        if ((err = netconn_write(conn, buf, sizein, NETCONN_COPY)) != ERR_OK)
            return err;

        if ((err = netconn_write(conn, dht_data, DHT_SIZE, NETCONN_COPY)) != ERR_OK)
            return err;

        if ((err = netconn_write(conn, ptcur, size - sizein, NETCONN_COPY)) != ERR_OK)
            return err;
    }
    else
    {
        if ((err = netconn_write(conn, ptcur, size, NETCONN_COPY)) != ERR_OK)
            return err;
    }

    return ERR_OK;
}
