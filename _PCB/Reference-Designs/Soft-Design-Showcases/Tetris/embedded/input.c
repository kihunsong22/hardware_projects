#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <mqueue.h>
#include "devices.h"
#include "tetris_conf.h"
#include "tetris.h"
#include "drv_ioport.h"

#define BUTTON_COUNT 5
#define DEBOUNCE 100

pthread_t       input_thread;

static bool stroke_action( char key);

char input_event_from_buttons(char buttons)
{
    static bool sw_is_up   [BUTTON_COUNT] = {true, true, true, true, true};
    static int  sw_up_count[BUTTON_COUNT] = {0, 0, 0, 0, 0};

    int i;
    char button_value;
    char result=0;

    for ( i=0; i<BUTTON_COUNT; i++ )
    {
        // for each button, it registers an event when it first goes down,
        // as long as it has been up DEBOUNCE times
        button_value = (buttons >> i) & 0x01;
        if (!button_value)
        {
            // button is up
            sw_up_count[i]++;
            if (sw_up_count[i] >= DEBOUNCE)
                sw_is_up[i] = true;
        }
        else
        {
            // button is down
            sw_up_count[i] = 0;
            if (sw_is_up[i])
            {
                result = result | (1 << i);
                sw_is_up[i] = false;
            }
        }
    }
    return result;
}

void*   input_start(void* arg)
{
    char            buttons;
    volatile int    stop = 0;
    mqd_t           mq = (mqd_t)arg;
    ioport_t*       button_port;

    button_port = ioport_open(BUTTONS);

    while (!stop)
    {
        /* read one character from input */
        buttons = ioport_get_value(button_port, 0);
        buttons = ~buttons & 0x1F;
        buttons = input_event_from_buttons(buttons);
        /* stroke actions */
        if ( stroke_action(buttons) == false )
        {
            /* send exit msg to logger */
            #define ENDED_BY_USER   "\n  ended by user"
            mq_send(mq, ENDED_BY_USER, sizeof(ENDED_BY_USER)-1, MSG_EXIT);
        }
    }

    return NULL;
}

static bool stroke_action( char buttons)
{
    bool    ret = true;

    switch (buttons)
    {
        case 0x01: pthread_kill(tetris_thread,SIGBUTTON1); break;
        case 0x02: pthread_kill(tetris_thread,SIGBUTTON2); break;
        case 0x04: pthread_kill(tetris_thread,SIGBUTTON3); break;
        case 0x08: pthread_kill(tetris_thread,SIGBUTTON4); break;
        case 0x10: pthread_kill(tetris_thread,SIGBUTTON5); break;
        default:
                break;
    }

    return true;
}


