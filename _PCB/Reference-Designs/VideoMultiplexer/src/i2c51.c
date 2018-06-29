/*
 * i2c51.c --
 *
 *     This file provides low level routines to communicate with an I2C bus
 *     using I/O port pins on an 8051 core.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */
 
#include "i2c.h"

#define SCL_PIN     P2_6
#define SDA_PIN     P2_7

#define CLEAR_SCL() (SCL_PIN = 0)
#define SET_SCL()   (SCL_PIN = 1)
#define TEST_SCL()  (SCL_PIN != 0)

#define CLEAR_SDA() (SDA_PIN = 0)
#define SET_SDA()   (SDA_PIN = 1)
#define TEST_SDA()  (SDA_PIN != 0)

/* DelayQuarter: Pause for one quarter of an I2C clock period (about 3us). */
static void
DelayQuarter(void)
{
    /* The following is about right for an 8051 running at 40MHz. */
    __asm(".REPEAT 6");
    __asm("nop");
    __asm(".ENDREP");
}

/* AssertClock: Cause a rising edge on SCL. */
static void
AssertClock(void)
{
    DelayQuarter();
    SET_SCL();
    while (!TEST_SCL()) {
       /* Wait if a slave is stretching the clock (holding it low). */
    }
    DelayQuarter();
}

/* I2cInit: Configure the I2C port pins for operation. */
void
I2cInit(void)
{
    /* Clear all slaves off the bus. */
    I2cStop();
    I2cStop();
    I2cStop();
}

/* I2cStop: Force a clock transition with SDA low. */
void
I2cStop(void)
{
    DelayQuarter();
    CLEAR_SDA();
    AssertClock();
    SET_SDA();
    DelayQuarter();
}

/* I2cStart: Issue a start condition and return 0, or a -ve error code. */
int
I2cStart(uchar controlByte)
{
    SET_SDA();
    SET_SCL();
    while (!TEST_SCL()) {
       /*
        * Wait for SCL to go high. It may be stretched low by
        * the slave prior to a repeated start condition.
        */
    }
    DelayQuarter();
    DelayQuarter();
    if (!(TEST_SCL() && TEST_SDA())) {
       return I2C_EBUSY;
    }
    CLEAR_SDA();
    DelayQuarter();
    DelayQuarter();
    CLEAR_SCL();
    return I2cPut(controlByte) ? 0 : I2C_ENODEV;
}

/* I2cPut: Send a byte and return 1 if it was acknowledged or else 0. */
uchar
I2cPut(uchar value)
{
    uchar   mask;

    for (mask = 0x80; mask; mask >>= 1) {
       DelayQuarter();
       if (value & mask) {
          SET_SDA();
       } else {
          CLEAR_SDA();
       }
       AssertClock();
       DelayQuarter();
       CLEAR_SCL();
    }

    /* Read the acknowledge bit. */
    DelayQuarter();
    SET_SDA();
    AssertClock();
    mask = TEST_SDA();
    DelayQuarter();
    CLEAR_SCL();
    return mask ? 0 : 1;
}

/* I2cGet: Return next byte read from I2C bus, acknowledging it if ack is 1. */
uchar
I2cGet(uchar ack)
{
    uchar   i, n;

    for (n = 0, i = 0; i < 8; i++) {
       n <<= 1;
       DelayQuarter();
       SET_SDA();
       AssertClock();
       if (TEST_SDA()) {
          n++;
       }
       DelayQuarter();
       CLEAR_SCL();
    }

    /* Send the acknowledge bit. */
    DelayQuarter();
    if (ack) {
       CLEAR_SDA();
    } else {
       SET_SDA();
    }
    AssertClock();
    DelayQuarter();
    CLEAR_SCL();
    return n;
}

