#ifndef __KEYPAD_H
#define __KEYPAD_H

#define KEY_IN      P1
#define KEY_BIT0    0
#define KEY_VALID   P1_7
#define KEY_RESET   P1_0

extern unsigned short keypad( void );

#endif // __KEYPAD_H
