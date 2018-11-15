

#include "nRF24L01.h"
#include "board.h"
#include "Oled.h"

void main( void )
{
	INT32U dly;
    INT8U tmp,x,testbuffer[10] = {"00000"};
	INT16U tx_couter = 0, itmp;

	SPI_Initial( );
	GPIO_Initial( );


	//初始化L01+的控制口CE = 0
	L01_CE_LOW( );

	//初始化L01+的控制口IRQ,使用查询方式
	PORTB |= ( 1<<6 );
	DDRB  &= ~( 1<<6 );

	//L01+的操作，已经被建成C库，见nRF24L01.h文件，提供SPI和CSN操作，即可调用其内部所有函数。
	//用户无需再关心L01+的寄存器操作问题。
	L01_Init( );
	L01_SetTRMode( TX_MODE );
	L01_WriteHoppingPoint( 0 );
	//L01_SetSpeed( SPD_250K );

	//关闭LED
	LED_Off( );

	LCD_Init( );
	LCD_Dis_Logo( );
	LCD_Dis_Str( 2,24,"AVR board" );
    LCD_Dis_Str( 4,0,"yhmcu.taobao.com" );
    LCD_Dis_Str( 6,0,"24L01+:TX " );
    LCD_Dis_Str( 6, 80, (char*)testbuffer );

	/*功能：
	1、此为发射程序，每隔一段延时，发送一个字符串。
	2、用一个LED指示发射开始，另一个LED指示发射状态
	3、发射开始：LED闪烁一次，发射结果：显示在OLED
	4、发射采用自动应答模式，若取消，通信距离将延长。
	*/
	while( 1 )
	{
        //延时发射
        for( dly = 0; dly < 5000; dly ++ );
        //发射一个字符串
        L01_FlushRX( );
        L01_FlushTX( );
        L01_WriteTXPayload_Ack( (INT8U*)"123\r\n", strlen( "123\r\n" ) );
        L01_CE_HIGH( );	// CE = 1,启动发射

        //等待发射中断产生，判断发射失败或成功
        while( PINB & ( 1<<6 ) );
        while( ( tmp = L01_ReadIRQSource( ) ) == 0 );
        L01_CE_LOW( );	// 发射完毕，CE = 0，省电

        if( tmp & ( 1<<TX_DS ) )
        {
            //发射成功，LED翻转
            LED_Toggle( );

            tx_couter ++;
            itmp = tx_couter;
            testbuffer[0] = ( itmp / 10000 ) + '0';
            itmp %= 10000;
            testbuffer[1] = ( itmp / 1000 ) + '0';
            itmp %= 1000;
            testbuffer[2] = ( itmp / 100 ) + '0';
            itmp %= 100;
            testbuffer[3] = ( itmp / 10 ) + '0';
            itmp %= 10;
            testbuffer[4] = itmp + '0';
            testbuffer[5] = 0;
            LCD_Dis_Str( 6, 80, (char*)testbuffer );
        }
        if( tmp & ( 1<<MAX_RT ) )
        {
            //发射失败，LED长亮
			LED_On( );
        }
        if( tmp & ( 1<<RX_DR )  )
        {

        }
        L01_ClearIRQ( IRQ_ALL );
	}
}


