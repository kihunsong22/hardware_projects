#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "fade.h"

//-------------------------------------------------------------------
// Copy from SSAS sysutils plugin
//-------------------------------------------------------------------
inline uint16_t big16( uint16_t val )
{
#ifdef __BIG_ENDIAN__
    return val;
#else
    return (val << 8) | (val >> 8);
#endif
}

inline uint16_t read_big16from16( const void * ptr )
{
    return big16( *(uint16_t *)ptr );
}
//-------------------------------------------------------------------

/********************************************************************
|*
|*  FUNCTION    : fade
|*
|*  PARAMETERS  : from = image to fade from
|*                to = image to fade to
|*                dest = area to write copy to
|*                step = fade value (0..255)
|*
|*  RETURNS     : None
|*
|*  DESCRIPTION : Initialize hardware
 */

void fade( uint16_t * img1, uint16_t * img2, uint16_t * dest, uint16_t step, uint32_t pixels )
{
    while( pixels-- )
    {
        uint16_t a, b, c1, c2, c3;
        a = read_big16from16( &img1[pixels] );
        b = read_big16from16( &img2[pixels] );
        c1 = ((a & 0x1F) * (256 - step)) / 256 + ((b & 0x1F) * step) / 256;
        a >>= 5;
        b >>= 5;
        c2 = ((a & 0x3F) * (256 - step)) / 256 + ((b & 0x3F) * step) / 256;
        a >>= 6;
        b >>= 6;
        c3 = ((a & 0x1F) * (256 - step)) / 256 + ((b & 0x1F) * step) / 256;
        dest[pixels] = big16( (c3 << 11) | (c2 << 5) | c1 );
    }
}

