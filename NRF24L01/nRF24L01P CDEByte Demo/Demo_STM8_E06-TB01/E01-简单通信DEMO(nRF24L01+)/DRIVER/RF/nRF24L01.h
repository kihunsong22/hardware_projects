/*===========================================================================
* 网址 ：http://yhmcu.taobao.com/   http://www.cdebyte.com/                 *
* 作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司                 * 
* 邮件 ：yihe_liyong@126.com                                                *
* 电话 ：18615799380                                                        *
============================================================================*/

#ifndef _nRF24L01_H_
#define _nRF24L01_H_

#include "STM8l10x_conf.h"
#include "MyTypedef.h"
#include "nRF24L01_Reg.h"

#define REPEAT_CNT      15  // 0-15, repeat transmit count
#define INIT_ADDR       1, 2, 3, 4, 5

// nRF24L01P相关控制引脚定义， CSN(PB4), IRQ(PA3), CE(PA2) 
#define PORT_L01_CSN    GPIOB
#define PIN_L01_CSN     GPIO_Pin_4

#define PORT_L01_IRQ    GPIOA
#define PIN_L01_IRQ     GPIO_Pin_3

#define PORT_L01_CE     GPIOA
#define PIN_L01_CE      GPIO_Pin_2

#define L01_CSN_LOW()   GPIO_ResetBits(PORT_L01_CSN, PIN_L01_CSN)
#define L01_CSN_HIGH()  GPIO_SetBits(PORT_L01_CSN, PIN_L01_CSN)

#define L01_CE_LOW()    GPIO_ResetBits(PORT_L01_CE, PIN_L01_CE)
#define L01_CE_HIGH()   GPIO_SetBits(PORT_L01_CE, PIN_L01_CE)

#define L01_IRQ_READ()  GPIO_ReadInputDataBit(PORT_L01_IRQ, PIN_L01_IRQ)

// nRF24L01P相关函数接口
// 初始化L01
void L01_Init(void);

// 复位TX FIFO指针      
void L01_FlushTX(void);

// 复位RX FIFO指针     
void L01_FlushRX(void);     

// 读取中断
INT8U L01_ReadIRQSource(void);          

// 清除中断
#define IRQ_ALL  ((1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT))
void L01_ClearIRQ(INT8U IRQ_Source); 
   
// 读取FIFO数据宽度
INT8U L01_ReadTopFIFOWidth(void);

// 读取接收到的数据       
INT8U L01_ReadRXPayload(INT8U *pBuff);  

// 设置L01模式 
typedef enum{ TX_MODE, RX_MODE } L01MD; 
void L01_SetTRMode(L01MD mode);

// 设置L01空速                 
typedef enum{ SPD_250K, SPD_1M, SPD_2M } L01SPD;
void L01_SetSpeed(L01SPD speed);

// 设置L01功率                 
typedef enum{ P_F18DBM, P_F12DBM, P_F6DBM, P_0DBM } L01PWR;
void L01_SetPower(L01PWR power);

// 设置L01频率                
void L01_WriteHoppingPoint(INT8U FreqPoint);    

INT8U L01_ReadStatusReg(void);

// 写数据到一个寄存器
void L01_WriteSingleReg(INT8U Addr, INT8U Value);

// 读取一个寄存器的值   
INT8U L01_ReadSingleReg(INT8U Addr);

// 读取多个寄存器的值                 
void L01_ReadMultiReg(INT8U StartAddr, INT8U nBytes, INT8U *pBuff);

// 写数据到多个寄存器
void L01_WriteMultiReg(INT8U StartAddr, INT8U *pBuff, INT8U Length);

// 写数据到TXFIFO(带ACK返回)
void L01_WriteTXPayload_Ack(INT8U *pBuff, INT8U nBytes);

// 写数据到TXFIFO(不带ACK返回)
void L01_WriteTXPayload_NoAck(INT8U *Data, INT8U Data_Length);

// 设置发送物理地址
void L01_SetTXAddr(INT8U *pAddr, INT8U Addr_Length);

// 设置接收物理地址
void L01_SetRXAddr(INT8U PipeNum, INT8U *pAddr, INT8U Addr_Length);

#endif//_nRF24L01_H_

/*===========================================================================
-----------------------------------文件结束----------------------------------
===========================================================================*/