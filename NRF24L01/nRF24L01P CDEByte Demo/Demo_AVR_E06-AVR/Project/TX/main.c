

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


	//��ʼ��L01+�Ŀ��ƿ�CE = 0
	L01_CE_LOW( );

	//��ʼ��L01+�Ŀ��ƿ�IRQ,ʹ�ò�ѯ��ʽ
	PORTB |= ( 1<<6 );
	DDRB  &= ~( 1<<6 );

	//L01+�Ĳ������Ѿ�������C�⣬��nRF24L01.h�ļ����ṩSPI��CSN���������ɵ������ڲ����к�����
	//�û������ٹ���L01+�ļĴ����������⡣
	L01_Init( );
	L01_SetTRMode( TX_MODE );
	L01_WriteHoppingPoint( 0 );
	//L01_SetSpeed( SPD_250K );

	//�ر�LED
	LED_Off( );

	LCD_Init( );
	LCD_Dis_Logo( );
	LCD_Dis_Str( 2,24,"AVR board" );
    LCD_Dis_Str( 4,0,"yhmcu.taobao.com" );
    LCD_Dis_Str( 6,0,"24L01+:TX " );
    LCD_Dis_Str( 6, 80, (char*)testbuffer );

	/*���ܣ�
	1����Ϊ�������ÿ��һ����ʱ������һ���ַ�����
	2����һ��LEDָʾ���俪ʼ����һ��LEDָʾ����״̬
	3�����俪ʼ��LED��˸һ�Σ�����������ʾ��OLED
	4����������Զ�Ӧ��ģʽ����ȡ����ͨ�ž��뽫�ӳ���
	*/
	while( 1 )
	{
        //��ʱ����
        for( dly = 0; dly < 5000; dly ++ );
        //����һ���ַ���
        L01_FlushRX( );
        L01_FlushTX( );
        L01_WriteTXPayload_Ack( (INT8U*)"123\r\n", strlen( "123\r\n" ) );
        L01_CE_HIGH( );	// CE = 1,��������

        //�ȴ������жϲ������жϷ���ʧ�ܻ�ɹ�
        while( PINB & ( 1<<6 ) );
        while( ( tmp = L01_ReadIRQSource( ) ) == 0 );
        L01_CE_LOW( );	// ������ϣ�CE = 0��ʡ��

        if( tmp & ( 1<<TX_DS ) )
        {
            //����ɹ���LED��ת
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
            //����ʧ�ܣ�LED����
			LED_On( );
        }
        if( tmp & ( 1<<RX_DR )  )
        {

        }
        L01_ClearIRQ( IRQ_ALL );
	}
}

