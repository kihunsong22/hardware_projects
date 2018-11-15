CC = iccavr
LIB = ilibw
CFLAGS =  -IF:\产品\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Project\TX -IF:\产品\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Project\TX -IF:\产品\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source -e -D__ICC_VERSION=722 -DATMega8  -l -g -MHasMul -MEnhanced -Wa-W -Wf-const_is_flash -DCONST="" 
ASFLAGS = $(CFLAGS) 
LFLAGS =  -g -e:0x2000 -Wl-W -bfunc_lit:0x26.0x2000 -dram_end:0x45f -bdata:0x60.0x45f -dhwstk_size:30 -beeprom:0.512 -fihx_coff -S2
FILES = main.o board.o nRF24L01.o OLED.o 

L01_TX:	$(FILES)
	$(CC) -o L01_TX $(LFLAGS) @L01_TX.lk  
main.o: .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\MyTypedef.h C:\iccv7avr\include\stdio.h C:\iccv7avr\include\stdarg.h C:\iccv7avr\include\_const.h C:\iccv7avr\include\stdlib.h C:\iccv7avr\include\limits.h C:\iccv7avr\include\string.h C:\iccv7avr\include\math.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01_Reg.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.h C:\iccv7avr\include\iom8v.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\mytypedef.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\Oled.h
main.o:	..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Project\TX\main.c
	$(CC) -c $(CFLAGS) ..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Project\TX\main.c
board.o: .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.h C:\iccv7avr\include\iom8v.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\mytypedef.h C:\iccv7avr\include\stdio.h C:\iccv7avr\include\stdarg.h C:\iccv7avr\include\_const.h C:\iccv7avr\include\stdlib.h C:\iccv7avr\include\limits.h C:\iccv7avr\include\string.h C:\iccv7avr\include\math.h
board.o:	..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.c
	$(CC) -c $(CFLAGS) ..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.c
nRF24L01.o: .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\MyTypedef.h C:\iccv7avr\include\stdio.h C:\iccv7avr\include\stdarg.h C:\iccv7avr\include\_const.h C:\iccv7avr\include\stdlib.h C:\iccv7avr\include\limits.h C:\iccv7avr\include\string.h C:\iccv7avr\include\math.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01_Reg.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.h C:\iccv7avr\include\iom8v.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\mytypedef.h
nRF24L01.o:	..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01.c
	$(CC) -c $(CFLAGS) ..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\nRF24L01.c
OLED.o: .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\OLED.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\mytypedef.h C:\iccv7avr\include\stdio.h C:\iccv7avr\include\stdarg.h C:\iccv7avr\include\_const.h C:\iccv7avr\include\stdlib.h C:\iccv7avr\include\limits.h C:\iccv7avr\include\string.h C:\iccv7avr\include\math.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\board.h C:\iccv7avr\include\iom8v.h .\..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\font.h
OLED.o:	..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\OLED.c
	$(CC) -c $(CFLAGS) ..\..\..\..\..\..\..\ODMPRO~1\E06-RFTB\E06-AVR\DEMO_AVR\nRF24L01P\Source\OLED.c
