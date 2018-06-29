/*
 * i2c.c --
 *
 *     This file provides routines to communicate with an I2C bus.
 *     At present, operation is only supported as an I2C master.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include "i2c.h"

/*
 * I2cRead: Read "count" bytes into "buf" from the slave at address "dev".
 * Start at the multi-byte subaddress given by the contents of array "addr".
 * Return the number of bytes read, or a negative error code.
 */

int
I2cRead(uchar dev, uchar addr[], int addrlen, uchar buf[], int count)
{
    int      i;

    if (addrlen > 0) {
        /* Use a write sequence to send the subaddress bytes. */
        i = I2cStart(dev);
        if (i < 0) {
            return i;
        }
        for (i = 0; i < addrlen; i++) {
            if (!I2cPut(addr[i])) {
                I2cStop();
                return I2C_ENOACK;
            }
        }
    }

    /* Issue a read. (It's a repeated start if there was a subaddress.) */
    i = I2cStart(dev | 1);
    if (i < 0) {
        return i;
    }

    /* Acknowledge all but the last byte during a multi-byte read. */
    if (count > 0) {
        for (i = 0; i < (count - 1); i++) {
            buf[i] = I2cGet(1);
        }
        buf[i] = I2cGet(0);
    } else {
        count = 0;
    }
    I2cStop();
    return count;
}

/*
 * I2cWrite: Write "count" bytes from "buf" to the slave at address "dev".
 * Start at the multi-byte subaddress given by the contents of array "addr".
 * Return the number of bytes written, or a negative error code.
 */

int
I2cWrite(uchar dev, uchar addr[], int addrlen, uchar buf[], int count)
{
    int      i;

    i = I2cStart(dev);
    if (i < 0) {
        return i;
    }
    for (i = 0; i < addrlen; i++) {
        if (!I2cPut(addr[i])) {
            I2cStop();
            return I2C_ENOACK;
        }
    }
    for (i = 0; i < count; i++) {
        if (!I2cPut(buf[i])) {
            break;
        }
    }
    I2cStop();
    return i;
}

/*
 * I2cPeek: Read a byte from the specified subaddress of an I2C device
 * and return the byte's value (0..255), or a negative error code.
 */

int
I2cPeek(uchar dev, uchar subaddr)
{
    int      result;
    uchar    value;

    result = I2cRead(dev, &subaddr, 1, &value, 1);
    if (result == 1) {
        result = (int)value;
    } else if (result == 0) {
        result = I2C_ENODATA;
    }
    return result;
}

/*
 * I2cPoke: Write a byte value to the specified subaddress of an I2C device
 * and return the number of bytes written (0 or 1), or a negative error code.
 */

int
I2cPoke(uchar dev, uchar subaddr, uchar value)
{
    return I2cWrite(dev, &subaddr, 1, &value, 1);
}

