/******************************************************************************
 * FILE:        @(#)tetris.h    1.3 05/12/02
 * DESCRIPTION:
 *      tetris interface
 *****************************************************************************/
#ifndef _H_TETRIS
#define _H_TETRIS

#include <stdbool.h>
#include <graphics.h>
#include "tetris_conf.h"

/* defines */

/* real time signals: for tetris thread */
#define SIGGRAVITY                      SIGRTMIN
#define SIGBUTTON1                      (SIGGRAVITY+1)
#define SIGBUTTON2                      (SIGBUTTON1+1)
#define SIGBUTTON3                      (SIGBUTTON2+1)
#define SIGBUTTON4                      (SIGBUTTON3+1)
#define SIGBUTTON5                      (SIGBUTTON4+1)
#define SIGUPDATE                       (SIGBUTTON5+1)

#define MOVE_LEFT    0
#define MOVE_RIGHT   1
#define MOVE_DOWN    2
#define MOVE_ROTATE  3

/* first/last board lines */
#define FIRSTBOARDROW                   1
#define LASTBOARDROW                    H_BOARD
#define FIRSTBOARDCOLUMN                1
#define LASTBOARDCOLUMN                 W_BOARD

/* enumeration of score-board types */
typedef enum
{
    S_POINT  = 0,
    S_LINE    = 1,
    S_PIECE   = 2,
    S_PHASE   = 3
} scoreboard_type_t;


extern graphics_t* graphics;
extern canvas_t *canvas;

/* threads */
extern pthread_t        input_thread;
extern pthread_t        logger_thread;
extern pthread_t        tetris_thread;
extern pthread_t        update_info_thread;
extern void     *input_start(void*);
extern void     *logger_start(void*);
extern void     *tetris_start(void*);
extern void     *update_info_start(void*);

/* board library ( user threads: tetris ) */
extern void     board_init(color_t,color_t);
extern bool     free_cell(int, int);
extern void     paint_cell(int, int, color_t);
extern void     paint_line(int);
extern void     paint_between_lines(int, int);
extern bool     full_line(int);
extern void     shift_down_lines(int, int);

/* display module ( user threads: tetris ) */
extern void     display_score_board(int, int, int, int);
extern void     display_screen_init(void);
extern void     display_buttons_normal(void);
extern void     display_buttons_paused(void);
extern void     display_fill(int x0, int y0, int x1, int y1, color_t);
extern void     display_end_game(void);
extern void     display_clear_screen(color_t);

/* display module (update_info thread) */
extern void display_info(void);
extern void     display_info_init(void);

/* message priorities */
#define MSG_EXIT                        2
#define MSG_DISPLAY                     1

#define MSG_MAXSIZE                     32
#define MSG_MAXMSG                      4

#endif
