/*****************************************************************************
 *      FILE:          lcd0.h
 *
 *      CREATED:       Thu Aug 14 15:19:31 2003
 *
 *      IN PACKAGE:    8051 peripheral library
 *
 *      TARGET:        P89C66x
 *
 *      COPYRIGHT:     Copyright 2001 - 2003 Altium BV
 *
 *      DESCRIPTION:   Configuration file for Alphanumeric LCD
 *
 ****************************************************************************/

#ifndef _LCD0_H
#define _LCD0_H

/* Include system defines */
#include "tealib_cfg.h"


/* Undefine all */
#undef     ALPHALCD_WIDTH
#undef     ALPHALCD_HEIGHT
#undef     ALPHALCD_CONFIG_1X16_AS_2_LINE
#undef     ALPHALCD_VFD_MODULE
#undef     ALPHALCD_HANDLE_NEWLINE
#undef     ALPHALCD_GOTOXY_CLIP
#undef     ALPHALCD_4BITBUS
#undef     ALPHALCD_BASEADDR
#undef     ALPHALCD_BITBANG
#undef     ALPHALCD_BITBANG_DB
#undef     ALPHALCD_BITBANG_E
#undef     ALPHALCD_BITBANG_RS
#undef     ALPHALCD_BITBANG_RW
#undef     ALPHALCD_USERCHAR_0
#undef     ALPHALCD_USERCHAR_1
#undef     ALPHALCD_USERCHAR_2
#undef     ALPHALCD_USERCHAR_3
#undef     ALPHALCD_USERCHAR_4
#undef     ALPHALCD_USERCHAR_5
#undef     ALPHALCD_USERCHAR_6
#undef     ALPHALCD_USERCHAR_7


/* Configuration */
#define    ALPHALCD_WIDTH       16
#define    ALPHALCD_HEIGHT      2
//#define  ALPHALCD_CONFIG_1X16_AS_2_LINE
//#define  ALPHALCD_VFD_MODULE
#define    ALPHALCD_HANDLE_NEWLINE
//#define  ALPHALCD_GOTOXY_CLIP
//#define  ALPHALCD_4BITBUS
#define    ALPHALCD_BASEADDR        0x200
//#define    ALPHALCD_BITBANG
//#define    ALPHALCD_BITBANG_DB      P0
//#define    ALPHALCD_BITBANG_E       P1_3
//#define    ALPHALCD_BITBANG_RS      P1_4
//#define    ALPHALCD_BITBANG_RW      P1_5
#define    ALPHALCD_USERCHAR_0      0x18, 0x18, 0x03, 0x04, 0x04, 0x04, 0x03, 0x00
#define    ALPHALCD_USERCHAR_1      0x18, 0x18, 0x07, 0x04, 0x06, 0x04, 0x04, 0x00
#define    ALPHALCD_USERCHAR_2      0x06, 0x04, 0x06, 0x04, 0x06, 0x04, 0x0E, 0x0E
#define    ALPHALCD_USERCHAR_3      0x01, 0x03, 0x1F, 0x08, 0x08, 0x08, 0x08, 0x08
//#define  ALPHALCD_USERCHAR_4  
//#define  ALPHALCD_USERCHAR_5  
//#define  ALPHALCD_USERCHAR_6  
//#define  ALPHALCD_USERCHAR_7  


/* Include the peripheral common source headerfile */
#include "lcd.h"

#endif
