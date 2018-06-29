/*****************************************************************************
|*  SwPlatform Tetris Example
|*
|* Devices:
|*  - TFT screen (VGA32_TFT)
|*  - UART8 serial device (WB_UART8 )
|*
|* Services used:
|*  - Posix_devio (serial, keyboard)
|*  - Multithreading
|*  - Graphics
|*  - Signalling
|*  - Message Queues
|*
|* Description:
|*      This serial-graphics Tetris example is a showcase for many of the services
|*      offered to designers of multithreading applications.
|*
\*****************************************************************************/
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mqueue.h>
#include "tetris.h"

/* all possible tetris piece types: indexes in 
 * the 'rotate_map' */
typedef enum
{
    left_snake_A,
    left_snake_B,
    left_snake_C,
    left_snake_D,
    right_snake_A,
    right_snake_B,
    right_snake_C,
    right_snake_D,
    T_A,
    T_B,
    T_C,
    T_D,
    left_Gun_A,
    left_Gun_B,
    right_Gun_A,
    right_Gun_B,
    I_A,
    I_B,
    square
} tetris_t;
#define NO_PIECES 19

/* A rotation map : j -> rotate_map(j).
 * Transformation after a 'rotate' action */
tetris_t   rotate_map[NO_PIECES] =
{
    left_snake_B,
    left_snake_C,
    left_snake_D,
    left_snake_A,
    right_snake_B,
    right_snake_C,
    right_snake_D,
    right_snake_A,
    T_B,
    T_C,
    T_D,
    T_A,
    left_Gun_B,
    left_Gun_A,
    right_Gun_B,
    right_Gun_A,
    I_B,
    I_A,
    square
};

#define SHAPE3x3     9

/* 'shape_map' for all pieces: indexed by tetris_t.
 * All pices MUST fit in a 3x3 square. */
int   shape_map[NO_PIECES][SHAPE3x3] =
{
    {
      /* left_ snake A */
      0, 0, 1,
      0, 1, 1,
      0, 1, 1,
    },
    {
      /* left_ snake B */
      0, 0, 0,
      1, 1, 0,
      1, 1, 1,
    },
    {
      /* left_ snake C */
      1, 0, 1,
      1, 0, 1,
      0, 0, 1,
    },
    {
      /* left_ snake D */
      0, 1, 1,
      0, 0, 0,
      1, 1, 1,
    },
    {
      /* right_ snake A */
      0, 1, 1,
      0, 1, 1,
      0, 0, 1,
    },
    {
      /* right_ snake B */
      0, 0, 0,
      0, 1, 1,
      1, 1, 1,
    },
    {
      /* right_ snake C */
      0, 0, 1,
      1, 0, 1,
      1, 0, 1,
    },
    {
      /* right_ snake D */
      1, 1, 0,
      0, 0, 0,
      1, 1, 1,
    },
    {
      /* T A */
      0, 1, 1,
      0, 0, 1,
      0, 1, 1,
    },
    {
      /* T B */
      0, 0, 0,
      1, 0, 1,
      1, 1, 1,
    },
    {
      /* T C */
      1, 0, 1,
      0, 0, 1,
      1, 0, 1,
    },
    {
      /* T D */
      1, 0, 1,
      0, 0, 0,
      1, 1, 1,
    },
    {
      /* left_ Gun A */
      1, 0, 1,
      0, 0, 1,
      0, 1, 1,
    },
    {
      /* left_ Gun B*/
      0, 0, 1,
      1, 0, 0,
      1, 1, 1,
    },
    {
      /* right_ Gun A*/
      0, 1, 1,
      0, 0, 1,
      1, 0, 1,
    },
    {
      /* right_ Gun B*/
      1, 0, 0,
      0, 0, 1,
      1, 1, 1,
    },
    {
      /*  I A */
      1, 0, 1,
      1, 0, 1,
      1, 0, 1,
    },
    {
      /* I B*/
      1, 1, 1,
      0, 0, 0,
      1, 1, 1,
    },
    {
      /* square */
      0, 0, 1,
      0, 0, 1,
      1, 1, 1,
    }
};

/* 'color_map' for all pieces: indexed by tetris_t */
color_t   color_map[NO_PIECES] =
{
    WHITE,
    WHITE,
    WHITE,
    WHITE,
    RED,
    RED,
    RED,
    RED,
    BLUE,
    BLUE,
    BLUE,
    BLUE,
    GREEN,
    GREEN,
    YELLOW,
    YELLOW,
    CYAN,
    CYAN,
    MAGENTA
};


/* score data */
struct score_t
{
    int  points;
    int  pieces;
    int  lines;
    int  phase;
};

/* paint me (tetristype) at given position with given color */
static void paint_piece(int, int, tetris_t, color_t);

/* being me (tetristype) at a given position ,
 * can a move after given event? */ 
static bool move_possible(int, int, tetris_t, int);

/* random generator of a tetris piece */
static tetris_t random_type(void);

/* high stores the highest line occupied by a tetris */
static int high = FIRSTBOARDROW;

static bool inittetris = false;

/* score board statics */
static struct score_t s_board;
static void init_score_board(void);
static void write_score_board(scoreboard_type_t s,int update);
static void update_score_board(void);

/* nscs to timespec convertion */
static  void    _nscs_to_timespec(time_t, struct timespec*);

pthread_t       tetris_thread;

void* tetris_start(void* arg)
{
    int             col, row;
    int             l;
    tetris_t        type;
    bool            piece_is_falling = true;
    int             color;
    int             signal;
    sigset_t        set;
    mqd_t           mq = (mqd_t)arg;
    struct sigevent ev;
    timer_t         timer;
    time_t          period;
    struct          itimerspec ts;
    bool            gravity;
    volatile int    game_in_progress = true;
    bool            paused=false;
    if (inittetris==0)
    {
        board_init(BLACK, WHITE);
        init_score_board();
        inittetris = 1;
    }

    sigemptyset(&set);
    sigaddset(&set,SIGGRAVITY);
    sigaddset(&set,SIGBUTTON1);
    sigaddset(&set,SIGBUTTON2);
    sigaddset(&set,SIGBUTTON3);
    sigaddset(&set,SIGBUTTON4);
    sigaddset(&set,SIGBUTTON5);

    /* create/set a 'one-shot' gravity timer */
    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo  = SIGGRAVITY;
    timer_create(CLOCK_REALTIME, &ev, &timer);
    period = SIGGFIRST_NSCS;
    _nscs_to_timespec(0, &ts.it_interval);
    _nscs_to_timespec( period , &ts.it_value);
    timer_settime(timer, 0, &ts, NULL);

    /* as soon as gravity is on .. pieces fall */
    while (game_in_progress)
    {
        /* start game */

        /* next lifetime of a fallen piece */
        /* coordinates of center of figure */
        row = LASTBOARDROW-1;
        col = LASTBOARDCOLUMN/2;

        /* get random figure */
        type = random_type();
        piece_is_falling = true;
        color = color_map[type];

        /* paint new one or again the old one */
        paint_piece(row, col, type, color);

        /* no restart timer unless indicated */
        gravity = false;

        while (piece_is_falling)
        {
            sigwait(&set,&signal);

            if (paused)
            {
                switch(signal)
                {
                    case SIGBUTTON1: // fallthrough
                    case SIGBUTTON2: // fallthrough
                    case SIGBUTTON3: // fallthrough
                    case SIGBUTTON4: // fallthrough
                    case SIGBUTTON5:
                        timer_create(CLOCK_REALTIME, &ev, &timer);
                        gravity = true;
                        paused  = false;
                        display_buttons_normal();
                        break;
                    default:
                        break;
                }
            }
            else
            {
                switch(signal)
                {
                    case SIGGRAVITY:
                        if (move_possible(row, col, type, MOVE_DOWN))
                        {
                            row -= 1;
                        }
                        else
                        {
                            piece_is_falling = false;
                            high = (high<row+1)?row+1:high;
                        }
                        gravity = true;
                        break;

                    case SIGBUTTON1: // left
                        if (move_possible(row, col, type, MOVE_LEFT))
                        {
                            col -= 1;
                        }
                        break;

                    case SIGBUTTON2: // right
                        if (move_possible(row, col, type, MOVE_RIGHT))
                        {
                            col += 1;
                        }
                        break;

                    case SIGBUTTON3: // rotate
                        if (move_possible(row, col, type, MOVE_ROTATE))
                        {
                            type = rotate_map[type];
                        }
                        break;

                    case SIGBUTTON4: // down
                        /* down until we hit upper part of other Tetris */
                        while (move_possible(row, col, type, MOVE_DOWN))
                        {
                            row -= 1;
                        }
                        piece_is_falling  = false;
                        high = (high<row+1)?row+1:high;
                        break;

                    case SIGBUTTON5: // pause
                        timer_delete(timer);
                        paused = true;
                        display_buttons_paused();
                        break;

                    default:
                        break;
                 }

                if (high == LASTBOARDROW)
                {
                    /* game is over: highest Tetris touches upper
                    * part of the board */
                    high =  FIRSTBOARDROW;
                    /* leave while(piece_is_falling) and prepares new board */
                    #define GAME_OVER       "\n   Game Over    "
                    mq_send(mq, GAME_OVER, sizeof(GAME_OVER)-1, MSG_EXIT);
                    game_in_progress=false;
                }
                else
                {
                    /* paint new one or again the old one */
                    paint_piece(row, col, type, color);
                    /* a tetris piece has gone all the way down */
                    if (!piece_is_falling)
                    {
                        /* end of tetris piece life */
                        write_score_board(S_PIECE,1);

                        /* check possible completed lines */
                        for (l=row+1; l>=FIRSTBOARDROW && l>=row-1; l--)
                        {
                            if(full_line(l))
                            {
                                /* line is completed */
                                write_score_board(S_LINE, 1);
                                /* erase completed line, update board */
                                shift_down_lines(l, high);
                                /* paint it */
                                paint_between_lines(l, high);
                                /* high is one less */
                                high--;
                            }
                        }

                        /* new score */
                        update_score_board();
                    }
                }

            }
            if (gravity)
            {
                /* re-start gravity timer */
                _nscs_to_timespec( period - s_board.phase*SIGGDELTA_NSCS,
                                &ts.it_value);
                timer_settime(timer, 0, &ts, NULL);
                gravity = false;
            }
        } // while (piece_is_falling)

     } // while 1


     /* compiler warning */
    return NULL;
}



static void paint_piece(int y0, int x0, tetris_t type, color_t color)
{
    int x, y;

    int* ptr = (int*)&shape_map[type];

    for (y = y0 + 1; y >= y0 - 1; y--)
    {
        for (x=x0-1; x<=x0+1;x++)
        {
            if (*ptr++ == 0)
            {
                if ( y >= FIRSTBOARDROW && y <= LASTBOARDROW &&
                     x >= FIRSTBOARDCOLUMN && x <= LASTBOARDCOLUMN)
                {
                    paint_cell(y, x, color);
                }
            }
        }
    }
}

static bool move_possible(int yo, int xo, tetris_t type, int move_type)
{
    int xc=xo;
    int yc=yo;
    int x;
    int y;

    int* ptr;

    /* erase: paint it black */
    paint_piece(yo, xo, type, BLACK);

    switch (move_type)
    {
        case MOVE_ROTATE:
                /* position does not change, yes type */
                type = rotate_map[type];
                break;
        case MOVE_DOWN:
                yc = yo - 1;
                break;
        case MOVE_RIGHT:
                /* one right */
                xc = xo + 1;
                break;
        case MOVE_LEFT:
                /* one left */
                xc = xo - 1;
                break;
        default:
                break;
    }

    /* do I fit in my new position ? */
    ptr = (int*)&shape_map[type];

    for (y = yc+1; y>=yc-1 ;y--)
    {
        for (x= xc-1; x<=xc+1;x++)
        {
            if (    *ptr++ == 0 && y>=FIRSTBOARDROW-1 && x>=FIRSTBOARDCOLUMN-1 &&
                    x<=LASTBOARDCOLUMN+1 && !free_cell(y,x) )
            {
                return false;
            }
        }
    }

    return true;
}


static tetris_t random_type(void)
{
    unsigned int var;
    /* default */
    tetris_t tet = left_snake_A;

    /* get a random number with timer interrupt */
    var = 1+(int) (10.0*rand()/(RAND_MAX+1.0));

    switch(var)
    {
        case 0: break;
        case 1: tet = right_snake_A; break;
        case 2: tet = T_A; break;
        case 3: tet = I_A; break;
        case 4: tet = left_Gun_A; break;
        case 5: tet = right_Gun_A; break;
        case 6: tet = square; break;
        case 7: tet = T_C; break;
        case 8: tet = left_snake_A; break;
        case 9: tet = square; break;
        default: break ;
    }

    return tet;
}

static void init_score_board(void)
{
    s_board.points = 0;
    s_board.pieces = 0;
    s_board.lines = 0;
    s_board.phase = 0;
}

static void write_score_board(scoreboard_type_t s, int update)
{
    switch(s)
    {
        case S_PIECE:
                s_board.pieces += update;
                s_board.points += 2*update;
                break;
        case S_LINE:
                s_board.lines  += update;
                s_board.points += 7*update;
                if (s_board.lines >= NO_LINES_PHASE * (s_board.phase+1))
                {
                        s_board.phase++;
                }
                break;
        default: break;
    }
}

static void update_score_board(void)
{
    int po, li, pi, ph;

    po = s_board.points;
    li = s_board.lines;
    pi = s_board.pieces;
    ph = s_board.phase;

    /* display score data: target */
    display_score_board(po, li, pi, ph);

    return;
}


static void _nscs_to_timespec(time_t nsecs, struct timespec* tmspec)
{
    if (nsecs == 0)
    {
        tmspec->tv_sec = 0;
        tmspec->tv_nsec = 0;
    }
    else
    {
        tmspec->tv_sec =  (nsecs / 1000000000l);
        tmspec->tv_nsec = (nsecs % 1000000000l);
    }
}

