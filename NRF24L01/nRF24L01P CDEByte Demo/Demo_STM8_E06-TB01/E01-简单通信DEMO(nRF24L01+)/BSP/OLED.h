/*===========================================================================
* 网址 ：http://yhmcu.taobao.com/   http://www.cdebyte.com/                 *
* 作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司                 * 
* 邮件 ：yihe_liyong@126.com                                                *
* 电话 ：18615799380                                                        *
============================================================================*/

#ifndef _OLED_H_
#define _OLED_H_

#include "STM8l10x_conf.h"
#include "mytypedef.h"

// OLED相关控制引脚定义，CMD(PD0), CSN(PB3), RST(PB2)
#define PORT_OLED_CMD   GPIOD
#define PIN_OLED_CMD    GPIO_Pin_0

#define PORT_OLED_CSN   GPIOB
#define PIN_OLED_CSN    GPIO_Pin_3

#define PORT_OLED_RST   GPIOB
#define PIN_OLED_RST    GPIO_Pin_2

// OLED操作函数
#define OLED_CSN_H()    GPIO_SetBits(PORT_OLED_CSN, PIN_OLED_CSN)
#define OLED_CSN_L()    GPIO_ResetBits(PORT_OLED_CSN, PIN_OLED_CSN)

#define OLED_CMD_H()    GPIO_SetBits(PORT_OLED_CMD, PIN_OLED_CMD)
#define OLED_CMD_L()    GPIO_ResetBits(PORT_OLED_CMD, PIN_OLED_CMD)

#define OLED_RST_H()    GPIO_SetBits(PORT_OLED_RST, PIN_OLED_RST)
#define OLED_RST_L()    GPIO_ResetBits(PORT_OLED_RST, PIN_OLED_RST)

// 相关函数声明
void LCD_Init(void);            // 初始化OLED
void LCD_Dis_Logo(void);        // 显示公司logo
void LCD_Dis_Logo1(void);       // 显示公司logo
void LCD_WrCmd(INT8U cmd);      // 写命令到OLED
void LCD_WrDat(INT8U dt);       // 写数据到OLED
void LCD_Fill(INT8U bmp_dat);   // 填充OLED

void LCD_Set_Pos(INT8U page, INT8U column);             // 定位光标到指定位置
void LCD_Dis_Char(INT8U page, INT8U column, char ch);   // 显示一个字符
void LCD_Dis_Str(INT8U page, INT8U column, char *str);  // 显示一串字符
void LCD_Dis_16x16(INT8U page, INT8U column, INT8U *p_data); // 显示一个汉字

#endif // _OLED_H_

/*===========================================================================
-----------------------------------文件结束----------------------------------
===========================================================================*/