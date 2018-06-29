/*
 * i2c.h --
 *
 *     This header file holds declarations of routines and
 *     data types used in the I2C module.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#ifndef _I2C_H
#define _I2C_H

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;

enum {
    I2C_EBUSY   = -1, /* Bus busy (master has lost arbitration). */
    I2C_ENODEV  = -2, /* Device address was not acknowledged. */
    I2C_ENOACK  = -3, /* Subaddress bytes were not acknowledged. */
    I2C_ENODATA = -4  /* No data was returned by read from slave. */
};

extern void	I2cInit(void);
extern int	I2cRead(uchar dev, uchar addr[], int addrlen,
			uchar buf[], int count);
extern int	I2cWrite(uchar dev, uchar addr[], int addrlen,
			uchar buf[], int count);
extern int	I2cPeek(uchar dev, uchar subaddr);
extern int	I2cPoke(uchar dev, uchar subaddr, uchar value);

/* Low level, implementation-dependent routines. */
extern void	I2cStop(void);
extern int	I2cStart(uchar controlByte);
extern uchar	I2cPut(uchar value);
extern uchar	I2cGet(uchar ack);

#endif /* _I2C_H */

