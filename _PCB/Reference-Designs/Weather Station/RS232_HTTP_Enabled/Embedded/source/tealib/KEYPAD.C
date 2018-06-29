#include "keypad.h"


unsigned char keypad( void )
{
    unsigned char   ret_val = 0;
       
    if ( KEY_VALID )
    {
        ret_val = (( KEY_IN & ( 0x0F << KEY_BIT0 )) >> KEY_BIT0 ) + 1;
        
        KEY_RESET = 1;
    }
        KEY_RESET = 0;


    return ret_val;
}
