NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                             lit   0026   067F =   1663. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0026  __lit_start
       0035  _Font8x16
       0625  _Our_Logo
       06A5  __lit_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   06A5   002D =     45. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       06A5  __idata_start
       06D2  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   06D2   0982 =   2434. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0369  __text_start
       0369  __start
       038A  _exit
       038B  _main
       0453  _SPI_ExchangeByte
       0458  _SPI_Initial
       0462  _GPIO_Initial
       0465  _L01_GetCEStatus
       0468  _L01_SetCE
       0472  _L01_ReadSingleReg
       047F  _L01_WriteSingleReg
       0492  _L01_WriteMultiReg
       04AF  _L01_FlushTX
       04B6  _L01_FlushRX
       04BD  _L01_ReuseTXPayload
       04C4  _L01_Nop
       04CB  _L01_ReadStatusReg
       04D5  _L01_ClearIRQ
       04E7  _L01_ReadIRQSource
       04EA  _L01_ReadTopFIFOWidth
       04F6  _L01_ReadRXPayload
       0515  _L01_WriteTXPayload_Ack
       0534  _L01_WriteTXPayload_NoAck
       054E  _L01_WriteRXPayload_InAck
       056C  _L01_SetTXAddr
       057F  _L01_SetRXAddr
       059C  _L01_SetSpeed
       05B4  _L01_SetPower
       05D6  _L01_WriteHoppingPoint
       05DE  _L01_IsTXEmpty
       05E2  _L01_SetTRMode
       05F9  _L01_SetPowerDown
       0603  _L01_SetPowerUp
       060D  _L01_Init
       0650  _LCD_Dis_16x16
       0673  _LCD_Dis_Logo
       06BC  _LCD_Set_Pos
       06D0  _LCD_Dis_Char
       0703  _LCD_Dis_Str
       0718  _LCD_WrCmd
       0724  _LCD_WrDat
       0730  _LCD_Fill
       0746  _LCD_Init
       0792  _strlen
       079E  mod16u
       07A0  div16u
       07A1  xdiv16u
       07BA  pop_xgset30FC
       07C3  pop_xgsetF0FC
       07CE  push_xgsetF0FC
       07D0  push_xgset30FC
       07D2  push_xgset00FC
       07D4  push_xgset003C
       07D9  push_xgset300C
       07DE  pop_xgset300C
       07E3  push_xgsetF00C
       07EA  pop_xgsetF00C
       07F1  push_xgset303C
       07F8  pop_xgset303C
       07FF  push_xgsetF03C
       0808  pop_xgsetF03C
       0811  asgncblk
       082A  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0060   002D =     45. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0060  __data_start
       008C  _CE_Status
       008D  __data_end

Area                               Start  End    Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0000   0002 =      2. bytes (abs,ovr,rom)

Files Linked      [ module(s) ]

C:\iccv7avr\lib\crtAVR.o	[ crtavr.s ]
main.o	[ main.c ]
board.o	[ board.c ]
nRF24L01.o	[ nRF24L01.c ]
OLED.o	[ OLED.c ]
<library>	[ strlen.s, div16u.s, gpop4.s, gpop5.s, gpush.s, gpushx.s, gpushz.s, gpusha.s, gpushb.s, pmstruct.s ]

User Global Definitions

ram_end = 0x45f
hwstk_size = 0x1e

User Base Address Definitions

func_lit = 0x26
data = 0x60
eeprom:0.512

