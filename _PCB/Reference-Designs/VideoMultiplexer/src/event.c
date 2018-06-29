/*
 * event.c --
 *
 *     This file implements an event queue for
 *     the video multiplexer project. It also provides
 *     a routine that polls the status of the keypad
 *     and adds events to the queue appropriately.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include    "videomux.h"

enum {
    QSIZE = 8
};

static struct {
    Event      action;
    KeyNum     key;
} queue[QSIZE];

static uchar head, tail;

/* EventInit: Set up an initially empty event queue. */
void
EventInit(void)
{
    head = 0;
    tail = 0;
}

/* PostEvent: Add an event to the tail of the queue. */
void
PostEvent(Event action, KeyNum key)
{
    uchar next;

    next = tail + 1;
    if (next >= QSIZE) {
        next = 0;
    }
    if (next == head) {
        /* The queue is full, so discard the event. */
        return;
    }
    queue[tail].action = action;
    queue[tail].key = key;
    tail = next;
}

/* FetchEvent: Return the next event from the queue or "None" if empty. */
Event
FetchEvent(KeyNum *keyPtr)
{
    uchar    next;
    Event    action;

    if (head == tail) {
        return None;
    }
    next = head + 1;
    if (next >= QSIZE) {
        next = 0;
    }
    action = queue[head].action;
    *keyPtr = queue[head].key;
    head = next;
    return action;
}

/* KeyScan: Post an event if a key changes state. Call every ~20ms. */
void
KeyScan(void)
{
    static uchar     down[4];  /* True if switch is closed. */
    static uchar     count[4]; /* No. scans state has been stable. */
    uchar            pressed;
    KeyNum           key;

    for (key = 0; key < 4; key++) {
        pressed = ((P1 & (1 << key)) == 0);
        if (down[key]) {
            if (!pressed) {
                if (++count[key] == 3) {
                    PostEvent(KeyRelease, key);
                    down[key] = 0;
                } else {
                    continue;
                }
            }
        } else {
            if (pressed) {
                if (++count[key] == 3) {
                    PostEvent(KeyPress, key);
                    down[key] = 1;
                } else {
                    continue;
                }
            }
        }
        count[key] = 0;
    }
}


