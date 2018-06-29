/*************************************************************************
**
**  VERSION CONTROL:
**
**  IN PACKAGE:     APS SMS engine
**
**  COPYRIGHT:      Copyright (c) 2003, Altium BV
**
**  DESCRIPTION:    SMS output, encoding of message and phone nr.
**
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#include "sms.h"
#include "sms_out.h"
#include "sms_comm.h"



/***********************************************************************************
 * 
 *  FUNCTION:       encode_message
 *
 *  ENVIRONMENT:    message,    message to PDU-encode
 *          buf,        room to store the encoded result
 *          
 *  REQUIREMENTS:   message and buf are valid and allocated
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    encodes a message according to the PDU format
 *
 *  TODO:   error report
 */
void encode_message(char * message, char * buf)
{
    unsigned int i;

    /* reset output */
    encode_bit(0,RESET_BIT_ENCODER,buf);
    i = strlen( message );
    buf[0] = "0123456789ABCDEF"[i / 16];
    buf[1] = "0123456789ABCDEF"[i % 16];
    buf[2] = '\0';
//    sprintf(buf,"%2.2X",strlen(message));

    for (i=0; i<strlen(message); i++)
    {
        encode_byte(message[i],buf);
    }
    /* flush bit encoder */
    encode_bit(0,FLUSH_BIT_ENCODER,buf);
}

/***********************************************************************************
 *
 *  FUNCTION:       encode_byte
 *
 *  ENVIRONMENT:    b,  byte (septet) to encode
 *          buf,    room to store encoded result
 *
 *  REQUIREMENTS:   b is a septet, buf is valid and allocated
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    bitwise encode the septet in PDU buffer
 *
 */
/* encode a 7 bit char */
void encode_byte(unsigned char b, char * buf)
{
    /* check that high bit is not set */
    if ((b & 0x80) == 0x80)
    {
//        sms_log( 1, "ERROR: unexpected (septet) value 0x%2.2X in encoder\n",b);
    }

    encode_bit((unsigned char)((b &  1))     ,FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b &  2) >> 1),FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b &  4) >> 2),FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b &  8) >> 3),FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b & 16) >> 4),FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b & 32) >> 5),FEED_BIT_ENCODER, buf);
    encode_bit((unsigned char)((b & 64) >> 6),FEED_BIT_ENCODER, buf);
}

/***********************************************************************************
 *
 *  FUNCTION:       encode_bit
 *
 *  ENVIRONMENT:    bit,    bit value (1/0) to store
 *          mode,   flag to reset, feed or flush the encoding part
 *          buf,    room to store PDU encoded stuff
 *
 *  REQUIREMENTS:   buf is valid and allocated
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    do the bit shift operations to store septets as 8-bit PDU data
 *
 *          septets bits for A,B,C..    B0 A6 A5 A4 A3 A2 A1 A0 C1 C0 B6 B5 B4 B3 B2 B1 D2 D1 D0 C6 C5 C4 C3 C2
 *          output octets            .  .  .  .  .  .  .  . |.  .  .  .  .  .  .  . |.  .  .  .  .  .  .  .
 *
 */
void encode_bit(unsigned char bit, BIT_ENCODER_MODE mode, char * buf)
{
    static unsigned char kept = 0;
    static unsigned char bits = 8;
    unsigned char mask;

    /* check that bit is 0 or 1, (unsigned so check >1) */
    if (bit>1)
    {
//        sms_log( 1, "ERROR: unexpected bit value 0x%2.2X in encoder\n",bit);

        /* let bit != 0 become 1 (arbitrary, but should never happen in the first place) */
        bit = 1;
    }

    if (mode == FEED_BIT_ENCODER)
    {
        if (bits==0)
        {
            add_hex(buf,kept,BUF_SIZE);
            kept = 0;
            bits = 8;
        }

        mask = 0x80;  /* fill highest bit */
        kept = (kept >> 1) | (bit*mask);
        bits--;
    }
    else
    {
        /* when invoked in flush mode, output the pending byte */
        if (mode == FLUSH_BIT_ENCODER)
        {
            kept = kept >> bits;
            add_hex(buf,kept,BUF_SIZE);
        }
        kept=0;
        bits=8;
    }
}
/***********************************************************************************
 * 
 *  FUNCTION:       send_sms
 *
 *  ENVIRONMENT:    dest_nr,    PDU coded normal notation int. phone nr.
 *          message,    message to send
 *
 *  REQUIREMENTS:   dest_nr, message are valid.
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    send an SMS message to specified nr.
 *
 */
void send_sms(char * dest_nr, char * message)
{
    char pdu[BUF_SIZE];
    char tmp[BUF_SIZE];
    char rsp[BUF_SIZE];

    strcpy(pdu,"001100");
    /*          ^^      SMSC length 00 */
    /*            ^^    SMS submit first octet 11 (valifity present, PDU format...)*/
    /*              ^^  message reference 00 (let phone decide) */

    nr_to_sms(dest_nr,tmp); /* generate destination nr. */
    strcat(pdu,tmp);

    strcat(pdu,"0000AA");
    /*          ^^      protocol id. */
    /*            ^^    data coding 7-bits */
    /*              ^^  4 day validity */


    encode_message(message,tmp);
    strcat(pdu,tmp);

    /* open in SMS-send mode and wait for prompt */
    sprintf( tmp, "ATE0+CMGS=%d\r", strlen(pdu)/2 -1);
    exec_cmd(tmp, rsp, 5, ">",1);

    if (strstr(rsp,">") != NULL)
    {
//        sms_log( 3, "INFO: start sending '%s'\n",pdu);

        /* send actual message */
        sprintf(tmp,"%s\x1A",pdu); /* add ctrl-Z */
        exec_cmd(tmp, rsp, 20, "OK",0);  /* NO newline here */
        if (strstr(rsp,"ERROR") != NULL)
        {
//            sms_log( 1, "ERROR: send SMS failure: %s\n",rsp);
        }
    }
    else
    {
//        sms_log( 1, "ERROR: did not receive prompt for sending message.\n");
    }
}

/***********************************************************************************
 * 
 *  FUNCTION:       nr_to_sms
 *
 *  ENVIRONMENT:    normal_nr,  normal notation int. phone nr.
 *          encoded_nr, PDU formatted phone nr. (swapped odd/even)
 *
 *  REQUIREMENTS:   normal_nr and encoded_nr are valid and allocated.
 *
 *  RETURN VALUE:   void
 *
 *  DESCRIPTION:    convert an international phone number to the PDU format
 *
 */
void nr_to_sms(char * normal_nr,char * encoded_nr)
{
    unsigned int i,l;

    /* get the length and set nr-format to international */
    sprintf(encoded_nr,"%2.2X91",strlen(normal_nr));

    /* set length to "upper" even value, if needed, 'f' will be added */
    l=strlen(normal_nr);
    l=l+l%2;

    /* swap characters on odd and even indexes */
    for (i=0; i<l; i++)
    {
        if ((i%2) == 0)
        {
            if ((i+1)<strlen(normal_nr))
            {
                encoded_nr[i+4]=normal_nr[i+1];
            }
            else
            {
                encoded_nr[i+4]='F'; /* padding F */
            }
        }
        else
        {
            encoded_nr[i+4]=normal_nr[i-1];
        }
        encoded_nr[i+4+1]=0; /* terminate string... */
    }
    
}

