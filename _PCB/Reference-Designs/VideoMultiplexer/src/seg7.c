/*
 * seg7.c --
 *
 *     This file provides routines to display numbers on a 2-digit,
 *     seven segment display. The code assumes that each 7-segment
 *     display is wired to an 8051 output port.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include "videomux.h"

enum {
    DECIMAL_POINT   = 0x80,
    MINUS_SIGN      = 0x40
};

/* Mapping from a 1-digit, hex value to segments on the display. */
static const char hexBits[16] = {
    0x3F, 0x06, 0x5B, 0x4F, /* 0, 1, 2, 3 */ 
    0x66, 0x6D, 0x7D, 0x07, /* 4, 5, 6, 7 */
    0x7F, 0x67, 0x77, 0x7C, /* 8, 9, A, B */
    0x39, 0x5E, 0x79, 0x71  /* C, D, E, F */
};   

/* ShowDigit: display n in hex on the specified port */ 
void
ShowDigit(Port *p, int n)
{
    if ((n < 0) || (n > 15)) {
        /* Report out-of-range error with a '.'. */
        *p = DECIMAL_POINT;
    } else {
        *p = hexBits[n];
    }
}

/* ShowInt: display n as a two-digit hex number on the specified ports */
void
ShowInt(Port *highDigit, Port *lowDigit, int n)
{
    if ((n < -15) || (n > 255)) {
        /* Report out-of-range error with "..". */
        *highDigit = DECIMAL_POINT;
        *lowDigit = DECIMAL_POINT;
    } else if (n >= 0) {
        ShowDigit(lowDigit, (n & 0xF));
        ShowDigit(highDigit, (n >> 4));
    } else {
        ShowDigit(lowDigit, ((-n) & 0xF));
        *highDigit = MINUS_SIGN;
    }
}
