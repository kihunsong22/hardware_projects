/*****************************************************************************
 *      FILE:          uart0.h
 *
 *      CREATED:       Mon Aug 11 11:34:33 2003
 *
 *      IN PACKAGE:    8051 peripheral library
 *
 *      TARGET:        FPGA R80515
 *
 *      COPYRIGHT:     Copyright 2001 - 2003 Altium BV
 *
 *      DESCRIPTION:   Configuration file for Uart 0
 *
 ****************************************************************************/

#ifndef _UART0_H
#define _UART0_H

/* Include system defines */
#include "tealib_cfg.h"


/* Include special types headerfile */
#include "uart_types.h"


/* Undefine all */
#undef     UART_CHANNEL
#undef     UART_TRANSMITTER_ON
#undef     UART_RECEIVER_ON
#undef     UART_IGN_ERRBYTE
#undef     UART_IGN_BUFOVR
#undef     UART_PARMS
#undef     UART_BAUDRATE
#undef     UART_TBUF
#undef     UART_RBUF
#undef     UART_BUF_MEMSPACE
#undef     UART_ISR_PRIORITY
#undef     UART_ISR_REGBANK


/* Configuration */
#define    UART_CHANNEL         0
#define    UART_TRANSMITTER_ON
#define    UART_RECEIVER_ON
#define    UART_IGN_ERRBYTE
//#define  UART_IGN_BUFOVR
#define    UART_PARMS           UART_N81
#define    UART_BAUDRATE        115200U
#define    UART_TBUF            1600
#define    UART_RBUF            1700
#define    UART_RBUF_RTS        100
#define    UART_BUF_MEMSPACE        __xdata
#define    UART_ISR_PRIORITY        0
#define    UART_ISR_REGBANK     1


/* Include the peripheral common source headerfile */
#include "uart.h"

#endif
