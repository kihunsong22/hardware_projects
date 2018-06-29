#include "tetris.h"

/* internal board representation */
static color_t board[H_BOARD+2][W_BOARD+2];
static void copy_line(int from, int to);

void board_init(color_t bkg, color_t fr)
{
        int i, j;
        display_clear_screen(bkg);
    
        /*
         * tetris area is width 14 * heigth 21 frame starts at (0,0) inside area
         * for tetris starts at (1,1)
         */
        display_fill(0, 0, 1 + W_BOARD, 1, fr);
        display_fill(1 + W_BOARD, 0, 1 + W_BOARD + 1, 1 + H_BOARD, fr);
        display_fill(1, 1 + H_BOARD, 1 + W_BOARD + 1, 1 + H_BOARD + 1, fr);
        display_fill(0, 1, 1, 1 + H_BOARD + 1, fr);

        /* Initialize the whole board to free */
        for (i=FIRSTBOARDROW; i<=LASTBOARDROW; i++)
        {
                for (j=FIRSTBOARDCOLUMN; j<=LASTBOARDCOLUMN; j++)
                {
                        board[i][j] = bkg;
                }
        }
        /* Initialize the frame to non-free */
        for (i=FIRSTBOARDROW-1; i<=LASTBOARDROW+1; i++)
        {
                board[i][FIRSTBOARDCOLUMN-1] = fr;
                board[i][LASTBOARDCOLUMN+1] = fr;
        }
        for (i=FIRSTBOARDCOLUMN-1; i<=LASTBOARDCOLUMN+1; i++)
        {
                board[FIRSTBOARDROW-1][i] = fr;
                board[LASTBOARDROW+1][i] = fr;
        }

        return;
}


void paint_cell(int row, int col, color_t color)
{

        if (    row>=FIRSTBOARDROW && row <=LASTBOARDROW  &&
                col>=FIRSTBOARDCOLUMN && col <=LASTBOARDCOLUMN )
        {
                board[row][col] = color;
                display_fill(col, row, col+1, row+1, color);
        }
        return;
}

bool free_cell(int row, int col)
{
        return (board[row][col]==(color_t)BLACK);
}


void paint_line(int row)
{
        int col;
        color_t color;
        if (row>=FIRSTBOARDROW && row <=LASTBOARDROW)
        {
                for (col=FIRSTBOARDCOLUMN; col<=LASTBOARDCOLUMN;col++)
                {
                        color = board[row][col];
                        paint_cell(row, col, color);
                }
        }
}


void paint_between_lines(int from, int to)
{
        int l;
        for (l=from; l<=to; l++)
        {
               paint_line(l);
        }
}

 /* returns true if line is full with Tetris pieces */
bool full_line(int row)
{
        int col;
        for (col=FIRSTBOARDCOLUMN; col<=LASTBOARDCOLUMN; col++)
        {
                if (free_cell(row, col))
                {
                        return false;
                }
        }
        return true;
}

/* Deletes a full Tetris line */
void shift_down_lines(int down, int top)
{
        int l;
        for (l=down; l<=top; l++)
        {
                copy_line(l+1,l);
        }

    return;
}


/* Copies a full Tetris line 'from' in the
 * position of line 'to' */
static void copy_line(int from, int to)
{
        int col;
        for (col=FIRSTBOARDCOLUMN; col<=LASTBOARDCOLUMN; col++)
        {
             board[to][col] = board[from][col];
        }

        return;
}

