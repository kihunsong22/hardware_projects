/*===========================================================================
网址 ：http://yhmcu.taobao.com/
作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司 
邮件 ：yihe_liyong@126.com
电话 ：18615799380
===========================================================================*/

#ifndef _BSP_H_
#define _BSP_H_

#include "STM8l10x_conf.h"
#include "nRF24L01.h"
#include "mytypedef.h"

// SPI引脚定义
#define PORT_SPI        GPIOB
#define PIN_SCLK        GPIO_Pin_5        
#define PIN_MOSI        GPIO_Pin_6
#define PIN_MISO        GPIO_Pin_7

// LED和KEY引脚定义
#define PORT_LED_R      GPIOB
#define PIN_LED_R       GPIO_Pin_0

#define PORT_LED_Y      GPIOD
#define PIN_LED_Y       GPIO_Pin_0

#define PORT_KEY        GPIOB
#define PIN_KEY         GPIO_Pin_1

#define PORT_USART      GPIOC
#define PIN_TXD         GPIO_Pin_3
#define PIN_RXD         GPIO_Pin_2

// LED操作函数，(ON)打开, (OFF)关闭，(TOG)翻转
#define LED_R_ON()       GPIO_ResetBits(PORT_LED_R, PIN_LED_R)        
#define LED_R_OFF()      GPIO_SetBits(PORT_LED_R, PIN_LED_R)
#define LED_R_TOG()      GPIO_ToggleBits(PORT_LED_R, PIN_LED_R)

#define LED_Y_ON()       GPIO_ResetBits(PORT_LED_Y, PIN_LED_Y)        
#define LED_Y_OFF()      GPIO_SetBits(PORT_LED_Y, PIN_LED_Y)
#define LED_Y_TOG()      GPIO_ToggleBits(PORT_LED_Y, PIN_LED_Y)

void SClK_Initial(void);                // 初始化系统时钟，系统时钟 = 16MHZ
void GPIO_Initial(void);                // 初始化通用IO端口
void SPI_Initial(void);                 // 初始化SPI
void TIM3_Initial(void);                // 初始化定时器3，定时时间为1ms
void USART_Initial(void);               // 初始化串口   
INT8U SPI_ExchangeByte(INT8U input);    // 通过SPI进行数据交换 

#endif //_BSP_H_

/*===========================================================================
-----------------------------------文件结束----------------------------------
===========================================================================*/
