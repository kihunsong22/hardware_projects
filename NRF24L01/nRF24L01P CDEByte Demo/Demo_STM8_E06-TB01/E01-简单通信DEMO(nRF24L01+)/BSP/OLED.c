/*===========================================================================
* 网址 ：http://yhmcu.taobao.com/   http://www.cdebyte.com/                 *
* 作者 ：李勇  原 亿和电子工作室  现 亿佰特电子科技有限公司                 * 
* 邮件 ：yihe_liyong@126.com                                                *
* 电话 ：18615799380                                                        *
============================================================================*/

#include "OLED.h"
#include "font.h"

extern INT8U SPI_ExchangeByte(INT8U input); // 通过SPI进行数据交换,见bsp.c

/*===========================================================================
* 函数 ：LCD_Dis_16x16() => 显示一个16X16的字符（汉字）                     * 
* 输入 ：page（0-7），column（0-127）表示显示位置，p_data指向需要显示的内容 * 
============================================================================*/
void LCD_Dis_16x16(INT8U page, INT8U column, INT8U *p_data)
{
    INT8U i;
    
    // 显示第一页的数据
    LCD_Set_Pos(page, column);
    for (i=0; i<16; i++)    { LCD_WrDat(*p_data++); }
    
    // 显示第二页的数据
    LCD_Set_Pos(page+1, column);
    for (i=0; i<16; i++)    { LCD_WrDat(*p_data++); }
}

/*===========================================================================
* 函数 ：LCD_Dis_Logo() => 显示公司logo                                     * 
* 说明 ：显示在第一行，显示内容为”亿和电子“                                 * 
============================================================================*/
void LCD_Dis_Logo(void)
{
    INT8U i = 0;
    
    LCD_Dis_16x16(0, i*16+32, (INT8U *)&Our_Logo[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+32, (INT8U *)&Our_Logo[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+32, (INT8U *)&Our_Logo[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+32, (INT8U *)&Our_Logo[32*i]);
}

/*===========================================================================
* 函数 ：LCD_Dis_Logo1() => 显示公司logo                                     * 
* 说明 ：显示在第一行，显示内容为”成都亿佰特“                               * 
============================================================================*/
void LCD_Dis_Logo1(void)
{
    INT8U i = 0;
    
    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
    i++;
//    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
    i++;
    LCD_Dis_16x16(0, i*16+16, (INT8U *)&Our_Logo1[32*i]);
}

/*===========================================================================
* 函数 ：LCD_Set_Pos() => 定位光标到指定位置                                * 
* 输入 ：page（0-7），column（0-127）表示光标的目标位置                     * 
============================================================================*/
void LCD_Set_Pos(INT8U page, INT8U column)
{
    LCD_WrCmd(0xb0 + (page&0x07));
    LCD_WrCmd(0x10 + ((column>>4)&0x0F));
    LCD_WrCmd(column & 0x0F);
}

/*===========================================================================
* 函数 ：LCD_Dis_Char() => 显示一个字符                                     * 
* 输入 ：page（0-7），column（0-127）表示显示位置，ch需要显示的字符         * 
============================================================================*/
void LCD_Dis_Char(INT8U page, INT8U column, char ch)
{
    INT8U   j = 0;
    INT16U  pos = 0;
    
    if (ch < ' ')           { return; }

    pos = 16 * (ch-' ');    
    LCD_Set_Pos(page, column);
    for (j=0; j<8; j++)     { LCD_WrDat(Font8x16[pos++]); }    
    LCD_Set_Pos(page+1, column);
    for (j=0; j<8; j++)     { LCD_WrDat(Font8x16[pos++]); }
}

/*===========================================================================
* 函数 ：LCD_Dis_Str() => 显示一串字符                                      * 
* 输入 ：page（0-7），column（0-127）表示显示位置，str指向显示的字符串      * 
============================================================================*/
void LCD_Dis_Str(INT8U page, INT8U column, char *str)
{
    while (*str)
    {
        LCD_Dis_Char(page, column, *str++);
        column += 8;
    }
}

/*===========================================================================
* 函数 ：LCD_WrCmd() => 写命令到OLED                                        * 
* 输入 ：cmd表示需要写入的命令                                              * 
============================================================================*/
void LCD_WrCmd(INT8U cmd)
{
    OLED_CSN_L();
    OLED_CMD_L();
    SPI_ExchangeByte(cmd);
    OLED_CSN_H();
}

/*===========================================================================
* 函数 ：LCD_WrDat() => 写数据到OLED                                        * 
* 输入 ：dt表示需要写入的数据                                               * 
============================================================================*/
void LCD_WrDat(INT8U dt)
{
    OLED_CSN_L();
    OLED_CMD_H();
    SPI_ExchangeByte(dt);
    OLED_CSN_H();
}

/*===========================================================================
* 函数 ：LCD_Fill() => 填充OLED                                             * 
* 输入 ：bmp_dat表示需要填充的数据                                          * 
============================================================================*/
void LCD_Fill(INT8U bmp_dat)
{
    INT8U y, x;
    
    for (y=0; y<8; y++)
    {
        LCD_WrCmd(0xb0 + y);
        LCD_WrCmd(0x01);
        LCD_WrCmd(0x10);
        for (x=0; x<128; x++)
        LCD_WrDat(bmp_dat);
    }
}

/*===========================================================================
* 函数 ：LCD_Init() => 初始化OLED                                           * 
============================================================================*/
void LCD_Init(void)
{
    INT16U x = 0;
    
    
    OLED_RST_L();               // 复位OLED
    for (x=0; x<1000; x++);     // 延时，等待OLED复位完成
    OLED_RST_H();
    
    LCD_WrCmd(0xae);    // 关闭OLED面板
    LCD_WrCmd(0x00);    // set low column address
    LCD_WrCmd(0x10);    // set high column address
    LCD_WrCmd(0x40);    // Set Mapping RAM Display Start Line (0x00~0x3F)
    LCD_WrCmd(0x81);    // set contrast control register
    LCD_WrCmd(0xcf);    // Set SEG Output Current Brightness
    LCD_WrCmd(0xa1);    // Set SEG/Column Mapping       0xa0左右反置 0xa1正常
    LCD_WrCmd(0xc8);    // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    LCD_WrCmd(0xa6);    // set normal display
    LCD_WrCmd(0xa8);    // set multiplex ratio(1 to 64)
    LCD_WrCmd(0x3f);    // 1/64 duty
    LCD_WrCmd(0xd3);    // set display offset Shift Mapping RAM Counter
    LCD_WrCmd(0x00);    // not offset
    LCD_WrCmd(0xd5);    // set display clock divide ratio/oscillator frequency
    LCD_WrCmd(0x80);    // set divide ratio, Set Clock as 100 Frames/Sec
    LCD_WrCmd(0xd9);    // set pre-charge period
    LCD_WrCmd(0xf1);    // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    LCD_WrCmd(0xda);    // set com pins hardware configuration
    LCD_WrCmd(0x12);
    LCD_WrCmd(0xdb);    // set vcomh
    LCD_WrCmd(0x40);    // Set VCOM Deselect Level
    LCD_WrCmd(0x20);    // Set Page Addressing Mode (0x00/0x01/0x02)
    LCD_WrCmd(0x02);    //
    LCD_WrCmd(0x8d);    // set Charge Pump enable/disable
    LCD_WrCmd(0x14);    // set(0x10) disable
    LCD_WrCmd(0xa4);    // Disable Entire Display On (0xa4/0xa5)
    LCD_WrCmd(0xa6);    // Disable Inverse Display On (0xa6/a7)
    LCD_WrCmd(0xaf);    // turn on oled panel
    LCD_Fill(0x00);     // 初始清屏
}

/*===========================================================================
-----------------------------------文件结束----------------------------------
===========================================================================*/
