#ifndef ARIAL10_H
#define ARIAL10_H

__rom unsigned char *arial10( const char c );
unsigned int arial10_puts(unsigned int x, unsigned int y, __rom char * s, unsigned char fg_color, unsigned char bg_color);
unsigned int arial10_getswidth(__rom char * s);

#endif


