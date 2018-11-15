/*
================================================================================
Copyright(c)  Yihe Embedded Studio, All rights reserved.
              www.yhmcu.com
              http://yhmcu.taobao.com

File Name   : nRF24L01.c
Description : nRF24L01 low level operations and configurations.
Author      : ShawZhang,LiYong
Date        : 2012-4-17 9:08:36
Version     : 2.0
Compiler    : Any
Note        : None
================================================================================
*/
#define _nRF24L01_C_
#include "nRF24L01.h"

INT8U CE_Status = 0;
/*
================================================================================
Function : L01_GetCEStatus( )
Description : Get the status of the CE PIN
Input : NONE
Output: 1:CE=1, 0:CE=0
================================================================================
*/
INT8U L01_GetCEStatus( void )
{
	return CE_Status;
}
/*
================================================================================
Function : L01_SetCE( )
Description : Set the CE PIN as 1 or 0
Input : -status, 1: CE=1, 0: CE=0
Output: None
================================================================================
*/
void L01_SetCE( INT8U status )
{
	CE_Status = status;
	if( status == 0 )  { L01_CE_LOW( ); }
	else               { L01_CE_HIGH( ); }
}
/*
================================================================================
Function : L01_ReadSingleReg( )
Description : Read a single register of nRF24L01
Input : -Addr, The address of the register
Output: The value read from the register
================================================================================
*/
INT8U L01_ReadSingleReg( INT8U Addr )
{
    INT8U btmp;
    L01_CSN_LOW( );
    SPI_ExchangeByte( R_REGISTER | Addr );
    btmp = SPI_ExchangeByte( 0xFF );
    L01_CSN_HIGH( );
    return btmp;
}
/*
================================================================================
Function : L01_ReadMultiReg( )
Description : Read several registers of nRF24L01
Input : -StartAddr, The start address of the registers
        -nBytes, How many registers do you want to read
        -pBuff, The buffer to save the values
Output: None
================================================================================
*/
/*void L01_ReadMultiReg( INT8U StartAddr, INT8U nBytes, INT8U *pBuff )
{
    INT8U btmp;
    L01_CSN_LOW( );
    SPI_ExchangeByte( R_REGISTER | StartAddr );
    for( btmp = 0; btmp < nBytes; btmp ++ )
    {
        *( pBuff + btmp ) = SPI_ExchangeByte( 0xFF );
    }
    L01_CSN_HIGH( );
}

================================================================================
Function : L01_WriteSingleReg( )
Description : Write a single byte to a register
Input : -Addr, The address of the register
        -Value, The value to be written
Output: None
================================================================================
*/
void L01_WriteSingleReg( INT8U Addr, INT8U Value )
{
	INT8U tmp = L01_GetCEStatus( );
	L01_SetCE( 0 );
    L01_CSN_LOW( );
    SPI_ExchangeByte( W_REGISTER | Addr );
    SPI_ExchangeByte( Value );
    L01_CSN_HIGH( );
	L01_SetCE( tmp );
}
/*
================================================================================
Function : L01_WriteMultiReg( )
Description : Read several registers of nRF24L01
Input : -StartAddr, The start address of the registers
        -pBuff, The buffer store the values
        -Length, How many registers do you want to write
Output: None
================================================================================
*/
void L01_WriteMultiReg( INT8U StartAddr, INT8U *pBuff, INT8U Length )
{
    INT8U i;
	INT8U tmp = L01_GetCEStatus( );
	L01_SetCE( 0 );
    L01_CSN_LOW( );
    SPI_ExchangeByte( W_REGISTER | StartAddr );
    for( i = 0; i < Length; i ++ )
    {
        SPI_ExchangeByte( *( pBuff + i ) );
    }
    L01_CSN_HIGH( );
	L01_SetCE( tmp );
}
/*
================================================================================
Function : L01_FlushTX( )
Description : Flush the TX buffer
Input : None
Output: None
================================================================================
*/
void L01_FlushTX( void )
{
    L01_CSN_LOW( );
    SPI_ExchangeByte( FLUSH_TX );
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_FlushRX( )
Description : Flush the RX buffer
Input : None
Output: None
================================================================================
*/
void L01_FlushRX( void )
{
    L01_CSN_LOW( );
    SPI_ExchangeByte( FLUSH_RX );
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_ReuseTXPayload( )
Description : Reuse the last transmitted payload
Input : None
Output: None
================================================================================
*/
void L01_ReuseTXPayload( void )
{
    L01_CSN_LOW( );
    SPI_ExchangeByte( REUSE_TX_PL );
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_Nop( )
Description : nop operation of nRF24L01
Input : None
Output: None
================================================================================
*/
void L01_Nop( void )
{
    L01_CSN_LOW( );
    SPI_ExchangeByte( L01_NOP );
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_ReadStatusReg( )
Description : Read statu register of nRF24L01
Input : None
Output: Statu register of nRF24L01
================================================================================
*/
INT8U L01_ReadStatusReg( void )
{
    INT8U Status;
    L01_CSN_LOW( );
    Status = SPI_ExchangeByte( R_REGISTER + L01REG_STATUS );
    L01_CSN_HIGH( );
    return Status;
}
/*
================================================================================
Function : L01_ClearIRQ( )
Description : Clear IRQ cuased by nRF24L01
Input : None
Output: None
================================================================================
*/
void L01_ClearIRQ( INT8U IRQ_Source )
{
    INT8U btmp = 0;

    IRQ_Source &= ( 1<<RX_DR ) | ( 1<<TX_DS ) | ( 1<<MAX_RT );
    btmp = L01_ReadStatusReg( );
    L01_CSN_LOW( );
	L01_WriteSingleReg( L01REG_STATUS, IRQ_Source | btmp );
    L01_CSN_HIGH( );
    L01_ReadStatusReg( );
}
/*
================================================================================
Function : L01_ReadIRQSource( )
Description : Read the IRQ source of nRF24L01+
Input : None
Output: IRQ source mask code
================================================================================
*/
INT8U L01_ReadIRQSource( void )
{
    return ( L01_ReadStatusReg( ) & ( ( 1<<RX_DR ) | ( 1<<TX_DS ) | ( 1<<MAX_RT ) ) );
}
/*
================================================================================
Function : L01_ReadTopFIFOWidth( )
Description : Read the payload width of the top buffer of FIFO
Input : None
Output: The width of the pipe buffer
================================================================================
*/
INT8U L01_ReadTopFIFOWidth( void )
{
    INT8U btmp;
    L01_CSN_LOW( );
    SPI_ExchangeByte( R_RX_PL_WID );
    btmp = SPI_ExchangeByte( 0xFF );
    L01_CSN_HIGH( );
    return btmp;
}
/*
================================================================================
Function : L01_ReadRXPayload( )
Description : Read the RX payload from internal buffer
Input : -pBuff, buffer to store the data
Output: The length of data read
================================================================================
*/
INT8U L01_ReadRXPayload( INT8U *pBuff )
{
    INT8U width, PipeNum;
    PipeNum = ( L01_ReadSingleReg( L01REG_STATUS ) >> 1 ) & 0x07;
    width = L01_ReadTopFIFOWidth( );

    L01_CSN_LOW( );
    SPI_ExchangeByte( R_RX_PAYLOAD );
    for( PipeNum = 0; PipeNum < width; PipeNum ++ )
    {
        *( pBuff + PipeNum ) = SPI_ExchangeByte( 0xFF );
    }
    L01_CSN_HIGH( );
    L01_FlushRX( );
    return width;
}
/*
================================================================================
Function : L01_WriteTXPayload( )
Description : Write TX payload to a pipe and prx will return ack back
Input : -PipeNum, number of the pipe
        -pBuff, A buffer stores the data
        -nBytes, How many bytes to be wrote to
Output: None
================================================================================
*/
void L01_WriteTXPayload_Ack( INT8U *pBuff, INT8U nBytes )
{
    INT8U btmp;
    INT8U length = ( nBytes > 32 ) ? 32 : nBytes;

    L01_FlushTX( );
    L01_CSN_LOW( );
    SPI_ExchangeByte( W_TX_PAYLOAD );
    for( btmp = 0; btmp < length; btmp ++ )
    {
        SPI_ExchangeByte( *( pBuff + btmp ) );
    }
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_WritePayload_NoAck( )
Description : write data in tx mode, and prx won't return ack back
Input : -Data, A buffer stores the address data
        -Data_Length, How many bytes of the data buff
Output: None
================================================================================
*/
void L01_WriteTXPayload_NoAck( INT8U *Data, INT8U Data_Length )
{
    if( Data_Length > 32 || Data_Length == 0 )
    {
        return ;
    }
    L01_CSN_LOW( );
    SPI_ExchangeByte( W_TX_PAYLOAD_NOACK );
    while( Data_Length-- )
    {
        SPI_ExchangeByte( *Data++ );
    }
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_WritePayload_InAck( )
Description : write data in tx fifo when rx mode
Input : -Data, A buffer stores the address data
        -Data_Length, How many bytes of the data buff
Output: None
================================================================================
*/
void L01_WriteRXPayload_InAck( INT8U *pData, INT8U Data_Length )
{
    INT8U length = ( Data_Length > 32 ) ? 32 : Data_Length;
    INT8U btmp;

    L01_CSN_LOW( );
    SPI_ExchangeByte( W_ACK_PAYLOAD );
    for( btmp = 0; btmp < length; btmp ++ )
    {
        SPI_ExchangeByte( *( pData + btmp ) );
    }
    L01_CSN_HIGH( );
}
/*
================================================================================
Function : L01_SetTXAddr( )
Description : Write address for the own device
Input : -pAddr, A buffer stores the address data
        -Addr_Length, How many bytes of the address
Output: None
================================================================================
*/
void L01_SetTXAddr( INT8U *pAddr, INT8U Addr_Length )
{
    INT8U Length = ( Addr_Length > 5 ) ? 5 : Addr_Length;
    L01_WriteMultiReg( L01REG_TX_ADDR, pAddr, Length );
}
/*
================================================================================
Function : L01_SetRXAddr( )
Description : Write address for a RX pipe
Input : -PipeNum, number of the pipe
        -pAddr, A buffer stores the address data
        -Addr_Length, How many bytes of the address
Output: None
================================================================================
*/
void L01_SetRXAddr( INT8U PipeNum, INT8U *pAddr, INT8U Addr_Length )
{
    INT8U Length = ( Addr_Length > 5 ) ? 5 : Addr_Length;
    INT8U pipe = ( PipeNum > 5 ) ? 5 : PipeNum;

    L01_WriteMultiReg( L01REG_RX_ADDR_P0 + pipe, pAddr, Length );
}
/*
================================================================================
Function : L01_SetSpeed )
Description : Send the communication speed of the RF device
Input :    speed,
Output: None
================================================================================
*/
void L01_SetSpeed( L01SPD speed )
{
	INT8U btmp = L01_ReadSingleReg( L01REG_RF_SETUP );

	btmp &= ~( ( 1<<5 ) | ( 1<<3 ) );
	if( speed == SPD_250K )		//250K
	{
		btmp |= ( 1<<5 );
	}
	else if( speed == SPD_1M )   //1M
	{
   		btmp &= ~( ( 1<<5 ) | ( 1<<3 ) );
	}
	else if( speed == SPD_2M )   //2M
	{
		btmp |= ( 1<<3 );
	}

	L01_WriteSingleReg( L01REG_RF_SETUP, btmp );
}
/*
================================================================================
Function : L01_SetPower )
Description : Send the transmitt power of the device
Input :    power
Output: None
================================================================================
*/
void L01_SetPower( L01PWR power )
{
    INT8U btmp = L01_ReadSingleReg( L01REG_RF_SETUP ) & ~0x07;
    switch( power )
    {
        case P_F18DBM:
            btmp |= PWR_18DB;
            break;
        case P_F12DBM:
            btmp |= PWR_12DB;
            break;
        case P_F6DBM:
            btmp |= PWR_6DB;
            break;
        case P_0DBM:
            btmp |= PWR_0DB;
            break;
        default:
            break;
    }
    L01_WriteSingleReg( L01REG_RF_SETUP, btmp );
}
/*
================================================================================
Function : L01_WriteHoppingPoint( )
Description : Set frequency for the device,must be 0-125
Input : -FreqPoint, The hopping point
Output: None
================================================================================
*/
void L01_WriteHoppingPoint( INT8U FreqPoint )
{
    L01_WriteSingleReg( L01REG_RF_CH, FreqPoint & 0x7F );
}
/*
================================================================================
Function : L01_IsTXEmpty( )
Description : Check the TX FIFO if is empty,1:empty, 0:not empty
Input : -FreqPoint, The hopping point
Output: 1:empty, 0:not empty
================================================================================
*/
INT8U L01_IsTXEmpty( void )
{
	return L01_ReadSingleReg( L01REG_FIFO_STATUS ) & ( 1<<TX_EMPTY );
}

/*
================================================================================
Function : L01_SetTRMode( )
Description : Set the device as TX or RX mode
Input : -mode, the mode to be set
            @TX_MODE, TX mode
            @RX_MODE, RX mode
Output: None
================================================================================
*/
void L01_SetTRMode( L01MD mode )
{
    INT8U controlreg = L01_ReadSingleReg( L01REG_CONFIG );
    if( mode == TX_MODE )       { controlreg &= ~( 1<<PRIM_RX ); L01_SetCE( 0 ); }
    else if( mode == RX_MODE )  { controlreg |= ( 1<<PRIM_RX ); L01_SetCE( 1 );}

    L01_WriteSingleReg( L01REG_CONFIG, controlreg );
}
/*
================================================================================
Function : L01_SetPowerDown( )
Description : Set the nRF24L01 into PowerDown mode
Input : None
Output: None
================================================================================
*/
void L01_SetPowerDown( void )
{
	INT8U controlreg = L01_ReadSingleReg( L01REG_CONFIG );
    L01_WriteSingleReg( L01REG_CONFIG, controlreg & ( ~( 1<<PWR_UP ) ) );
}
/*
================================================================================
Function : L01_SetPowerUp( )
Description : Set the nRF24L01 into Powerup mode
Input : None
Output: None
================================================================================
*/
void L01_SetPowerUp( void )
{
	INT8U controlreg = L01_ReadSingleReg( L01REG_CONFIG );
    L01_WriteSingleReg( L01REG_CONFIG, controlreg | ( 1<<PWR_UP ) );
}
/*
================================================================================
Function : L01_Init( )
Description : Initialize the nRF24L01
Input : None
Output: None
================================================================================
*/
void L01_Init( void )
{
    INT8U addr[5] = {INIT_ADDR};

	INT8U tmp = L01_GetCEStatus( );
    L01_SetCE( 0 );
	L01_SetPowerDown( );
    L01_ClearIRQ( IRQ_ALL );
#if DYNAMIC_PACKET == 1
    //dynamic payload length
    L01_WriteSingleReg( L01REG_DYNPD, ( 1<<0 ) );//Enable pipe 0 dynamic payload length
    L01_WriteSingleReg( L01REG_FEATRUE, 0x06 );
    L01_ReadSingleReg( L01REG_DYNPD );
    L01_ReadSingleReg( L01REG_FEATRUE );
#elif DYNAMIC_PACKET == 0
    //Fixed packet length
    L01_WriteSingleReg( L01REG_RX_PW_P0, FIXED_PACKET_LEN );
#endif//DYNAMIC_PACKET

    L01_WriteSingleReg( L01REG_CONFIG,/* ( 1<<MASK_TX_DS ) |*////receive interrupt
                                      ( 1<<EN_CRC ) );     //Enable CRC, 1 byte
    L01_WriteSingleReg( L01REG_EN_AA, ( 1<<ENAA_P0 ) );   //Auto ack in pipe 0
    L01_WriteSingleReg( L01REG_EN_RXADDR, ( 1<<ERX_P0 ) );//Enable pipe 0 receive
    L01_WriteSingleReg( L01REG_SETUP_AW, AW_5BYTES );     //Address width : 5Byte
    L01_WriteSingleReg( L01REG_RETR, (((REPEAT_TIME/250)&0X0F)<<4) |
                        ( REPEAT_CNT & 0x0F ) );          //repeat SETTING
    L01_WriteSingleReg( L01REG_RF_CH, 0x0 );             //Initial channel
    L01_WriteSingleReg( L01REG_RF_SETUP, 0x27 );
    L01_SetTXAddr( &addr[0], 5 );                          //Set TX address
    L01_SetRXAddr( 0, &addr[0], 5 );                       //Set RX address
	L01_SetPowerUp( );
}
/*
================================================================================
====================================End of file=================================
================================================================================
*/
