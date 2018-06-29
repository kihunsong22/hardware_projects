#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "graphics.h"
#include "tetris.h"

/* x offset in cell units */
#define X_OFF                   4
/* y offset in cell units */
#define Y_OFF                   5

/* size cell pixels */
#define W_CELL                  10

/* board is (H_BOARD+2)(W_BOARD+2) */
#define B_X0                    X_OFF
#define B_X1                    B_X0 + LASTBOARDCOLUMN+1
#define B_Y0                    Y_OFF
#define B_Y1                    B_Y0 + LASTBOARDROW+1

#define S_OFF_Y                 4
#define S_POS_Y                 B_Y1 + S_OFF_Y
#define S_OFF_X                 2
#define S_POS_X                 B_X0 + S_OFF_X
#define S_N_OFF_X               10
#define S_N_POS_X               10 + S_POS_X

#define H_SCREEN 320
#define W_SCREEN 240

#define THE_TIME                "System time:"

/* linker labels */
extern __no_sdata graphics_bitmap_t _lc_ub_left_bmp;
extern __no_sdata graphics_bitmap_t _lc_ub_right_bmp;
extern __no_sdata graphics_bitmap_t _lc_ub_rotate_bmp;
extern __no_sdata graphics_bitmap_t _lc_ub_down_bmp;
extern __no_sdata graphics_bitmap_t _lc_ub_pause_bmp;

static void     set_cursor_pos(int c, int l);
static void     set_cursor_pos_put_int(int c, int l, int i);
static void     set_cursor_pos_put_char(int c, int l, char ch);
static void     draw_string(int x, int y, const char* str, const font_t* font,
                        color_t fore, color_t back,fontstyle_t fontstyle );

void display_score_board(int po, int li, int pi, int ph)
{
    set_cursor_pos_put_int( (X_OFF+4)*W_CELL + graphics_get_stringwidth(canvas,"POINTS :",NULL,FS_BOLD),
                            (Y_OFF-2)*W_CELL, po);
}

void display_clear_screen(color_t c)
{
        graphics_fill_canvas(canvas,c);
        return;
}

/* Fill square defined by two cells C0 and C1
 * with coordinates (xo,yo) - (x1,y1) with given color */
void display_fill(int x0, int y0, int x1, int y1, color_t color)
{
        static bool first = true;
        int xmin = (x0>x1)?x1:x0;
        int xmax = (x0>x1)?x0:x1;
        int ymin = (y0>y1)?y1:y0;
        int ymax = (y0>y1)?y0:y1;

        /* init screen in first 'display_fill' */
        if (first) {
                display_screen_init();
                first = false;
        }
        graphics_fill_rect( canvas,
                            (B_X0 + xmin ) * W_CELL,(B_Y1+1 - ymax) * W_CELL,
                            (xmax-xmin)*W_CELL, (ymax-ymin)*W_CELL, color);
        return;
}

#define BUTTON_SIZE_X 16
#define BUTTON_SIZE_Y 16
#define BUTTON_YPOS H_SCREEN-BUTTON_SIZE_Y
#define BUTTON1_XPOS   6
#define BUTTON2_XPOS  59
#define BUTTON3_XPOS 112
#define BUTTON4_XPOS 165
#define BUTTON5_XPOS 218

void display_buttons_normal(void)
{
       graphics_fill_rect (canvas, 0, BUTTON_YPOS, W_SCREEN, BUTTON_YPOS, BLACK);
       graphics_draw_bitmap(canvas, &_lc_ub_left_bmp,   BUTTON1_XPOS, BUTTON_YPOS, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0);
       graphics_draw_bitmap(canvas, &_lc_ub_right_bmp,  BUTTON2_XPOS, BUTTON_YPOS, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0);
       graphics_draw_bitmap(canvas, &_lc_ub_rotate_bmp, BUTTON3_XPOS, BUTTON_YPOS, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0);
       graphics_draw_bitmap(canvas, &_lc_ub_down_bmp,   BUTTON4_XPOS, BUTTON_YPOS, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0);
       graphics_draw_bitmap(canvas, &_lc_ub_pause_bmp,  BUTTON5_XPOS, BUTTON_YPOS, BUTTON_SIZE_X, BUTTON_SIZE_Y, 0);
}

void draw_centered_string(void)
{
}

void display_buttons_paused(void)
{
       int string_width;
       string_width = graphics_get_stringwidth(canvas,"PAUSED",NULL,FS_BOLD);
       graphics_fill_rect (canvas, 0, BUTTON_YPOS, W_SCREEN, BUTTON_YPOS, BLACK);
       graphics_draw_string(canvas,(W_SCREEN-string_width) / 2,BUTTON_YPOS,"PAUSED",NULL,RED,FS_BOLD);
}

void display_screen_init(void)
{
        display_clear_screen(BLACK);
        display_buttons_normal();
        draw_string(X_OFF*W_CELL,(Y_OFF-2)*W_CELL,"POINTS :",NULL,WHITE,BLACK,FS_BOLD);
        return;
}


static void set_cursor_pos_put_int(int c, int l, int i)
{
        char buf[48];
        sprintf(buf, "%d", i);
        draw_string(c,l,buf,NULL,WHITE,BLACK,FS_BOLD);
        return;
}

void display_info(void)
{
       struct timespec ts;
       char buf[48];
       clock_gettime(CLOCK_REALTIME, &ts);
       sprintf(buf, "%02d:%02d:%02d", (int)(ts.tv_sec/3600),
            (int)((ts.tv_sec/60)%60), (int)(ts.tv_sec%60));
       draw_string(X_OFF*W_CELL,(Y_OFF-4)*W_CELL,buf,NULL,WHITE,BLACK,FS_BOLD);
}

void display_end_game(void)
{
       int string_width;
       string_width = graphics_get_stringwidth(canvas,"GAME OVER",NULL,FS_BOLD);

       graphics_fill_rect  (canvas, (X_OFF+1)*W_CELL, (Y_OFF+3)*W_CELL, W_BOARD*W_CELL, 3*W_CELL, BLACK);
       graphics_draw_string(canvas,(W_SCREEN-string_width) / 2,(Y_OFF+4)*W_CELL,"GAME OVER",NULL,RED,FS_BOLD);
}

static void draw_string(int x, int y, const char* str, const font_t* font,
                        color_t fore, color_t back,fontstyle_t fontstyle )
{
       graphics_fill_rect  (canvas,x,y,
                            graphics_get_stringwidth(canvas,str,font,fontstyle),
                            graphics_get_fontheight(canvas,font),
                            back);
       graphics_draw_string(canvas,x,y,str,font,fore,fontstyle);
}
