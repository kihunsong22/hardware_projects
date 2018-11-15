NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                             lit   0026   0695 =   1685. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0026  __lit_start
       004B  _Font8x16
       063B  _Our_Logo
       06BB  __lit_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   06BB   0028 =     40. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       06BB  __idata_start
       06E3  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   06E4   0944 =   2372. bytes (rel,con,rom)

       Addr  Global Symbol
      -----  --------------------------------
       0372  __text_start
       0372  __start
       0393  _exit
       0394  _SPI_ExchangeByte
       0399  _SPI_Initial
       03A3  _GPIO_Initial
       03A6  _main
       045B  _L01_GetCEStatus
       045E  _L01_SetCE
       0468  _L01_ReadSingleReg
       0475  _L01_WriteSingleReg
       0488  _L01_WriteMultiReg
       04A5  _L01_FlushTX
       04AC  _L01_FlushRX
       04B3  _L01_ReuseTXPayload
       04BA  _L01_Nop
       04C1  _L01_ReadStatusReg
       04CB  _L01_ClearIRQ
       04DD  _L01_ReadIRQSource
       04E0  _L01_ReadTopFIFOWidth
       04EC  _L01_ReadRXPayload
       050B  _L01_WriteTXPayload_Ack
       052A  _L01_WriteTXPayload_NoAck
       0544  _L01_WriteRXPayload_InAck
       0562  _L01_SetTXAddr
       0575  _L01_SetRXAddr
       0592  _L01_SetSpeed
       05AA  _L01_SetPower
       05CC  _L01_WriteHoppingPoint
       05D4  _L01_IsTXEmpty
       05D8  _L01_SetTRMode
       05EF  _L01_SetPowerDown
       05F9  _L01_SetPowerUp
       0603  _L01_Init
       0646  _LCD_Dis_16x16
       0669  _LCD_Dis_Logo
       06B2  _LCD_Set_Pos
       06C6  _LCD_Dis_Char
       06F9  _LCD_Dis_Str
       070E  _LCD_WrCmd
       071A  _LCD_WrDat
       0726  _LCD_Fill
       073C  _LCD_Init
       0788  mod16u
       078A  div16u
       078B  xdiv16u
       07A4  pop_xgset30FC
       07AD  pop_xgsetF0FC
       07B8  push_xgsetF0FC
       07BA  push_xgset30FC
       07BC  push_xgset00FC
       07BE  push_xgset003C
       07C3  push_xgset300C
       07C8  pop_xgset300C
       07CD  push_xgsetF00C
       07D4  pop_xgsetF00C
       07DB  push_xgset303C
       07E2  pop_xgset303C
       07E9  push_xgsetF03C
       07F2  pop_xgsetF03C
       07FB  asgncblk
       0814  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0060   0028 =     40. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0060  __data_start
       0087  _CE_Status
       0088  __data_end

Area                               Start  End    Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0000   0002 =      2. bytes (abs,ovr,rom)

Files Linked      [ module(s) ]

C:\iccv7avr\lib\crtAVR.o	[ crtavr.s ]
board.o	[ board.c ]
main.o	[ main.c ]
nRF24L01.o	[ nRF24L01.c ]
OLED.o	[ OLED.c ]
<library>	[ div16u.s, gpop4.s, gpop5.s, gpush.s, gpushx.s, gpushz.s, gpusha.s, gpushb.s, pmstruct.s ]

User Global Definitions

ram_end = 0x45f
hwstk_size = 0x1e

User Base Address Definitions

func_lit = 0x26
data = 0x60
eeprom:0.512

