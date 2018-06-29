/*****************************************************************************\
|*
|*  VERSION CONTROL:    $Version$   $Date$
|*
|*  IN PACKAGE:
|*
|*  COPYRIGHT:          Copyright (c) 2005, Altium
|*
|*  DESCRIPTION:        AVI file parsing
|*
\*****************************************************************************/

//------------------------------------------------------------

// if defined uses printf to dump header info about AVI file
//#define AVIDUMPINFO

//------------------------------------------------------------

#ifdef AVIDUMPINFO
#include <stdio.h>
#endif

#include <unistd.h>
#include <string.h>

#include "aviparse.h"
#include "util_endian.h"

//------------------------------------------------------------

typedef struct
{
    uint32_t dwMicroSecPerFrame;
    uint32_t dwMaxBytesPerSec;
    uint32_t dwPaddingGranularity;
    uint32_t dwFlags;
    uint32_t dwTotalFrames;
    uint32_t dwInitialFrames;
    uint32_t dwStreams;
    uint32_t dwSuggestedBufferSize;
    uint32_t dwWidth;
    uint32_t dwHeight;
    uint32_t dwReserved[4];
} AVIMAINHEADER;


typedef struct
{
     uint32_t fccType;
     uint32_t fccHandler;
     uint32_t dwFlags;
     int16_t wPriority;
     int16_t wLanguage;
     uint32_t dwInitialFrames;
     uint32_t dwScale;
     uint32_t dwRate;
     uint32_t dwStart;
     uint32_t dwLength;
     uint32_t dwSuggestedBufferSize;
     uint32_t dwQuality;
     uint32_t dwSampleSize;
     struct
     {
         uint16_t left;
         uint16_t top;
         uint16_t right;
         uint16_t bottom;
     } rcFrame;
} AVISTREAMHEADER;


typedef struct
{
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t biXPelsPerMeter;
  int32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;


typedef struct
{
  uint16_t wFormatTag;
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
//  uint16_t cbSize;
} WAVEFORMATEX;

//------------------------------------------------------------

#define AUDIO_FORMAT_PCM 1

#define BI_RGB       0x00000000
#define BI_BITFIELDS 0x00000003

//------------------------------------------------------------

/*********************************************************************
* FUNCTION: avi_parse
*
* parse given AVI file
*
* format             after setup has been called contains details of video & audio stream in AVI file
* avbuf              buffer to use for audio & video frames
* avbufsize          size of avbuf
* input              callback to datasupplying function
* setup              calback to setup video & audio formats
*                    returns     if <>0 avi processing will be aborted and value wil be returned
* videoprocess       callback to process single videoframe
*                    returns     if <>0 avi processing will be aborted and value wil be returned
* audioprocess       callback to process single videoframe
*                    size        bytes writtin in buf
*                    returns     if <>0 avi processing will be aborted and value wil be returned
*
* returns            0 success, <>0 error (-1 for internal errors, returned value for aborts by callbacks)
*/
int avi_parse(AVIFORMAT *format, uint32_t *avbuf, int avbufsize,
              int (*input)(uint32_t *buf, int size),
              int (*setup)(void),
              int (*videoprocess)(uint32_t **bufp, int size),
              int (*audioprocess)(uint32_t **bufp, int size))
{
    AVIMAINHEADER avimainheader;
    uint32_t buf[1];
    int32_t filesize;
    int32_t listsize;
    int32_t chunksize;
    int retcode;

    memset(format, 0, sizeof(AVIFORMAT));

    // RIFF AVI header

    input(buf, 4);
    if (strncmp((char*) buf, "RIFF", 4))
    { return -1; }

    input(buf, 4); filesize = read_little32from8((uint8_t*) buf);

    input(buf, 4);
    filesize -= 4;
    if (strncmp((char*) buf, "AVI ", 4))
    { return -1; }

    // LIST hdrl with stream definitions

    input(buf, 4);
    filesize -= 4;
    if (strncmp((char*) buf, "LIST", 4))
    { return -1; }

    input(buf, 4); listsize = read_little32from8((uint8_t*) buf);
    filesize -= 4;
    filesize -= listsize;

    input(buf, 4);
    listsize -= 4;
    if (strncmp((char*) buf, "hdrl", 4))
    { return -1; }

    input(buf, 4);
    listsize -= 4;
    if (strncmp((char*) buf, "avih", 4))
    { return -1; }

    input(buf, 4); chunksize = read_little32from8((uint8_t*) buf);
    listsize -= 4;

    input((uint32_t*) &avimainheader, sizeof(AVIMAINHEADER));
    listsize -= sizeof(AVIMAINHEADER);
#ifdef AVIDUMPINFO
    printf("avih\n");
    printf("- width/height %d/%d\n", little32(avimainheader.dwWidth), little32(avimainheader.dwHeight));
    printf("- frames %d\n", little32(avimainheader.dwTotalFrames));
    printf("- microsecperframe %d (fps %d.%1d)\n", little32(avimainheader.dwMicroSecPerFrame),
          (unsigned) (1000000 / little32(avimainheader.dwMicroSecPerFrame)),
          (unsigned) (10000000 / little32(avimainheader.dwMicroSecPerFrame) % 10));
    printf("- streams %d\n", little32(avimainheader.dwStreams));
    printf("- initial frames %d\n", little32(avimainheader.dwInitialFrames));
#endif

    format->video.microsecperframe = little32(avimainheader.dwMicroSecPerFrame);

    for (uint32_t streamnr = 0; streamnr < little32(avimainheader.dwStreams); ++streamnr)
    {
        // LIST strl definition for each stream
        AVISTREAMHEADER avistreamheader;
        int32_t streamlistsize;

        input(buf, 4);
        listsize -= 4;
        if (strncmp((char*) buf, "LIST", 4))
        { return -1; }

        input(buf, 4); streamlistsize = read_little32from8((uint8_t*) buf);
        listsize -= 4;
        listsize -= streamlistsize;

        input(buf, 4);
        streamlistsize -= 4;
        if (strncmp((char*) buf, "strl", 4))
        { return -1; }

        input(buf, 4);
        streamlistsize -= 4;
        if (strncmp((char*) buf, "strh", 4))
        { return -1; }

        input(buf, 4); chunksize = read_little32from8((uint8_t*) buf);
        streamlistsize -= 4;

        input((uint32_t*) &avistreamheader, sizeof(AVISTREAMHEADER));
        streamlistsize -= sizeof(AVISTREAMHEADER);
#ifdef AVIDUMPINFO
        printf("strh\n");
        printf("- stream FOURCC '%.4s'\n", (char*) &avistreamheader.fccType);
        printf("- initial frames %d\n", little32(avistreamheader.dwInitialFrames));
        printf("- scale %d\n", little32(avistreamheader.dwScale));
        printf("- rate %d\n", little32(avistreamheader.dwRate));
        printf("- start %d\n", little32(avistreamheader.dwStart));
        printf("- length %d\n", little32(avistreamheader.dwLength));
        printf("- samplesize %d\n", little32(avistreamheader.dwSampleSize));
        printf("- top/left/bottom/right %d/%d/%d/%d\n",
            little16(avistreamheader.rcFrame.top), little16(avistreamheader.rcFrame.left),
            little16(avistreamheader.rcFrame.bottom), little16(avistreamheader.rcFrame.right));
#endif
        input(buf, 4);
        streamlistsize -= 4;
        if (strncmp((char*) buf, "strf", 4))
        { return -1; }

        input(buf, 4); chunksize = read_little32from8((uint8_t*) buf);
        streamlistsize -= 4;

        if (strncmp((char*) &avistreamheader.fccType, "vids", 4) == 0)
        {
            // video stream
            BITMAPINFOHEADER bitmapinfoheader;
            uint16_t bitcount;

            input((uint32_t*) &bitmapinfoheader, sizeof(BITMAPINFOHEADER));
            chunksize -= sizeof(BITMAPINFOHEADER);
            streamlistsize -= sizeof(BITMAPINFOHEADER);
#ifdef AVIDUMPINFO
            printf("strf\n");
            printf("- width/height %d/%d\n", little32(bitmapinfoheader.biWidth), little32(bitmapinfoheader.biHeight));
            printf("- bitcount %d\n", little16(bitmapinfoheader.biBitCount));
            printf("- compressiontype %d\n", little32(bitmapinfoheader.biCompression));
#endif
            format->video.width = little32(bitmapinfoheader.biWidth);
            format->video.height = little32(bitmapinfoheader.biHeight);
            bitcount = little16(bitmapinfoheader.biBitCount);

            if (little32(bitmapinfoheader.biCompression) == BI_RGB)
            {
                if (bitcount == 16)
                {
                    // default 16bit, meaning RGB555
                    format->video.mode = VIDEOMODE_RGB555;
                }
                else if (bitcount == 24)
                {
                    format->video.mode = VIDEOMODE_RGB888;
                }
                else
                {
                    // unsupported colorformat
                    return -1;
                }
            }
            else if (little32(bitmapinfoheader.biCompression) == BI_BITFIELDS)
            {
                uint32_t rmask;
                uint32_t gmask;
                uint32_t bmask;

                // special x-bit, bitfields for RGB in first 3 doubleword entries after this header
                if (chunksize < 12)
                {
                    // should have been 3 doublewords
                    return -1;
                }

                input(buf, 4); rmask = read_little32from8((uint8_t*) buf);
                input(buf, 4); gmask = read_little32from8((uint8_t*) buf);
                input(buf, 4); bmask = read_little32from8((uint8_t*) buf);

                chunksize -= 3 * sizeof(uint32_t);
                streamlistsize -= 3 * sizeof(uint32_t);
#ifdef AVIDUMPINFO
                printf("- RGB bitfields %08X %08X %08X\n", rmask, gmask, bmask);
#endif

                if ((bitcount == 24) && (rmask == 0x0000FF) && (gmask == 0x00FF00) && (bmask == 0xFF0000))
                {
                    format->video.mode = VIDEOMODE_RGB888;
                }
                else if ((bitcount == 16) && (rmask == 0xF800) && (gmask == 0x07E0) && (bmask == 0x001F))
                {
                    format->video.mode = VIDEOMODE_RGB565;
                }
                else if ((bitcount == 16) && (rmask == 0x7C00) && (gmask == 0x03E0) && (bmask == 0x001F))
                {
                    format->video.mode = VIDEOMODE_RGB555;
                }
                else
                {
                    // unsupported colorformat
                    return -1;
                }
            }
            else
            {
                // unsupported videocompression
                return -1;
            }
        }
        else if (strncmp((char*) &avistreamheader.fccType, "auds", 4) == 0)
        {
            // audio stream
            WAVEFORMATEX waveformatex;

            input((uint32_t*) &waveformatex, sizeof(WAVEFORMATEX));
            chunksize -= sizeof(WAVEFORMATEX);
            streamlistsize -= sizeof(WAVEFORMATEX);
#ifdef AVIDUMPINFO
            printf("strf\n");
            printf("- formattag %d\n", little16(waveformatex.wFormatTag));
            printf("- channels %d\n", little16(waveformatex.nChannels));
            printf("- samples per sec %d\n", little32(waveformatex.nSamplesPerSec));
            printf("- bits per sample %d\n", little16(waveformatex.wBitsPerSample));
#endif
            if ( little16(waveformatex.wFormatTag) != AUDIO_FORMAT_PCM)
            { return -1; }

            format->audio.initialframes = little32(avistreamheader.dwInitialFrames);
            format->audio.channels = little16(waveformatex.nChannels);
            format->audio.samplespersec = little32(waveformatex.nSamplesPerSec);
            format->audio.bitspersample = little16(waveformatex.wBitsPerSample);
        }

        if (chunksize > 0)
        {
            // unused stuff after unsupported stream
            input(0, chunksize);
            streamlistsize -= chunksize;
        }

        // skip unknown and optional JUNK alignment chunk
        input(0, streamlistsize);
    }

    // skip unknown chunks
    input(0, listsize);

    input(buf, 4);
    filesize -= 4;

    if (strncmp((char*) buf, "JUNK", 4) == 0)
    {
        // skip optional JUNK alignment chunk
        input(buf, 4); listsize = read_little32from8((uint8_t*) buf);
        filesize -= 4;
        filesize -= listsize;

        input(0, listsize);
        input(buf, 4);
        filesize -= 4;
    }

    if (strncmp((char*) buf, "LIST", 4))
    { return -1; }

    input(buf, 4); listsize = read_little32from8((uint8_t*) buf);
    filesize -= 4;
    filesize -= listsize;

    input(buf, 4);
    listsize -= 4;
    if (strncmp((char*) buf, "movi", 4))
    { return -1; }

    // callback to setup format for video & audio frames
    if (!setup)
    {
        // only needed to parse header, return OK
        return 0;
    }
    retcode = setup();
    if (retcode)
    { return retcode; }

    while (listsize > 0)
    {
        uint32_t sizebuf[1];
        input(buf, 4);
        listsize -= 4;

        input(sizebuf, 4); chunksize = read_little32from8((uint8_t*) sizebuf);
        listsize -= 4;
        listsize -= chunksize;

        // just checking 3rd char for d/w is very optimistic, but thanks to all the
        // goofy AVI implementations out there any decent checking is beyond our KISS goal
        if (((char*) buf)[2] == 'd')
        {
            // hopefully an uncompressed video frame
            // lseek(chunksize, SEEK_CUR);
            if (chunksize <= avbufsize)
            {
                input(avbuf, chunksize);
                retcode = videoprocess(&avbuf, chunksize);
                if (retcode)
                { return retcode; }
            }
            else
            {
#ifdef AVIDUMPINFO
                printf("skipping large videoframe '%.4s' (size %d)\n", (char*) buf, (unsigned) chunksize);
#endif
                input(0, chunksize);
            }
        }
        else if (((char*) buf)[2] == 'w')
        {
            // hopefully an uncompressed audio frame
            if (chunksize <= avbufsize)
            {
                input(avbuf, chunksize);
                retcode = audioprocess(&avbuf, chunksize);
                if (retcode)
                { return retcode; }
            }
            else
            {
#ifdef AVIDUMPINFO
                printf("skipping large audioframe '%.4s' (size %d)\n", (char*) buf, (unsigned) chunksize);
#endif
                input(0, chunksize);
            }
        }
        else
        {
            // unsupported frame
#ifdef AVIDUMPINFO
            printf("aborting on unknown frame '%.4s' (size %d)\n", (char*) buf, (unsigned) chunksize);
            return -1;
#else
            input(0, chunksize);
#endif
        }

        // align on word boundery
        if (chunksize & 1)
        {
            // skip a byte
            input(buf, 1);
            --listsize;
        }
    }

    return 0;
}

//------------------------------------------------------------

