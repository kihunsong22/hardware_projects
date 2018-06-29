#include <lwip.h>
#include <lwip/err.h>
#include <lwip/api.h>

#define SLEEP(seconds, nanoseconds) {              \
                struct timespec tv;                \
                tv.tv_sec = (seconds);             \
                tv.tv_nsec = (nanoseconds);        \
                while (nanosleep(&tv, &tv) == -1); \
        }

extern err_t print_picture(struct netconn *conn, unsigned char *buf, int size);

