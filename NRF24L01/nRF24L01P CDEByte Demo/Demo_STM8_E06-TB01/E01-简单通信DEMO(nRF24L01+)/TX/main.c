/*===========================================================================
* 网址 ：http://www.cdebyte.com/   http://yhmcu.taobao.com/                 *
* 作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司                 * 
* 邮件 ：yihe_liyong@126.com                                                *
* 电话 ：18615799380                                                        *
============================================================================*/

#include "bsp.h"         

// 常量定义
#define TX              1       // 发送模式
#define RX              0       // 接收模式

#define SEND_GAP        1000    // 每间隔1s发送一次数据        
#define SEND_LENGTH     5       // 发送数据每包的长度

INT8U   Cnt1ms = 0;             // 1ms计数变量，每1ms加一 
INT8U   SendFlag = 0;           // =1，发送无线数据，=0不处理
INT16U  SendTime = 1;           // 计数数据发送间隔时间                
INT16U  SendCnt = 0;            // 计数发送的数据包数                

// 需要发送的数据  
INT8U   SendBuffer[SEND_LENGTH] = { "123\r\n" }; 

// 串口相关变量
INT8U   COM_TxNeed = 0;
INT8U   COM_RxCounter = 0;
INT8U   COM_TxCounter = 0;
INT8U   COM_RxBuffer[65] = { 0 };
INT8U   COM_TxBuffer[65] = { 0 };

#define USRAT_SendByte()    USART_SendData8(COM_TxBuffer[COM_TxCounter++])
#define USRAT_RecvByte()    COM_RxBuffer[COM_RxCounter++]=USART_ReceiveData8()

/*===========================================================================
* 函数 : DelayMs() => 延时函数(ms级)                                        *
* 输入 ：x, 需要延时多少(0-255)                                             *
============================================================================*/
void DelayMs(INT8U x)
{
    Cnt1ms = 0;
    while (Cnt1ms <= x);
}

/*===========================================================================
* 函数 ：TIM3_1MS_ISR() => 定时器3服务函数, 定时时间基准为1ms               *
============================================================================*/
void TIM3_1MS_ISR(void)
{
    Cnt1ms++;
    
    if (0 != SendTime)      // 1ms时间到，置位SendFlag标志，主函数查询发送数据      
    { 
        if (--SendTime == 0)    { SendTime = SEND_GAP; SendFlag = 1; }
    } 
}

/*=============================================================================
* 函数 : USART_Send() => 通过串口发送数据                                     *
* 输入 ：buff, 待发送数据     size， 发送长度                                *
=============================================================================*/
void USART_Send(INT8U *buff, INT8U size)
{
    if (size == 0)          { return; }
    
    COM_TxNeed = 0;
    
    while (size --)         { COM_TxBuffer[COM_TxNeed++] = *buff++; }
    
    COM_TxCounter = 0;
    USART_ITConfig(USART_IT_TXE, ENABLE);
}

/*=============================================================================
* 函数:  USART_RX_Interrupt() => 串口接收中断                                 *
=============================================================================*/
void USART_RX_Interrupt(void)
{
    USRAT_RecvByte();
}

/*=============================================================================
* 函数:  USART_TX_Interrupt() =>串口发送中断
=============================================================================*/
void USART_TX_Interrupt(void)
{
    if (COM_TxCounter < COM_TxNeed)     { USRAT_SendByte(); }   
    else    
    { 
        USART_ITConfig(USART_IT_TC, ENABLE);
        USART_ITConfig(USART_IT_TXE, DISABLE);

        if (USART_GetFlagStatus(USART_FLAG_TC))      
        {
            USART_ITConfig(USART_IT_TC, DISABLE); 
            COM_TxNeed = 0;
            COM_TxCounter = 0;
        }
    }
}

/*===========================================================================
* 函数 ：MCU_Initial() => 初始化CPU所有硬件                                 *
* 说明 ：关于所有硬件的初始化操作，已经被建成C库，见bsp.c文件               *
============================================================================*/
void MCU_Initial(void)
{
    SClK_Initial();         // 初始化系统时钟，16M     
    GPIO_Initial();         // 初始化GPIO                  
    TIM3_Initial();         // 初始化定时器3，基准1ms 
    USART_Initial();        // 初始化串口
    SPI_Initial();          // 初始化SPI               

    enableInterrupts();     // 打开总中断              
}

/*===========================================================================
* 函数 ：RF_Initial() => 初始化RF芯片                                       *
* 输入 ：mode, =0,接收模式， else,发送模式                                  *
* 说明 ：L01+的操作，已经被建成C库，见nRF24L01.c文件， 提供SPI和CSN操作，	*
         即可调用其内部所有函数用户无需再关心L01+的寄存器操作问题。			*
============================================================================*/
void RF_Initial(INT8U mode)
{
	L01_Init();                                     // 初始化L01寄存器
	if (RX == mode)     { L01_SetTRMode(RX_MODE); } // 接收模式      
	L01_WriteHoppingPoint(60);                      // 设置频率            
	L01_SetSpeed(SPD_1M);                           // 设置空速为1M        
	
	L01_FlushRX();                                  // 复位接收FIFO指针    
    L01_FlushTX();                                  // 复位发送FIFO指针
    L01_ClearIRQ(IRQ_ALL);                          // 清除所有中断
    if (RX == mode)     { L01_CE_HIGH(); }          // CE = 1, 启动接收          
}

/*===========================================================================
* 函数: System_Initial() => 初始化系统所有外设                              *
============================================================================*/
void System_Initial(void)
{
    MCU_Initial();      // 初始化CPU所有硬件
    RF_Initial(TX);     // 初始化无线芯片,发送模式           
}

/*===========================================================================
* 函数 : BSP_RF_SendPacket() => 无线发送数据函数                            *
* 说明 ：Sendbuffer指向待发送的数据，length发送数据长度                     *
============================================================================*/
INT8U RF_SendPacket(INT8U *Sendbuffer, INT8U length)
{
    INT8U tmp = 0;
    
    L01_CE_LOW();               // CE = 0, 关闭发送    
    
    L01_SetTRMode(TX_MODE);     // 设置为发送模式      	
    L01_WriteTXPayload_NoAck(SendBuffer, length);  
        
    L01_CE_HIGH();              // CE = 1, 启动发射 
    
    LED0_TOG();    
    USART_Send("Transmit ok\r\n", 13);
    
    DelayMs(250);
    
    // 等待发射中断产生
    while (0 != L01_IRQ_READ());
    while (0 == (tmp=L01_ReadIRQSource()));
    
    L01_FlushTX();              // 复位发送FIFO指针    
	L01_ClearIRQ(IRQ_ALL);      // 清除中断 
	             
    L01_CE_LOW();               // CE = 0, 关闭发送    
    
    return (tmp & (1<<TX_DS));  // 返回发送是否成功   
}

/*===========================================================================
* 函数 : main() => 主函数，程序入口                                         *
* 说明 ：没1s发送一包数据，每包数据长度为5个字节，“123/r/n”                 *
============================================================================*/
void main(void)
{
	System_Initial();                       // 初始化系统所有外设               

	while (1)
	{
	    if (0 != SendFlag)                  // 1s到，发送数据
        {
            // 数据发送成功(收到接收方的应答)
            if (RF_SendPacket(SendBuffer, SEND_LENGTH))
            {
                LED1_ON();                  // LED点亮，用于指示应答成功
                USART_Send("Ack ok\r\n", 8);
            }
            else
            {
                LED1_OFF();
                USART_Send("Ack error\r\n", 11);
            }
            
            SendFlag = 0;                
        }
	}
}
