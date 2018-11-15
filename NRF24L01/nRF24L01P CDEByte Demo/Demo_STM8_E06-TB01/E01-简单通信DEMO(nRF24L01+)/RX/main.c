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
       
#define SEND_LENGTH     5       // 发送数据每包的长度

INT8U   Cnt1ms = 0;             // 1ms计数变量，每1ms加一                
INT16U  RecvCnt = 0;            // 计数接收的数据包数                

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
    RF_Initial(RX);     // 初始化无线芯片,发送模式           
}

/*===========================================================================
* 函数 ：RF_RecvHandler() => 无线数据接收处理                               * 
============================================================================*/
void RF_RecvHandler(void)
{
    INT8U i=0, length=0, recv_buffer[10]={ 0 };
           
    if (0 == L01_IRQ_READ())                    // 检测无线模块是否产生接收中断 
    {
        if (L01_ReadIRQSource() & (1<<RX_DR))   // 检测无线模块是否接收到数据
        {
            // 数据请零，防止误判断
            for (i=0; i<SEND_LENGTH; i++)   { recv_buffer[0] = 0; } 
                
            // 读取接收到的数据长度和数据内容
            length = L01_ReadRXPayload(recv_buffer);
            
            // 判断接收数据是否正确
            if ((SEND_LENGTH==length) && (recv_buffer[0]=='1')\
            && (recv_buffer[1]=='2') && (recv_buffer[2]=='3'))
            {
                LED0_TOG();                     // LED闪烁，用于指示收到正确数据
                RecvCnt++;
                USART_Send("Receive ok\r\n", 12);
            }                
        }    
        
        L01_FlushRX();                          // 复位接收FIFO指针    
        L01_ClearIRQ(IRQ_ALL);                  // 清除中断            
    }
}

/*===========================================================================
* 函数 : main() => 主函数，程序入口                                         *
* 说明 ：如果接收到数据长度为5个字节，且前三个字节为“123”即数据接收正确     *
============================================================================*/
void main(void)
{
	System_Initial();       // 初始化系统所有外设               

	while (1)
	{
	    RF_RecvHandler();   // 无线数据接收处理               
	}
}

