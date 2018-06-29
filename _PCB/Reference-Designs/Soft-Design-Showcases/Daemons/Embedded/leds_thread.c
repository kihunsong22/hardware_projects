#include <stdint.h>
#include <per_ioport.h>
#include <time.h>
#include <pthread.h>

#include "devices.h" // device IDs

#define  LED_FLASH_MASK                 0x1

void* leds_thread( void* argc )
{
    volatile uint8_t    *ba_leds;
    if ( ba_leds = (void *)per_ioport_get_base_address(PRTIO), !ba_leds )
    {
       return NULL;
    }
    *ba_leds = 0x0;
    while(1)
    {
            struct timespec ts = { 1, 0 };
            if ( *ba_leds & LED_FLASH_MASK )
            {
               *ba_leds &= ~LED_FLASH_MASK;
            }
            else
            {
               *ba_leds |= LED_FLASH_MASK;
            }
            nanosleep(&ts,NULL);
    }
    return NULL;
}



