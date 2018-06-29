/*************************************************************************
**
**  VERSION CONTROL:	$Revision:   1.0  $
**			$Date:   Jun 15 2001 07:55:12  $
**
**  IN PACKAGE:		TEA SMS engine
**
**  COPYRIGHT:		Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:	SMS output, encoding of message and phone nr.
**
**************************************************************************/
#ifndef _SMS_OUT_H_
#define _SMS_OUT_H_

typedef enum bit_encode_mode {
	RESET_BIT_ENCODER,
	FEED_BIT_ENCODER,
	FLUSH_BIT_ENCODER
} BIT_ENCODER_MODE;

void send_sms(char * dest_nr, char * message);
void nr_to_sms(char * normal_nr,char * encoded_nr);
void encode_message(char * message, char * buf);
void encode_byte(unsigned char b, char * buf);
void encode_bit(unsigned char bit, BIT_ENCODER_MODE mode, char * buf);

#endif