#include "..\tealib\vga.h"

/* Arial 10 character set */
#include "arial10_0.c"
#include "arial10_1.c"
#include "arial10_2.c"
#include "arial10_3.c"
#include "arial10_4.c"
#include "arial10_5.c"
#include "arial10_6.c"
#include "arial10_7.c"
#include "arial10_8.c"
#include "arial10_9.c"
#include "arial10_colon.c"
#include "arial10_minus.c"
/* lower case */
#include "arial10_h.c"
#include "arial10_m.c"
#include "arial10_s.c"
/* upper case */
#include "arial10_NN.c"
#include "arial10_EE.c"
#include "arial10_SS.c"
#include "arial10_WW.c"


unsigned char __rom * __rom arial10_charset[] =
    {/* 0 */ arial10_0,
     /* 1 */ arial10_1,
     /* 2 */ arial10_2,
     /* 3 */ arial10_3,
     /* 4 */ arial10_4,
     /* 5 */ arial10_5,
     /* 6 */ arial10_6,
     /* 7 */ arial10_7,
     /* 8 */ arial10_8,
     /* 9 */ arial10_9,
     /* - */ arial10_minus,
     /* : */ arial10_colon,
     /* N */ arial10_N,
     /* E */ arial10_E,
     /* S */ arial10_S,
     /* W */ arial10_W,
     /* h */ arial10_h,
     /* m */ arial10_m,
     /* s */ arial10_s };


__rom unsigned char *arial10( const char c )
{
    __rom unsigned char *ret_val = 0;
    
    if ( c >= '0' && c <= '9' )
    {
        ret_val = arial10_charset[c-'0'];
    }
    else if ( c == '-' )
    {
        ret_val = arial10_charset[10];
    }
    else if ( c == ':' )
    {
        ret_val = arial10_charset[11];
    }
    else if ( c == 'N' )
    {
        ret_val = arial10_charset[12];
    }
    else if ( c == 'E' )
    {
        ret_val = arial10_charset[13];
    }
    else if ( c == 'S' )
    {
        ret_val = arial10_charset[14];
    }
    else if ( c == 'W' )
    {
        ret_val = arial10_charset[15];
    }
    else if ( c == 'h' )
    {
        ret_val = arial10_charset[16];
    }
    else if ( c == 'm' )
    {
        ret_val = arial10_charset[17];
    }
    else if ( c == 's' )
    {
        ret_val = arial10_charset[18];
    }

    return ret_val;
}



