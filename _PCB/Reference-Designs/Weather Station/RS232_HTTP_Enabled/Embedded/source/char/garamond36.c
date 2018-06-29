#include "..\tealib\vga.h"

/* garamond 36 character set */
#include "garamond36_0.c"
#include "garamond36_1.c"
#include "garamond36_2.c"
#include "garamond36_3.c"
#include "garamond36_4.c"
#include "garamond36_5.c"
#include "garamond36_6.c"
#include "garamond36_7.c"
#include "garamond36_8.c"
#include "garamond36_9.c"
#include "garamond36_minus.c"
#include "garamond36_dot.c"
/* upper case */
#include "garamond36_NN.c"
#include "garamond36_EE.c"
#include "garamond36_SS.c"
#include "garamond36_WW.c"
     
unsigned char __rom * __rom garamond36_charset[] =
    {/* 0 */ garamond36_0,
     /* 1 */ garamond36_1,
     /* 2 */ garamond36_2,
     /* 3 */ garamond36_3,
     /* 4 */ garamond36_4,
     /* 5 */ garamond36_5,
     /* 6 */ garamond36_6,
     /* 7 */ garamond36_7,
     /* 8 */ garamond36_8,
     /* 9 */ garamond36_9,
     /* - */ garamond36_minus,
     /* . */ garamond36_dot,
     /* N */ garamond36_N,
     /* E */ garamond36_E,
     /* S */ garamond36_S,
     /* W */ garamond36_W };


__rom unsigned char *garamond36( const char c )
{
    __rom unsigned char *ret_val = 0;
    
    if ( c >= '0' && c <= '9' )
    {
        ret_val = garamond36_charset[c-'0'];
    }
    else if ( c == '-' )
    {
        ret_val = garamond36_charset[10];
    }
    else if ( c == '.' )
    {
        ret_val = garamond36_charset[11];
    }
    else if ( c == 'N' )
    {
        ret_val = garamond36_charset[12];
    }
    else if ( c == 'E' )
    {
        ret_val = garamond36_charset[13];
    }
    else if ( c == 'S' )
    {
        ret_val = garamond36_charset[14];
    }
    else if ( c == 'W' )
    {
        ret_val = garamond36_charset[15];
    }

    return ret_val;
}



