
#include <pthread.h>
#include <time.h>

#include "tetris.h"


pthread_t       update_info_thread;

void *update_info_start(void *p)
{
    volatile int forever = 1;

    struct timespec ts = { 1, 0 };
    while (forever)
    {
        display_info();
        nanosleep ( &ts, NULL );
        display_info();
    }
    return NULL;
}
