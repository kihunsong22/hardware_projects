/*
 * videomux.h --
 *
 *     This header file holds declarations of routines and
 *     data types used in the video multiplexer project.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#ifndef _VIDEOMUX_H
#define _VIDEOMUX_H

#include    "i2c.h"

/* I2C slave addresses. */
enum {
    Encoder = 0x88,
    DecoderA = 0x48,
    DecoderB = 0x4A
};

typedef enum {
    None,
    KeyPress,
    KeyRelease
} Event;

typedef uchar KeyNum;

/* Event manipulation routines from event.c. */
extern void      EventInit(void);
extern void      PostEvent(Event action, KeyNum key);
extern Event     FetchEvent(KeyNum *keyPtr);
extern void      KeyScan(void);

/* Configuration routines from config.c. */
extern int       EncoderInit(uchar dev);
extern int       DecoderInit(uchar dev);
extern void      UpdatePicture(void);
extern void      UpdatePip(void);
extern void      SetChannel(KeyNum n);
extern void      SetPip(KeyNum n);

#endif /* _VIDEOMUX_H */

