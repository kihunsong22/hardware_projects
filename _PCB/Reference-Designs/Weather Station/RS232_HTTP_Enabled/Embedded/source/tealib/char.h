// character set


__rom unsigned char ariala[] = 
    {/* width in pixels */  16,
     /* height in pixels */ 16,
     /* data */ 0x00, 0x00,
                0x00, 0x00,
                0x03, 0xE0,
                0x0, 0x10,
                0x08, 0x08,
                0x11, 0xC4,
                0x12, 0x44,
                0x22, 0x44,
                0x22, 0x44,
                0x22, 0x48,
                0x21, 0xB0,
                0x10, 0x00,
                0x10, 0x00,
                0x08, 0x40,
                0x07, 0x80,
                0x00, 0x00 };
                
     
//     0x99, 0x42, 0x24, 0x99, 0x99, 0x24, 0x42, 0x99 };

__rom unsigned char arialb[] = 
    {/* width in pixels */  8,
     /* height in pixels */ 8,
     /* data */ 0x38, 0x24, 0x24, 0x38, 0x24, 0x24, 0x24, 0x38 };

__rom unsigned char arialc[] = 
    {/* width in pixels */  8,
     /* height in pixels */ 8,
     /* data */ 0x1C, 0x22, 0x20, 0x20, 0x20, 0x20, 0x22, 0x1C };

unsigned char __rom * __rom arial[] =
    {/* A */ &ariala[0],
     /* B */ &arialb[0],
     /* C */ &arialc[0] };
     

