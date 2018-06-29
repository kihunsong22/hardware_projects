
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <devices.h>
#include "tetris.h"

/* no gain of having stacks created dynamically:
 * threads run always.
 * Give'em enough space. */
static char input_thread_stack[6*PTHREAD_STACK_MIN];
static char tetris_thread_stack[12*PTHREAD_STACK_MIN];
static char logger_thread_stack[6*PTHREAD_STACK_MIN];
static char update_info_thread_stack[6*PTHREAD_STACK_MIN];


graphics_t* graphics;
canvas_t *canvas;

int main(int argc, char* argv[])
{
        sigset_t                set;
        pthread_attr_t          attr;
        struct sched_param      schedparam;
        mqd_t                   mq_send;
        mqd_t                   mq_rec;
        struct mq_attr          attribute;
        mode_t                  mode;

        /* common attr settings */
        pthread_attr_init(&attr);
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

        /* all created threads inherit all signals blocked */
        sigfillset(&set);
        pthread_sigmask(SIG_BLOCK, &set, NULL);

        /* open devices */
        graphics = graphics_open(GRAPHICS_1);
        if ( graphics == NULL ) return -1;
        canvas = graphics_get_visible_canvas(graphics);

        /* open ipc channel: logger threa d is receiver, other senders */
        attribute.mq_flags = 0;
        attribute.mq_maxmsg = MSG_MAXMSG;
        attribute.mq_msgsize = MSG_MAXSIZE;
        mode = S_IRWXU | S_IRWXG | S_IRWXO;
        mq_send = mq_open("mq_log", O_WRONLY | O_CREAT, mode, &attribute);
        mq_rec  = mq_open("mq_log", O_RDONLY );

        /* info */
        schedparam.sched_priority = UPDATE_INFO_THREAD_PRIORITY;
        pthread_attr_setschedparam(&attr, &schedparam);
        pthread_attr_setstackaddr(&attr, update_info_thread_stack);
        pthread_attr_setstacksize(&attr, sizeof(update_info_thread_stack));
        pthread_create(&update_info_thread, &attr, update_info_start, NULL);

        /* input */
        schedparam.sched_priority = INPUT_THREAD_PRIORITY;
        pthread_attr_setschedparam(&attr, &schedparam);
        pthread_attr_setstackaddr(&attr, (void *)&input_thread_stack[0]);
        pthread_attr_setstacksize(&attr, sizeof(input_thread_stack));
        pthread_create(&input_thread, &attr, input_start, mq_send);

        /* logger */
        schedparam.sched_priority = LOGGER_THREAD_PRIORITY;
        pthread_attr_setschedparam(&attr, &schedparam);
        pthread_attr_setstackaddr(&attr, (void *)&logger_thread_stack[0]);
        pthread_attr_setstacksize(&attr, sizeof(logger_thread_stack));
        pthread_create(&logger_thread, &attr, logger_start, mq_rec);

        /* tetris */
        schedparam.sched_priority = TETRIS_THREAD_PRIORITY;
        pthread_attr_setschedparam(&attr, &schedparam);
        pthread_attr_setstackaddr(&attr, (void *)&tetris_thread_stack[0]);
        pthread_attr_setstacksize(&attr, sizeof(tetris_thread_stack));
        pthread_create(&tetris_thread, &attr, tetris_start, mq_send);

        /* join created threads in reverse termination order */
        pthread_join(update_info_thread, NULL);
        pthread_join(input_thread, NULL);
        pthread_join(logger_thread, NULL);
        pthread_join(tetris_thread, NULL);

        /* and finally exits */
        return 1;
}

