#include "..\tealib\vga.h"

/* Arial 10 character set */
#include "arial8_0.c"
#include "arial8_1.c"
#include "arial8_2.c"
#include "arial8_3.c"
#include "arial8_4.c"
#include "arial8_5.c"
#include "arial8_6.c"
#include "arial8_7.c"
#include "arial8_8.c"
#include "arial8_9.c"
#include "arial8_dot.c"
#include "arial8_colon.c"
#include "arial8_minus.c"
/* lower case */
#include "arial8_d.c"
#include "arial8_h.c"
#include "arial8_i.c"
#include "arial8_m.c"
#include "arial8_n.c"
#include "arial8_s.c"
/* upper case */
#include "arial8_EE.c"
#include "arial8_NN.c"
#include "arial8_SS.c"
#include "arial8_WW.c"


unsigned char __rom * __rom arial8_charset[] =
    {/* 0 */ arial8_0,
     /* 1 */ arial8_1,
     /* 2 */ arial8_2,
     /* 3 */ arial8_3,
     /* 4 */ arial8_4,
     /* 5 */ arial8_5,
     /* 6 */ arial8_6,
     /* 7 */ arial8_7,
     /* 8 */ arial8_8,
     /* 9 */ arial8_9,
     /* . */ arial8_dot,
     /* - */ arial8_minus,
     /* : */ arial8_colon,
     /* E */ arial8_E,
     /* N */ arial8_N,
     /* S */ arial8_S,
     /* W */ arial8_W,
     /* d */ arial8_d,
     /* h */ arial8_h,
     /* i */ arial8_i,
     /* m */ arial8_m,
     /* n */ arial8_n,
     /* s */ arial8_s };


__rom unsigned char *arial8( const char c )
{
    __rom unsigned char *ret_val = 0;
    
    if ( c >= '0' && c <= '9' )
    {
        ret_val = arial8_charset[c-'0'];
    }
    else if ( c == '.' )
    {
        ret_val = arial8_charset[10];
    }
    else if ( c == '-' )
    {
        ret_val = arial8_charset[11];
    }
    else if ( c == ':' )
    {
        ret_val = arial8_charset[12];
    }
    else if ( c == 'E' )
    {
        ret_val = arial8_charset[13];
    }
    else if ( c == 'N' )
    {
        ret_val = arial8_charset[14];
    }
    else if ( c == 'S' )
    {
        ret_val = arial8_charset[15];
    }
    else if ( c == 'W' )
    {
        ret_val = arial8_charset[16];
    }
    else if ( c == 'd' )
    {
        ret_val = arial8_charset[17];
    }
    else if ( c == 'h' )
    {
        ret_val = arial8_charset[18];
    }
    else if ( c == 'i' )
    {
        ret_val = arial8_charset[19];
    }
    else if ( c == 'm' )
    {
        ret_val = arial8_charset[20];
    }
    else if ( c == 'n' )
    {
        ret_val = arial8_charset[21];
    }
    else if ( c == 's' )
    {
        ret_val = arial8_charset[22];
    }

    return ret_val;
}



