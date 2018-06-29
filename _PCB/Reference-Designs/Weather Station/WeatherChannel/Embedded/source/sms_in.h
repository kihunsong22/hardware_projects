/*************************************************************************
**
**  VERSION CONTROL:	$Revision:   1.0  $
**			$Date:   Jun 15 2001 07:55:12  $
**
**  IN PACKAGE:		TEA SMS engine
**
**  COPYRIGHT:		Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:	input for SMS (reading and decoding)
**
**************************************************************************/
#ifndef _SMS_IN_H_
#define _SMS_IN_H_

/* sms.h needed for SMS_T */
#include "sms.h"

typedef enum bit_decode_mode {
	RESET_BIT_DECODER,
	FEED_BIT_DECODER
} BIT_DECODER_MODE;

void check_for_sms_messages(sms_callback_function process_sms, void * data);
int  get_number_of_messages(void);
void remove_message(int index);
void decode_response(char * str, SMS_T * sms_message);
void sms_decode_state(unsigned char c, BIT_DECODER_MODE mode, SMS_T * sms_message);
int  decode_byte(unsigned char b, BIT_DECODER_MODE mode, int data_length, char * buf);
int add_constructed_byte(char * buf,unsigned char constructed,int * len,int data_length);


extern const char charset[128];


#endif