/* Common source part of the timer implementation */

static __data unsigned int tick_down;
static __data unsigned int reload_ticks;    /* number of ticks at which the tmr_ticks must be reloaded */
#if TIMER_SIZE == 4
__data volatile TMR_TIMER system_time;
#else
__data TMR_TIMER system_time;
#endif

void TMR_IDLE( void );


/*****************************************************************************
 *
 *  FUNCTION:   getclock
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: none
 *
 *  RETURN VALUE:   TMR_TIMER (long or int, depending on the typedef of TMR_TIMER)
 *
 *  DESCRIPTION:    get the current clock value
 *          This function uses inline asm to speed up the code
 */
TMR_TIMER getclock( void )
{
    TMR_TIMER ret;


#if TIMER_SIZE == 4
    do
    {
        ret = tmr0_system_time;
    }
    while ( ret < ( tmr0_system_time - 2 ));
#else


__asm(
"again: \n"
#if 0
//#if TIMER_SIZE == 4
    /* load timer value */
    " MOV   %0.0,%1       \n"
    " MOV   %0.1,%1+1     \n"
    " MOV   %0.2,%1+2     \n"
    " MOV   %0.3,%1+3     \n"

    /* 
     * Check if value is changed since last load.
     * If difference in LSB greater then 1, load again.
     * Other bytes must be the same, this means that if the
     * last byte overfows, the test fails.
     * Loading the value again in this case is much faster
     * than calculating and checking the real difference
     * between 2 long-ints.
     */
    " CLR   C           \n"
    " MOV   A,%1        \n"
    " SUBB  A,%0.0      \n"
    " JNZ   again       \n"
    " MOV   A,%1+1      \n"
    " SUBB  A,%0.1      \n"
    " JNZ   again       \n"
    " MOV   A,%1+2      \n"
    " SUBB  A,%0.2      \n"
    " JNZ   again       \n"
    " MOV   A,%1+3      \n"
    " SUBB  A,%0.3      \n"
    " JZ    ok          \n"
    " DEC   A           \n"
    " JNZ   again       \n"
#else
    /* load timer value */
    " MOV   %0.0,%1     \n"
    " MOV   %0.1,%1+1   \n"

    /* 
     * Check if value is changed since last load.
     * If difference in LSB greater then 1, load again.
     * Other bytes must be the same, this means that if the
     * last byte overfows, the test fails.
     * Loading the value again in this case is faster
     * than calculating and checking the real difference
     * between 2 ints.
     */
    " CLR   C           \n"
    " MOV   A,%1        \n"
    " SUBB  A,%0.0      \n"
    " JNZ   again       \n"
    " MOV   A,%1+1      \n"
    " SUBB  A,%0.1      \n"
    " JZ    ok          \n"
    " DEC   A           \n"
    " JNZ   again       \n"
#endif
"ok:"
    : "=R" (ret)
    : "m" ( system_time )
    : );
#endif
    return ret;
}

    
/*****************************************************************************
 *
 *  FUNCTION:   delay
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: TMR_TIMER ticks: delay in timer ticks
 *
 *  RETURN VALUE:   none
 *
 *  DESCRIPTION:    wait for the given ticks
 *          if TMR_IDLE is defined, call the idle function repeatedly
 *          during the delay
 */
void delay( TMR_TIMER ticks )
{
    TMR_TIMER expire_time;
    expire_time = getclock() + ticks;
    while (( getclock() - expire_time ) < 0 )   /* using signed arithm. corrects automatically for overlow */
    {
#ifdef TMR_IDLE
        TMR_IDLE();
#endif
    }
}


/*****************************************************************************
 *
 *  FUNCTION:   interrupt
 *
 *  AVAILABILITY:   INTERRUPT
 *
 *  PARAMETERS: none
 *
 *  RETURN VALUE:   none
 *
 *  DESCRIPTION:    adjust the clock
 *          This function uses inline asm to speed up the code
 */
__interrupt( TMR_INT ) __frame( A, PSW ) void interrupt( void )
{
/*
 * Written in asm to avoid long library calls. This speeds up the interrupt.
 * 'C' takes 2 times more time then asm.
 */
#if TIMER_SIZE == 4
    tick_down--;
    if ( tick_down == 0 )
    {
      system_time++;  
      tick_down = reload_ticks;
    }

#else
__asm(

    // tick_down--;
    " MOV   A,%2+1      \n"
    " JNZ   down1       \n"
    " DEC   %2      \n"
"down1:             \n"
    " DEC   %2+1        \n"

    // if ( tick_down == 0 )
    " MOV   A,%2+1      \n"
    " ORL   A,%2        \n"
    " JNZ   skip        \n"

    //{
    //  system_time++;  
    " MOV   A,%1+1      \n"
    " ADD   A,#1        \n"
    " MOV   %1+1,A      \n"
    " MOV   A,%1        \n"
    " ADDC  A,#0        \n"
    " MOV   %1,A        \n"

    //  tick_down = reload_ticks;
    " MOV   %2,%0       \n"
    " MOV   %2+1,%0+1   \n"

    //}
"skip:              \n"
    :
    : "m"( reload_ticks ), "m"( system_time ), "m"( tick_down ) 
    : "A" );
#endif
}


