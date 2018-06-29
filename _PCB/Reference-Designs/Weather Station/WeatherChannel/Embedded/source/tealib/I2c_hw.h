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

#ifndef _I2C_HW_H
#define _I2C_HW_H

/*
 * Machine / compiler specific definitions.
 */
 
typedef unsigned char   Uint8;
typedef unsigned short  Uint16;
typedef unsigned long   Uint32;
typedef signed char Int8;
typedef signed short    Int16;
typedef signed long Int32;

/*
 * Connections.
 */
  
#define SDA P1_5
#define SCL P1_4
#define HIGH    1
#define LOW     0

/*
 * Device addresses
 */

#define ADC  0xC8     // Address of A/D converter
#define DAC  0x7A     // Address of D/A convertor

#endif /* _I2C_HW_H */
