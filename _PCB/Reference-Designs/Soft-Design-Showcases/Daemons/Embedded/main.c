/*****************************************************************************
|*  SwPlatform Daemons Example
|*
|* Devices:
|*  - LEDs and switches (WB_PRTIO)
|*  - TFT screen (VGA32_TFT)
|*  - JPEG decoder (WB_JPGDEC)
|*  - SD card (SPI_W + SD_CARD)
|*  - UART8 serial device (WB_UART8 )
|*  - PS2 controller      ( )
|*
|* Services used:
|*  - FAT filesystem (fs_fat)
|*  - POSIX_DEVIO (serial, keyboard)
|*  - MULTITHREADING
|*
|* Description:
|*      This example shows how users can configure background threads to execute 
|*      concurrently to the main thread's activities.
|*
|*      By the time the kernel reaches main() all the background threads
|*      will be ready to execute (the 'how' depends on their related
|*      configurations for priority and policies).
|*
|*      The main() thread must always exist. In this example, however,
|*      this main thread immediately exits and the action continues in the
|*      configured background threads.
|*    
|*      slideshow_thread: the full slideshow showcase example as a thread
|*                        Prio 10, FIFO
|*                        Always eligible to do processing, i.e. it nevers goes
|*                        to waiting/sleep.
|*
|*      echo_thread:      this thread echoes to stdout every sc
|*                        Prio 20, FIFO
|*                        Almost always on waiting (timeout event).
|*                        Preempts slideshow.
|*
|*      leds_thread:      this thread flashes some leds every sc
|*                        Prio 21, FIFO
|*                        Almost always on waiting (timeout event).
|*                        Preempts slideshow and echo.
|*
|*      The example is called "Daemons" because of the analogy between background
|*      threads and daemon processes in Linux/Unix systems.
|*
\*****************************************************************************/

#include <stdlib.h>
         
void* main( void* argc )
{
    (void)argc;
    
     // as explained, there is nothing to do in main thread: all happens in daemons
     return NULL;
}

