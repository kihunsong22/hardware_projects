/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Version$   $Date$
|*
|*  IN PACKAGE:         avi parser
|*
|*  COPYRIGHT:          Copyright (c) 2005, Altium
|*
|*  DESCRIPTION:        AVI file parsing
|*
\*****************************************************************************/

#ifndef AVIPARSE_H_
#define AVIPARSE_H_

#include <stdint.h>

#define VIDEOMODE_RGB555  1
#define VIDEOMODE_RGB565  2
#define VIDEOMODE_RGB888  3

typedef struct
{
    struct
    {
        uint32_t microsecperframe;
        uint32_t height;
        uint32_t width;
        uint8_t mode;
    } video;

    struct
    {
        uint32_t initialframes;
        uint16_t channels;
        uint32_t samplespersec;
        uint16_t bitspersample;
    } audio;
} AVIFORMAT;


int avi_parse(AVIFORMAT *format, uint32_t *avbuf, int avbufsize,
              int (*input)(uint32_t *buf, int size),
              int (*setup)(void),
              int (*videoprocess)(uint32_t **bufp, int size),
              int (*audioprocess)(uint32_t **bufp, int size));

#endif
