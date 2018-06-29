/*
 * i2cm.c --
 *
 *     This file provides routines to communicate with an I2C bus
 *     using the I2CM core. The code assumes the I2CM registers are 
 *     memory-mapped, in this case to SFRs.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */
 
#include "videomux.h"

enum {
    PRESCALE = 99
};

typedef __sfr volatile unsigned char Reg;
 
#define I2CM_CTL   (*(Reg *)0xF8)
#define I2CM_STS   (*(Reg *)0xF9)
#define I2CM_CLK0  (*(Reg *)0xFA)
#define I2CM_CLK1  (*(Reg *)0xFB)
#define I2CM_WDAT  (*(Reg *)0xFC)
#define I2CM_RDAT  (*(Reg *)0xFD)

/* Control register bits. */
enum {
    CTL_ENABLE  = (1 << 0),
    CTL_IEN     = (1 << 1),
    CTL_IACK    = (1 << 2),
    CTL_WRITE   = (1 << 3),
    CTL_READ    = (1 << 4),
    CTL_STOP    = (1 << 5),
    CTL_START   = (1 << 6),
    CTL_ACK     = (1 << 7)
};

/* Status register bits. */
enum {
    STS_INTREQ  = (1 << 0),
    STS_RXACK   = (1 << 1),
    STS_BUSY    = (1 << 2),
    STS_ARBLOST = (1 << 3)
};

#ifdef RIDICULOUS

static const uchar swap[16] = {
    0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
    0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

static void
dwell(void) {
    int   i;
    
    for (i = 0; i < 10000; i++) {
        /* Pause */
    }   
}

static uchar
Flip(uchar n)
{
    return (swap[n & 0xF] << 4) | swap[n >> 4];
}

#endif /* RIDICULOUS */

/* Wait: Poll the status register for an interrupt request. */
static void
Wait(void)
{
    while (!(I2CM_STS & STS_INTREQ)) {
        /* Do nothing. */
    }
    /* Clear the interrupt request. */
    I2CM_CTL = (CTL_ENABLE | CTL_IACK);
}

/* I2cmInit: Set up the prescaler, etc. for the I2CM core. */
void
I2cmInit(void)
{
    I2CM_CTL = 0;
    I2CM_CLK0 = (PRESCALE & 0xFF);
    I2CM_CLK1 = (PRESCALE >> 8);
    I2cStop();
}

/* I2cStop: Direct the I2CM core to issue a stop condition. */
void
I2cStop(void)
{
    I2CM_CTL = (CTL_ENABLE | CTL_IACK | CTL_STOP);
}

/* I2cStart: Issue a start condition and return 0, or a -ve error code. */
int
I2cStart(uchar controlByte)
{
    I2CM_WDAT = controlByte;
    I2CM_CTL = (CTL_ENABLE | CTL_IEN | CTL_START | CTL_WRITE);
    Wait();
    if (I2CM_STS & STS_ARBLOST) {
        return I2C_EBUSY;
    }
    if (I2CM_STS & STS_RXACK) {
        return 0;
    }
    I2cStop();
    return I2C_ENODEV;
}

/* I2cPut: Send a byte and return 1 if it was acknowledged or else 0. */
uchar
I2cPut(uchar value)
{
    I2CM_WDAT = value;
    I2CM_CTL = (CTL_ENABLE | CTL_IEN | CTL_WRITE);
    Wait();
    return (I2CM_STS & STS_RXACK) != 0; 
}

/* I2cGet: Return next byte read from I2C bus, acknowledging it if ack is 1. */
uchar
I2cGet(uchar ack)
{
    if (ack) {
        I2CM_CTL = (CTL_ENABLE | CTL_IEN | CTL_READ | CTL_ACK);
    } else {
        I2CM_CTL = (CTL_ENABLE | CTL_IEN | CTL_READ);   
    }
    Wait();
    return I2CM_RDAT;    
}

