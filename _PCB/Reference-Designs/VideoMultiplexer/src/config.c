/*
 * config.c --
 *
 *     This file provides routines to configure the video encoder
 *     and decoders for the Video Multiplexer project. It also
 *     handles updates of the custom hardware on the FPGA.
 *
 *     Author: Peter Allworth (Linear Solutions Pty Ltd)
 */

#include  "videomux.h"

/* Register interface to custom peripherals (PIP, etc.). */
typedef __sfr volatile unsigned char Port;
#define RegAddr        (*((Port *)0xFE))
#define RegData        (*((Port *)0xFF))

/* Register addresses for picture-in-picture (PIP) hardware. */
enum {
    Xul     = 0x00, /* PIP bounding box upper left X coordinate (1..720) */
    Yul     = 0x02, /* PIP bounding box upper left Y coordinate (1..288) */
    Xlr     = 0x04, /* PIP bounding box lower right X coordinate (1..720) */
    Ylr     = 0x06, /* PIP bounding box upper left Y coordinate (1..288) */
    Xlim    = 0x10, /* PIP DMA X limit */
    Ylim    = 0x12, /* PIP DMA Y limit */
    Xinc    = 0x14, /* PIP DMA X increment */
    Yinc    = 0x16, /* PIP DMA Y increment */
    StartL  = 0x18, /* PIP DMA start address low byte */
    StartH  = 0x19, /* PIP DMA start address high byte */
    StartU  = 0x1A, /* PIP DMA start address upper byte */
    DecXlim    = 0x20, /* Decimator DMA X limit */
    DecYlim    = 0x22, /* Decimator DMA Y limit */
    DecXinc    = 0x24, /* Decimator DMA X increment */
    DecYinc    = 0x26, /* Decimator DMA Y increment */
    DecStartL  = 0x28, /* Decimator DMA start address low byte */
    DecStartH  = 0x29, /* Decimator DMA start address high byte */
    DecStartU  = 0x2A, /* Decimator DMA start address upper byte */
};

/* Constants for the screen layout of Picture-in-Picture. */
enum {
    ScreenHeight    = 288,
    ScreenWidth     = 720,
    PipHeight       = (ScreenHeight/4),
    PipWidth        = (ScreenWidth/4),
    Xleft           = ((ScreenWidth/2 - PipWidth)/2),
    Xright          = (Xleft + (ScreenWidth/2)),
    Yupper          = ((ScreenHeight/2 - PipHeight)/2),
    Ylower          = (Yupper + (ScreenHeight/2)),
};

#ifdef SAA7121

/*
 * Configuration for SAA7121 comes from Philips application note AN97086.
 * The chip is set up to work as a slave, detect sync sygnals out of the data
 * stream, PAL output format with WSS, TTX and closed caption disabled.
 *
 * The configuration tables consist of records of the form:
 *     <no. data bytes> [ <start address> <data byte> ... ]
 * A zero value for <no. data bytes> indicates the end of the table.
 */

#define EncoderCfg  cfgSAA7121
static const __rom uchar cfgSAA7121[] = {
    38, 0x00, /* null */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    9, 0x26, /* WSS, BS, BE, DECCOL, DECFIS, copyguard */
       0x00, 0x00, 0x21, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00,
    11, 0x2F, /* null */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    1, 0x3A, /* colour bar enable (off) */
       0x13,
    6, 0x5A, /* CHPS, GAINU, GAINV, BLCKL, BLNNL */
       0x0C, 0x7D, 0xAF, 0x23, 0x35, 0x35,
    3, 0x60, /* FISE, PAL, etc., RTCE, BSTA */
       0x00, 0x06, 0x2F,
    4, 0x63, /* FSC (PAL) */
       0xCB, 0x8A, 0x09, 0x2A,
    4, 0x67, /* Line 21 captioning data. */
       0x55, 0x56, 0x67, 0x58,
    1, 0x6B, /* RCV1 & RCV2 configuration. */
       0x20,
    4, 0x6C, /* Trigger, multi and closed caption control. */
       0x05, 0x20, 0xA0, 0x14,
    3, 0x70, /* RCV2 start and end. */
       0x80, 0xE8, 0x10,
    7, 0x73, /* Teletext configuration (PAL). */
       0x42, 0x03, 0x03, 0x05, 0x16, 0x04, 0x16,
    3, 0x7A, /* FAL, LAL, TTX60. */
       0x18, 0x38, 0x40,
    1, 0x7D, /* null */
       0x00,
    2, 0x7E, /* LINE */
       0x00, 0x00,
    0        /* End of configuration table. */
};

#else

/*
 * Configuration for SAA7127 is based on a Linux driver file "saa7127.c".
 * The chip is set up to work as a slave, detect sync sygnals out of the data
 * stream, PAL output format with WSS, TTX and closed caption disabled.
 *
 * The configuration tables consist of records of the form:
 *     <no. data bytes> [ <start address> <data byte> ... ]
 * A zero value for <no. data bytes> indicates the end of the table.
 */

#define EncoderCfg  cfgSAA7127
static const __rom uchar cfgSAA7127[] = {
    37, 0x01, /* null */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00,
    9, 0x26, /* WSS, BS, BE, DECCOL, DECFIS, copyguard */
       0x08, 0x00, 0x21, 0x1D, 0x00, 0x00, 0x00, 0x0F, 0x00,
    9, 0x2F, /* null */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    2, 0x38, /* Gain luminance and colour difference for RGB */
       0x1A, 0x1A,
    1, 0x3A, /* colour bar enable (off) */
       0x13,
    25, 0x3B, /* null */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00,
    6, 0x54, /* VPS enable, input control 2 */
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    6, 0x5A, /* CHPS, GAINU, GAINV, BLCKL, BLNNL */
       0x6B, 0x7D, 0xAF, 0x33, 0x35, 0x35,
    3, 0x60, /* FISE, PAL, etc., RTCE, BSTA */
       0x00, 0x86, 0x2F,
    4, 0x63, /* FSC (PAL) */
       0xCB, 0x8A, 0x09, 0x2A,
    4, 0x67, /* Line 21 captioning data. */
       0x55, 0x56, 0x67, 0x58,
    1, 0x6B, /* RCV1 & RCV2 configuration. */
       0x00, /* (Set to 0x12 if RCV1, RCV2 to be outputs.) */
    4, 0x6C, /* Trigger, multi and closed caption control. */
       0x09, 0x20, 0xA0, 0x14,
    3, 0x70, /* RCV2 start and end. */
       0xc9, 0x68, 0x60,
    7, 0x73, /* Teletext configuration (PAL). */
       0x42, 0x03, 0x03, 0x05, 0x16, 0x04, 0x16,
    3, 0x7A, /* FAL, LAL, TTX60. */
       0x18, 0x38, 0x40,
    1, 0x7D, /* null */
       0x00,
    2, 0x7E, /* LINE */
       0x00, 0x00,
    0        /* End of configuration table. */
};

#endif

/*
 * The following configuration is from Table 41 of the datasheet.
 */

enum {
    ModeReg = 0x02,            /* Address of Mode register. */
    Mode0   = 0xC0             /* Mode 0 -> CVBS from A11 input. */
};

#define DecoderCfg  cfgSAA7111A
static const __rom uchar cfgSAA7111A[] = {
    1, 0x00, 0x00,
    1, 0x01, 0x00,
    1, ModeReg, Mode0,
    1, 0x03, 0x23,
    1, 0x04, 0x00,
    1, 0x05, 0x00,
    1, 0x06, 0xEB,
    1, 0x07, 0xE0,
    1, 0x08, 0x88,
    1, 0x09, 0x01,
    1, 0x0A, 0x80,
    1, 0x0B, 0x47,
    1, 0x0C, 0x40,
    1, 0x0D, 0x00,
    1, 0x0E, 0x01,
    1, 0x0F, 0x00,
    1, 0x10, 0xC8, /* OFTS = B'11' -> CCIR-656, 8-bits */
    1, 0x11, 0x1C,
    1, 0x12, 0x00,
    1, 0x13, 0x00,
    1, 0x14, 0x00,
    1, 0x15, 0x00,
    1, 0x16, 0x00,
    1, 0x17, 0x00,
    1, 0x18, 0x00,
    1, 0x19, 0x00,
    1, 0x1D, 0x00,
    1, 0x1E, 0x00,
    0
};

/* Configure: Set up an I2C device based on a table of values. */
static int
Configure(uchar dev, __rom uchar table[])
{
    uchar count, addr;
    int   i, n, total;

    /* Table format is <count> [ <address> <values> ... ]. */
    for(total = 0, i = 0; (count = table[i++]) != 0;) {
       addr = table[i++];
       while (count-- != 0) {
          n = I2cPoke(dev, addr++, table[i++]);
          if (n < 0) {
             return n;
          } else if (n != 1) {
             return total;
          }
          total++;
       }
    }
    return total;
}

/* EncoderInit: Set up the I2C registers of a video encoder. */
int
EncoderInit(uchar dev)
{
    return Configure(dev, EncoderCfg);
}

/* DecoderInit: Set up the I2C registers of a video decoder. */
int
DecoderInit(uchar dev)
{
    return Configure(dev, DecoderCfg);
}

static struct {
    unsigned  xul, yul, xlr, ylr;     /* PIP bounding box. */
    uchar     rdfield;                /* field buffer being read */
    uchar     wrfield;                /* field buffer being written */
} mailbox;

/* NextField: Return the next field no. in a modulo 4 sequence. */
static uchar
NextField(uchar fld)
{
    if (++fld > 3) {
        fld = 0;
    }
    return fld;
}

/* NextField: Return the next field no. in a modulo 4 sequence. */
static uchar
PrevField(uchar fld)
{
    if (fld-- == 0) {
        fld = 3;
    }
    return fld;
}

/* PutByte: Write a byte to a register in the FPGA. */
static void
PutByte(uchar addr, uchar value)
{
    RegAddr = addr;
    RegData = value;
}

/* PutWord: Write a 16 bit value to a register pair in the FPGA. */
static void
PutWord(uchar addr, unsigned value)
{
    /* Write least significant byte then most significant. */
    PutByte(addr, (uchar)value);
    PutByte(addr + 1, (uchar)(value >> 8));
}

/* UpdatePicture: Called during interrupts to update the registers. */
void
UpdatePicture(void)
{
#define FLDA   P2_0  /* current value of the BT656 F bit */
    uchar      next_field;

    /* bring current field up-to-date and set up DMA pointers for next */
    mailbox.rdfield = mailbox.wrfield;
    do {
        mailbox.rdfield = PrevField(mailbox.rdfield);
    } while ((mailbox.rdfield & 1) != FLDA);
    next_field = NextField(mailbox.rdfield);

    /* DMA test (2 bytes per pixel) */
    PutWord(Xlim, 2*PipWidth);
    PutWord(Ylim, PipHeight);
    PutWord(Xinc, 1);
    PutWord(Yinc, 0);
    PutByte(StartL, 0);
    PutByte(StartH, (next_field << 7));
    PutByte(StartU, (next_field >> 1));

    /* Update the PIP bounding box. */
    PutWord(Xul, mailbox.xul);
    PutWord(Yul, mailbox.yul);
    PutWord(Xlr, mailbox.xlr);
    PutWord(Ylr, mailbox.ylr);
}

/* UpdatePip: Called during interrupts to update the registers. */
void
UpdatePip(void)
{
#define FLDB   P2_1  /* current value of the BT656 F bit */
    uchar      next_field;

    /* bring current field up-to-date and set up DMA pointers for next */
    while ((mailbox.wrfield & 1) != FLDB) {
        mailbox.wrfield = NextField(mailbox.wrfield);
    }
    next_field = NextField(mailbox.wrfield);

    /* Decimator setup */
    PutWord(DecXlim, 2*PipWidth);
    PutWord(DecYlim, PipHeight);
    PutWord(DecXinc, 1);
    PutWord(DecYinc, 0);
    PutByte(DecStartL, 0);
    PutByte(DecStartH, (next_field << 7));
    PutByte(DecStartU, (next_field >> 1));
}

/* SetChannel: Select the input channel for the main image. */
void
SetChannel(KeyNum n)
{
    /* Disable interrupts during update of the mailbox. */
    EA = 0;
    mailbox.xul = 0;
    mailbox.yul = 0;
    mailbox.xlr = 0;
    mailbox.ylr = 0;
    EA = 1;
    (void)I2cPoke(DecoderA, ModeReg, (Mode0 + n));
}

/* SetPip: Select the input channel and bounding box for PIP. */
void
SetPip(KeyNum n)
{
    (void)I2cPoke(DecoderB, ModeReg, (Mode0 + n));
    /* Disable interrupts during update of the mailbox. */
    EA = 0;
    mailbox.xul = (n & 1)? Xright : Xleft;
    mailbox.yul = (n > 1)? Ylower : Yupper;
    mailbox.xlr = mailbox.xul + PipWidth - 1;
    mailbox.ylr = mailbox.yul + PipHeight - 1;
    EA = 1;
}
