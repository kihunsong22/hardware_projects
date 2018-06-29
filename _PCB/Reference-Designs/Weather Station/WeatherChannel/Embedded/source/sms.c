/*************************************************************************
**
**  VERSION CONTROL:    $Revision:   1.6  $
**          $Date:   Sep 21 2001 11:30:22  $
**
**  IN PACKAGE:     TEA SMS engine TEUCO
**
**  COPYRIGHT:      Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:    SMS module, read available messages and
**          process them...
**
**          TODO: for emit enable,
**          - callback mechanism: what to do with received messages
**          - parse incoming SMS_T and if allowed perform emit-action
**          - reply message to sender
**
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>

#include <time.h>
#include "sms.h"
#include "sms_comm.h"
#include "sms_init.h"
#include "sms_in.h"
#include "sms_out.h"
#include "tealib/timer0.h"


const char charset[128] = {'@' , (char)163 ,       '$' , (char)165 , (char)232 , (char)233 , (char)249 , (char)236 ,
                     (char)242 , (char)199 , (char) 10 , (char)216 , (char)248 , (char) 13 , (char)197 , (char)229 ,
             (char)255 ,       '_' , (char)255 , (char)255 , (char)255 , (char)255 , (char)255 , (char)255 ,
             (char)255 , (char)255 , (char)255 , (char)255 , (char)198 , (char)230 , (char)223 , (char)201 ,
                   ' ' ,       '!' , (char) 34 ,       '#' ,       '$' ,       '%' ,       '&' , (char) 39 ,
                   '(' ,       ')' ,       '*' ,       '+' ,       ',' ,       '-' ,       '.' ,       '/' ,
                   '0' ,       '1' ,       '2' ,       '3' ,       '4' ,       '5' ,       '6' ,       '7' ,
                   '8' ,       '9' ,       ':' ,       ';' ,       '<' ,       '=' ,       '>' ,       '?' ,
             (char)161 ,       'A' ,       'B' ,       'C' ,       'D' ,       'E' ,       'F' ,       'G' ,
                   'H' ,       'I' ,       'J' ,       'K' ,       'L' ,       'M' ,       'N' ,       'O' ,
                   'P' ,       'Q' ,       'R' ,       'S' ,       'T' ,       'U' ,       'V' ,       'W' ,
                   'X' ,       'Y' ,       'Z' , (char)196 , (char)214 , (char)209 , (char)220 , (char)167 ,
             (char)191 ,       'a' ,       'b' ,       'c' ,       'd' ,       'e' ,       'f' ,       'g' ,
                   'h' ,       'i' ,       'j' ,       'k' ,       'l' ,       'm' ,       'n' ,       'o' ,
                   'p' ,       'q' ,       'r' ,       's' ,       't' ,       'u' ,       'v' ,       'w' ,
                   'x' ,       'y' ,       'z' , (char)228 , (char)246 , (char)241 , (char)252 , (char)224 };


void show_message(SMS_T new_sms_message, void * data);
void check_args(int argc, char * argv[]);
void pause_seconds(const unsigned int seconds);

/***********************************************************************************
 * 
 *  FUNCTION:       add_hex
 *
 *  ENVIRONMENT:    buf,        string to store data and add the specified byte to
 *          c,      byte value to add as a hex string to buf
 *          max_len,    max. length of buf
 *
 *  REQUIREMENTS:   buf is valid
 *
 *  RETURN VALUE:   -
 *
 *  DESCRIPTION:    add a byte in hex notation to a char array
 *          
 *
 */
void add_hex(char * buf, char c, int max_len)
{
    const char hex_table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    int len;

    len = strlen(buf);

    /* CHECK IF THERE IS ROOM */
    if ( len < (max_len-1) )
    {
        /* high nibble */
        buf[len] = hex_table[(c >> 4) & 0x0f];
        len++;
        /* low nibble */
        buf[len] = hex_table[c & 0x0f];
        len++;
        buf[len]=0;  /* terminate */
    }
}


/***********************************************************************************
 * 
 *  FUNCTION:       pause_seconds
 *
 *  ENVIRONMENT:    seconds,    number of senconds to sleep
 *
 *  REQUIREMENTS:
 *
 *  RETURN VALUE:   -
 *
 *  DESCRIPTION:    wait for a specified amount of seconds
 *
 */
void pause_seconds(const unsigned int seconds)
{
    tmr0_delay( seconds * TMR0_TICKS_PER_SECOND );
}

