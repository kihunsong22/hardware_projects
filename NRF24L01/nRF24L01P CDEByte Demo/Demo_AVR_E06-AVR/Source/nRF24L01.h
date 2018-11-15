/*
================================================================================
Copyright(c)  Yihe Embedded Studio, All rights reserved.
              www.yh-mcu.com
              http://yh-mcu.taobao.com

File Name   : nRF24L01.c
Description : nRF24L01 low level operations and configurations.
Author      : ShawZhang,LiYong
Date        : 2012-11-07 
Version     : 2.0
Compiler    : Any
Note        : None
================================================================================
*/
#ifndef _nRF24L01_H_
#define _nRF24L01_H_

#include "MyTypedef.h"
#include "nRF24L01_Reg.h"
/*
================================================================================
============================Configurations and Options==========================
================================================================================
*/
#define DYNAMIC_PACKET      1 //1:DYNAMIC packet length, 0:fixed
#define FIXED_PACKET_LEN    32//Packet size in fixed size mode
#define REPEAT_CNT          15//0-15, repeat transmit count
#define REPEAT_TIME			2000//250, 500, 750......4000
#define INIT_ADDR           0X99, 0X88, 0X21, 0X36, 0XAB

/*
================================================================================
======================External IMPORT function list=============================
You MUST offers the following functions or macros for this c file:nRF24L01.c
1.L01_CSN_LOW( );                       //Pull low the CS line of SPI
2.L01_CSN_HIGH( );                      //Pull high the CS line if SPI
3.INT8U SPI_ExchangeByte( INT8U value );//SPI Exchange a byte
4.L01_CE_LOW( );
5.L01_CE_HIGH( );
================================================================================
*/
#ifdef _nRF24L01_C_

#include "board.h"


#endif//_nRF24L01_C_
/*
================================================================================
=========================Local EXPORT function list=============================
======================Do NOT modify the following items=========================
================================================================================
*/
/*Read a single register of nRF24L01*/
INT8U L01_ReadSingleReg( INT8U Addr );

/*Read several registers of nRF24L01*/
void L01_ReadMultiReg( INT8U StartAddr, INT8U nBytes, INT8U *pBuff );

/*Write a single byte to a register*/
void L01_WriteSingleReg( INT8U Addr, INT8U Value );

/*Read several registers of nRF24L01*/
void L01_WriteMultiReg( INT8U StartAddr, INT8U *pBuff, INT8U Length );

/*Flush the TX buffer*/
void L01_FlushTX( void );

/*Flush the RX buffer*/
void L01_FlushRX( void );

/*reuse payload in tx fifo*/
void L01_ReuseTXPayload( void );

/* nop operation of nRF24L01 */
void L01_Nop( void );

/*Reuse the last transmitted payload*/
void L01_ReuseTXPayload( void );

/* read statu register of nRF24l01*/
INT8U L01_ReadStatusReg( void );

/*L01 clear IRQ*/
#define IRQ_ALL  ( (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT) )
void L01_ClearIRQ( INT8U IRQ_Source );

/*Read the IRQ source of nRF24L01+*/
INT8U L01_ReadIRQSource( void );

/*Read the payload width of the top buffer of FIFO*/
INT8U L01_ReadTopFIFOWidth( void );

/*Read the RX payload from internal buffer*/
INT8U L01_ReadRXPayload( INT8U *pBuff );

/*Write TX payload to a pipe and prx will return ack back*/
void L01_WriteTXPayload_Ack( INT8U *pBuff, INT8U nBytes );

/*write data in tx mode, and prx won't return ack back*/
void L01_WriteTXPayload_NoAck( INT8U *Data, INT8U Data_Length );

/*write data in tx fifo when rx mode*/
void L01_WriteRXPayload_InAck( INT8U *pData, INT8U Data_Length );

/*Write address for the own device*/
void L01_SetTXAddr( INT8U *pAddr, INT8U Addr_Length );

/*Write address for a RX pipe*/
void L01_SetRXAddr( INT8U PipeNum, INT8U *pAddr, INT8U Addr_Length );

/*Set a hopping point for the device,must be 0-125*/
void L01_WriteHoppingPoint( INT8U FreqPoint );

/*Set the device as TX or RX mode*/
typedef enum{ TX_MODE, RX_MODE }L01MD;
void L01_SetTRMode( L01MD mode );

/*Send the communication speed of the RF device*/
typedef enum{ SPD_250K, SPD_1M, SPD_2M  }L01SPD;
void L01_SetSpeed( L01SPD speed );

/*Send the transmitt power of the device*/
typedef enum{ P_F18DBM, P_F12DBM, P_F6DBM, P_0DBM }L01PWR;
void L01_SetPower( L01PWR power );

/*Set the nRF24L01 into Powerup mode*/
void L01_SetPowerUp( void );

/*Set the nRF24L01 into PowerDown mode*/
void L01_SetPowerDown( void );

/*Check the TX FIFO if is empty,1:empty, 0:not empty*/
INT8U L01_IsTXEmpty( void );

/*Initialize the nRF24L01*/
void L01_Init( void );


#endif//_nRF24L01_H_
/*
================================================================================
====================================End of file=================================
================================================================================
*/