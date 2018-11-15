/*===========================================================================
* 网址 ：http://www.cdebyte.com/   http://yhmcu.taobao.com/                 *
* 作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司                 * 
* 邮件 ：yihe_liyong@126.com                                                *
* 电话 ：18615799380                                                        *
============================================================================*/

#include "bsp.h"

/*===========================================================================
* 函数 ：SClK_Initial() => 初始化系统时钟，系统时钟 = 16MHZ                 *
============================================================================*/
void SClK_Initial(void)
{
	CLK_MasterPrescalerConfig(CLK_MasterPrescaler_HSIDiv1); // 16M
}

/*===========================================================================
* 函数 ：GPIO_Initial() => 初始化通用IO端口
============================================================================*/
void GPIO_Initial(void)
{
    // 配置LED引脚和KEY引脚
    GPIO_Init(PORT_KEY, PIN_KEY, GPIO_Mode_In_PU_No_IT);
    
    GPIO_Init(PORT_LED_R, PIN_LED_R, GPIO_Mode_Out_PP_High_Slow);
    GPIO_SetBits(PORT_LED_R, PIN_LED_R);
    
    GPIO_Init(PORT_LED_Y, PIN_LED_Y, GPIO_Mode_Out_PP_High_Slow);
    GPIO_SetBits(PORT_LED_Y, PIN_LED_Y);

    // 配置nRF24L01P相关控制引脚
    GPIO_Init(PORT_L01_IRQ, PIN_L01_IRQ, GPIO_Mode_In_PU_No_IT);
    
    GPIO_Init(PORT_L01_CE, PIN_L01_CE, GPIO_Mode_Out_PP_High_Slow);
    GPIO_SetBits(PORT_L01_CE, PIN_L01_CE);
    
    GPIO_Init(PORT_L01_CSN, PIN_L01_CSN, GPIO_Mode_Out_PP_High_Slow);
    GPIO_SetBits(PORT_L01_CSN, PIN_L01_CSN);
}

/*=============================================================================
*Function:  USART_Initial() => 初始化串口
=============================================================================*/
void USART_Initial(void)
{
    GPIO_Init(PORT_USART, PIN_RXD, GPIO_Mode_In_FL_No_IT);      // RXD
    GPIO_Init(PORT_USART, PIN_TXD, GPIO_Mode_Out_OD_HiZ_Fast);  // TXD
    
    CLK_PeripheralClockConfig(CLK_Peripheral_USART, ENABLE);

    USART_Init((uint32_t)9600, USART_WordLength_8D, USART_StopBits_1,
                USART_Parity_No, (USART_Mode_TypeDef)(USART_Mode_Rx | USART_Mode_Tx));

    USART_ITConfig(USART_IT_RXNE, ENABLE);
    
    USART_ClearITPendingBit();
    
    USART_Cmd(ENABLE);
    
}

/*===========================================================================
* 函数 ：SPI_Initial() => 初始化SPI                                         *
============================================================================*/
void SPI_Initial(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI, ENABLE);
	
	SPI_DeInit();
	
	// 配置SPI相关参数,2分频（8MHZ）
	SPI_Init(SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2,
             SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge,
             SPI_Direction_2Lines_FullDuplex, SPI_NSS_Soft);

	SPI_Cmd(ENABLE);
	
	// SPI相关IO口配置
	GPIO_Init(PORT_SPI, PIN_MISO, GPIO_Mode_In_PU_No_IT);       // MISO (PB7)
	GPIO_Init(PORT_SPI, PIN_SCLK, GPIO_Mode_Out_PP_High_Slow);  // SCLK (PB5)
	GPIO_Init(PORT_SPI, PIN_MOSI, GPIO_Mode_Out_PP_High_Slow);  // MOSI (PB6)
}

/*===========================================================================
* 函数 ：TIM3_Initial() => 初始化定时器3，定时时间为1ms                     *
============================================================================*/
void TIM3_Initial(void)
{
    TIM3_DeInit();

    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, ENABLE);

    // 配置Timer3相关参数，时钟为16/16 = 1MHZ，定时时间 = 1000/1000000 = 1ms
    TIM3_TimeBaseInit(TIM3_Prescaler_16, TIM3_CounterMode_Up, 1000);
    TIM3_ITConfig(TIM3_IT_Update, ENABLE);

    TIM3_Cmd(ENABLE);
}

/*===========================================================================
* 函数 ：SPI_ExchangeByte() => 通过SPI进行数据交换                          * 
* 输入 ：需要写入SPI的值                                                    * 
* 输出 ：通过SPI读出的值                                                    * 
============================================================================*/
INT8U SPI_ExchangeByte(INT8U input)
{
	while (RESET == SPI_GetFlagStatus(SPI_FLAG_TXE));   // 等待数据传输完成	
	SPI_SendData(input);
	while (RESET == SPI_GetFlagStatus(SPI_FLAG_RXNE));  // 等待数据接收完成
	return (SPI_ReceiveData());
}

/*===========================================================================
-----------------------------------文件结束----------------------------------
===========================================================================*/
