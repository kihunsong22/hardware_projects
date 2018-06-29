/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Revision: 1 $
|*          $Date: 3/22/00 2:01p $
|*
|*  IN PACKAGE:     TASKING I2C application note
|*
|*  COPYRIGHT:      Copyright (c) 2000 TASKING, Inc.
|*
|*  DESCRIPTION:    Provide definitions and function prototypes for the I2C interface
|*
\*****************************************************************************/

#ifndef _I2CPLAT_H
#define _I2CPLAT_H

#include "i2c_hw.h"

/*
 * Return values:
 *  I2C_OK  Everything's fine
 *  I2C_NAK Did not get an acknowledge
 *  I2C_TOUT    Acknowledge timed out
 *  I2C_ARB Bus arbitration lost
 */

typedef enum { I2C_OK, I2C_NAK, I2C_TOUT, I2C_ARB } I2C_ERR;

/*
 * Function prototypes
 */

extern I2C_ERR I2C_init( void );
extern I2C_ERR I2C_read( Uint8 slave, Uint8 *buf, Uint16 size );
extern I2C_ERR I2C_write( Uint8 slave, const Uint8 *buf, Uint16 size );

#endif /* _I2CPLAT_H */
