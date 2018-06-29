#ifndef SCREEN_H
#define SCREEN_H

/* enums */

enum { fill, line };

#define SPLASH_DELAY1               100
#define SPLASH_DELAY2               150

/* refresh interval for history and value ( in tmr0_ticks ) */
#define SCR_HIST_REFRESH_INTERVAL   100UL
#define SCR_VAL_REFRESH_INTERVAL    100

/* x-pos of logo */
#define L_POS_X2                    630

/* history-, position- and color-settings for temp, speed and direction */
#define HIST_ARRAYSIZE              432

#define T_HIST_POS_X                20
#define T_HIST_POS_Y                104
#define T_HIST_MULT                 2
#define T_HIST_DIV                  1
#define T_VAL_POS_X2                610
#define T_VAL_POS_Y                 105
#define T_LBL_POS_Y                 80
#define T_BG_COLOR                  grey
#define T_FG1_COLOR                 red
#define T_FG2_COLOR                 darkred
#define T_LINE_COLOR                darkgrey

#define S_HIST_POS_X                20
#define S_HIST_POS_Y                274
#define S_HIST_MULT                 2
#define S_HIST_DIV                  10
#define S_VAL_POS_X2                610
#define S_VAL_POS_Y                 235
#define S_LBL_POS_Y                 210
#define S_BG_COLOR                  grey
#define S_FG1_COLOR                 green
#define S_FG2_COLOR                 darkgreen
#define S_LINE_COLOR                darkgrey

#define D_HIST_POS_X                20
#define D_HIST_POS_Y                384
#define D_HIST_MULT                 2
#define D_HIST_DIV                  9
#define D_VAL_POS_X2                610
#define D_VAL_POS_Y                 345
#define D_LBL_POS_Y                 320
#define D_BG_COLOR                  grey
#define D_FG1_COLOR                 cyan
#define D_FG2_COLOR                 darkcyan
#define D_LINE_COLOR                darkgrey


void screen_init( void );
void screen_update( void );
void screen_temp_hist_init( void );
void screen_speed_hist_init( void );
void screen_dir_hist_init( void );


extern unsigned char chart_type;

#endif
