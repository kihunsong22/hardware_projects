/*
 * main.c --
 *
 *     This file is the main program for the Video Multiplexer.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include "videomux.h"

/* isr: Service routine called at end of vertical blanking period. */
void __interrupt(__INTNO(0))
isr0()
{
    UpdatePicture();
    KeyScan();
}

void __interrupt(__INTNO(2))
isr2()
{
    UpdatePip();
}

/* ClearLEDs: Turn off the LEDs next to every key. */
void
ClearLEDs(void)
{
    /* The LED controls are active low. */
    P0 = 0xFF;
}

/* SetLED: Light the LED next to key 0, 1, 2 or 3. */
void
SetLED(KeyNum n)
{
    P0 &= ~(1 << n);
}

/* main: Initialise the hardware and handle user interactions. */
int
main()
{
    Event   e;
    KeyNum  key, first = 0, second = 0;
    enum {
        Start, OneDown, TwoDown, OneUp
    } state;

    I2cInit();
    (void)DecoderInit(DecoderA);
    (void)DecoderInit(DecoderB);
    (void)EncoderInit(Encoder);
#ifdef COLOURBAR
    /* Only enable this to verify that the encoder is working. */
    (void)I2cPoke(Encoder, 0x3A, 0x93);
#endif
    ClearLEDs();
    SetLED(0);
    EventInit();
    IT0 = 1; /* set INT0 and INT1 edge-triggered */
    IT1 = 1;
    EX0 = 1; /* enable INT0 and INT1 */
    EX1 = 1;
    EA = 1; /* enable interrupts */
    for (state = Start;;) {
        e = FetchEvent(&key);
        if (e == None) {
            continue;
        }
        switch (state) {
        case Start:
            if (e == KeyPress) {
                /* First key pressed selects the main picture. */
                first = key;
                state = OneDown;
                ClearLEDs();
                SetLED(first);
                SetChannel(first);
            }
            break;
        case OneDown:
            if ((e == KeyRelease) && (key == first)) {
                state = Start;
            } else if ((e == KeyPress) && (key != first)) {
                /* Second, while first held, selects picture-in-picture. */
                second = key;
                state = TwoDown;
                ClearLEDs();
                SetLED(first);
                SetLED(second);
                SetPip(second);
            }
            break;
        case TwoDown:
            if (e == KeyRelease) {
                if (key == first) {
                    /* Wait until second key is released. */
                    state = OneUp;
                } else if (key == second) {
                    /* Ready to select a new picture-in-picture. */
                    state = OneDown;
                }
            }
            break;
        case OneUp:
            if ((e == KeyRelease) && (key == second)) {
                /* Can start new selection once both keys are released. */
                state = Start;
            }
            break;
        default:
            state = Start;
            break;
        }
    }
}

