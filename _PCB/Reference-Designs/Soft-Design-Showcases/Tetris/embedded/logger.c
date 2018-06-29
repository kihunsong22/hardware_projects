#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <mqueue.h>
#include "tetris.h"
#include <devctl.h>

pthread_t       logger_thread;

void*   logger_start(void* arg)
{
        mqd_t           mq = (mqd_t)arg;
        unsigned int    priority;
        char            buf[MSG_MAXSIZE];
        ssize_t         reclen;
        volatile int    forever = 1;

        while (forever)
        {
                /* read next log */
                reclen = mq_receive( mq, buf, MSG_MAXSIZE , &priority);

                if (priority == MSG_EXIT)
                {
                        /* display_end_game */
                        display_end_game();
                        /* and cancel other threads */
                        pthread_cancel(input_thread);
                        pthread_cancel(tetris_thread);
                        pthread_exit(NULL);
                }
        }

        return NULL;

}

