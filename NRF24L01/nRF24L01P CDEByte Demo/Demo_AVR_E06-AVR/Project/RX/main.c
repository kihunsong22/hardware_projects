
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

    //��ʼ��L01+�Ŀ��ƿ�CE = 1
	PORTB |= ( 1<<7 );
	DDRB |= ( 1<<7 );

    //��ʼ��L01+�Ŀ��ƿ�IRQ,ʹ�ò�ѯ��ʽ
    PORTB |= ( 1<<6 );
	DDRB &= ~( 1<<6 );

    //L01+�Ĳ������Ѿ�������C�⣬��nRF24L01.h�ļ����ṩSPI��CSN���������ɵ������ڲ����к�����
    //�û������ٹ���L01+�ļĴ����������⡣
    L01_Init( );
    L01_SetTRMode( RX_MODE );
    L01_WriteHoppingPoint( 0 );
    //L01_SetSpeed( SPD_250K );

	//�ر�LED
	LED_Off( );
    LCD_Init( );            //OLED��ʼ��

    LCD_Dis_Logo( );

    LCD_Dis_Str( 2,24,"AVR  board" );
    LCD_Dis_Str( 4,0,"yhmcu.taobao.com" );
    LCD_Dis_Str( 6,0,"24L01+:RX " );
    LCD_Dis_Str( 6, 80, (char*)testbuffer );
	while( 1 )
	{
        //���RX��TX���������ȴ������ж�
        L01_FlushRX( );
        L01_FlushTX( );
        //�ȴ������жϲ�����IRQ�������ͣ���PB6���
        //while( PINB & ( 1<<6 ) );
        //�����жϱ�ǲ�Ϊ0
        while( ( tmp = L01_ReadIRQSource( ) ) == 0 );
        //�ж��ж�����
        if( tmp & ( 1<<TX_DS ) )
        {
        }
        else if( tmp & ( 1<<MAX_RT ) )
        {
        }
        else if( tmp & ( 1<<RX_DR )  )
        {
            //���ճɹ����������ݣ��ж��Ƿ�ͷ�������һ�£�����LED ��ת
            for( tmp = 0; tmp < 32; tmp ++ )
            {
                testbuffer[tmp] = 0;
            }
            tmp = L01_ReadRXPayload( testbuffer );
            //�жϽ��յ�������ȷ�񣬶�Ӧ�ķ�����򣬷��͵����ַ���"123\r\n"
            if( tmp == 5 && testbuffer[0] == '1' && testbuffer[1] == '2' && testbuffer[2] == '3' )
            {

                LED_Toggle( );      //����������ȷ,LED��ת
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