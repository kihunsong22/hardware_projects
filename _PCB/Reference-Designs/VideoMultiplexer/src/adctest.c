/*
 * main.c --
 *
 *     This file is the main program for the Video Multiplexer.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include "videomux.h"

enum {
    ADC_ADDR = 0xC8
};

int
AdcInit(void)
{
    uchar   config[2];
    
    config[0] = 0x80;   /* Unipolar, internal clock, reference is Vdd. */
    config[1] = 0x61;   /* Scan channel 0 only, single-ended. */
    return I2cWrite(ADC_ADDR, (uchar *)0, 0, config, sizeof(config));
}

int
AdcGet(void)
{
    int     n;
    uchar   v;
    
    n = I2cRead(ADC_ADDR, (uchar *)0, 0, &v, 1);
    if (n < 1) {
        return n;
    }
    return (int)v;
}

void
delay()
{
    int     i;
    
    for (i = 0; i < 0xAFFF; i++) {
        __asm("nop");
    }
}

int
main()
{
    static __xdata uchar  mem[256] __at(0xAA00);
    int   i;

    I2cInit();
#if 0
    /* Test dual port ram. */
    for (;;) {
       for (i = 0; i < 256; i++) {
          mem[i] = 255 - i;
       }
       for (i = 0; i < 256; i++) {
          i = mem[i];
#if 0
          ShowInt(&P1, &P0, i);
          delay();
#endif
       }
    }
#else
    /* Read voltage from ADC and display on 7 seg display in USER IO1. */
    AdcInit(); 
    for (;;) {
#if 1
        ShowInt(&P1, &P0, 300);
        delay();
#endif
        i = AdcGet();
        ShowInt(&P1, &P0, i);
        delay();
    }
#endif
}
