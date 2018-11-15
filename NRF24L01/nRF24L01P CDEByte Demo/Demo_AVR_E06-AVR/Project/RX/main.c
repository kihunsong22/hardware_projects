
#include "board.h"
#include "OLED.h"
#include "nRF24L01.h"

void main( void )
{
    INT32U dly;
    INT8U tmp, testbuffer[32] = {'0','0','0','0','0', 0};
    INT16U rx_conter = 0, itmp;

    SPI_Initial( );
    GPIO_Initial( );

    //初始化L01+的控制口CE = 1
	PORTB |= ( 1<<7 );
	DDRB |= ( 1<<7 );

    //初始化L01+的控制口IRQ,使用查询方式
    PORTB |= ( 1<<6 );
	DDRB &= ~( 1<<6 );

    //L01+的操作，已经被建成C库，见nRF24L01.h文件，提供SPI和CSN操作，即可调用其内部所有函数。
    //用户无需再关心L01+的寄存器操作问题。
    L01_Init( );
    L01_SetTRMode( RX_MODE );
    L01_WriteHoppingPoint( 0 );
    //L01_SetSpeed( SPD_250K );

	//关闭LED
	LED_Off( );
    LCD_Init( );            //OLED初始化

    LCD_Dis_Logo( );

    LCD_Dis_Str( 2,24,"AVR  board" );
    LCD_Dis_Str( 4,0,"yhmcu.taobao.com" );
    LCD_Dis_Str( 6,0,"24L01+:RX " );
    LCD_Dis_Str( 6, 80, (char*)testbuffer );
	while( 1 )
	{
        //清空RX和TX缓冲区，等待接收中断
        L01_FlushRX( );
        L01_FlushTX( );
        //等待接收中断产生，IRQ将被拉低，用PB6检测
        //while( PINB & ( 1<<6 ) );
        //读出中断标记不为0
        while( ( tmp = L01_ReadIRQSource( ) ) == 0 );
        //判断中断类型
        if( tmp & ( 1<<TX_DS ) )
        {
        }
        else if( tmp & ( 1<<MAX_RT ) )
        {
        }
        else if( tmp & ( 1<<RX_DR )  )
        {
            //接收成功，读出数据，判断是否和发射内容一致，进行LED 翻转
            for( tmp = 0; tmp < 32; tmp ++ )
            {
                testbuffer[tmp] = 0;
            }
            tmp = L01_ReadRXPayload( testbuffer );
            //判断接收的数据正确否，对应的发射程序，发送的是字符串"123\r\n"
            if( tmp == 5 && testbuffer[0] == '1' && testbuffer[1] == '2' && testbuffer[2] == '3' )
            {

                LED_Toggle( );      //接收数据正确,LED翻转
                //Display the received count
                rx_conter ++;
                itmp = rx_conter;
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
        }
        L01_ClearIRQ( IRQ_ALL );
    }
}