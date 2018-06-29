/*************************************************************************
**
**  VERSION CONTROL:	$Revision:   1.4  $
**			$Date:   Sep 21 2001 11:28:28  $
**
**  IN PACKAGE:		TEA SMS engine
**
**  COPYRIGHT:		Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:	input for SMS (reading and decoding)
**
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WINDOOS
#include <windows.h>
#endif

#include <time.h>

#include "sms.h"
#include "sms_in.h"
#include "sms_comm.h"


/***********************************************************************************
 * 
 *  FUNCTION:		check_for_sms_messages
 *
 *  ENVIRONMENT:	process_sms,	sms message structure
 *			data,		user data to pass to callback function
 *			
 *  REQUIREMENTS:	communication connection is ready
 *
 *  RETURN VALUE:	void
 *
 *  DESCRIPTION:	check for any received messages (read and not read), 
 *			read and delete them.
 *
 */
void check_for_sms_messages(sms_callback_function process_sms, void * data)
{
	int message_id;
	char command[16];
	char rsp[BUF_SIZE];
	char * message;
	char * message_end;
	int max_msg_id;
	SMS_T new_sms_message;


	/* get number of messages stored */
	max_msg_id=get_number_of_messages(); /* default */

	for (message_id=1; message_id<=max_msg_id; message_id++)
	{
		sprintf(command,"ATE0+CMGR=%d\r",message_id);
		exec_cmd(command,rsp,10,"OK",1);

		/* check if this message was stored in "received message" (either read or unread) memory */
		if ((strstr(rsp," 1,")!=NULL) || (strstr(rsp," 0,")!=NULL))
		{
		
			message = strstr(rsp,"\r"); /* skip first */
			message = strstr(message+1,"\r");
			message_end = strstr(message+1,"\r");
			message_end[0]=0;
			/* skip the |+CMGR: 0,,0||OK| messages */

			if (strlen(message)>2)
			{
				sms_log( 2, "INFO: message %d has content\n",message_id);
				message += 2;
				decode_response(message, &new_sms_message);
				new_sms_message.index = message_id;
				process_sms(new_sms_message, data);
				//remove_message(message_id);
			}
			else
			{
				sms_log( 4, "INFO: message %d has no content\n",message_id);
			}
		}
		else
		{
			sms_log( 3, "INFO: message %d has type 'send'\n",message_id);
		}
	}
}

/***********************************************************************************
 * 
 *  FUNCTION:		get_number_of_messages
 *
 *  ENVIRONMENT:	void
 *			
 *  REQUIREMENTS:	communication connection is ready
 *
 *  RETURN VALUE:	number of max. messages (or default 25 on error)
 *
 *  DESCRIPTION:	get the maximum number of messages from modem 
 *
 */
int get_number_of_messages(void)
{
	char rsp[BUF_SIZE];
	unsigned int pos;
	char * number;

	static int msg_nr=25;  /* use static, if success once, don't set it again... */
	static int passed_once=0;

	

	if (passed_once==0)
	{
		exec_cmd("ATE0+CPMS?\r",rsp,5,"OK",1);

		/* reply like +CPMS: <mem1>,used1,total1,<mem2>,used2,total2,<mem3>,used3,total3 OK */
		if (strstr(rsp,"OK") != NULL)
		{
			/* find total entries for mem1 */
			pos = 0;
			number = rsp;
			while ((number[pos]!=',') && (pos<strlen(number)))
			{
				pos++;
			}
			if (pos<strlen(number))
			{
				pos++;  /* skip this comma ! */

				while ((number[pos]!=',') && (pos<strlen(number)))
				{
					pos++;
				}
				if (pos<strlen(number))
				{
					number = number+pos+1;
					pos = 0;
					while ((number[pos]!=',') && (pos<strlen(number)))
					{
						pos++;
					}
					if (pos<strlen(number))
					{
						number[pos]=0;
						sms_log( 3, "INFO: max. nr of entries: %s\n",number);
						msg_nr = atoi(number);

						/* check for sanity */
						if (msg_nr > 0)
						{
							passed_once = 1;
						}
					}
				}
			}
		}
		else
		{
			sms_log( 2, "ERROR: Unable to get number of messages, using default\n");
		}
	}
	return msg_nr;
}

/***********************************************************************************
 * 
 *  FUNCTION:		remove_message
 *
 *  ENVIRONMENT:	index,	index of message to be deleted
 *			
 *  REQUIREMENTS:	communication connection is ready
 *
 *  RETURN VALUE:	void
 *
 *  DESCRIPTION:	delete specified message
 *
 */
void remove_message(int index)
{
	char rsp[BUF_SIZE];
	char command[16];

	sprintf(command,"ATE0+CMGD=%i\r",index);
	exec_cmd(command,rsp,5,"OK",1);
	if (strstr(rsp,"OK") != NULL)
	{
		sms_log( 3, "INFO: message %i deleted\n",index);
	}
	else
	{
		sms_log( 1, "ERROR: could not delete message %i\n",index);
	}
	
}



/***********************************************************************************
 * 
 *  FUNCTION:		decode_byte
 *
 *  ENVIRONMENT:	b, 		byte to decode
 *			mode, 		flag to reset or feed the decoding part
 *			data_length,	length of data to come
 *			buf,		room to store decoded stuff
 *
 *  REQUIREMENTS:	buf is valid an d allocated
 *
 *  RETURN VALUE:	number of written bytes
 *
 *  DESCRIPTION:	bit-operations to decode the 8-bit PDU data bytes into septets
 *			translation is like:
 *
 *			input octets   			 .  .  .  .  .  .  .  . |.  .  .  .  .  .  .  . |.  .  .  .  .  .  .  .
 *			septets bits for A,B,C..	B0 A6 A5 A4 A3 A2 A1 A0 C1 C0 B6 B5 B4 B3 B2 B1 D2 D1 D0 C6 C5 C4 C3 C2
 *
 */
int  decode_byte(unsigned char b, BIT_DECODER_MODE mode, int data_length, char * buf)
{
	static unsigned char kept = 0;
	static unsigned char bits = 7;
	static int len = 0;
	int ret = 0;
	int i;
	unsigned char mask;
	unsigned char constructed;

	if (mode == RESET_BIT_DECODER)
	{
		len = 0;
		kept = 0;
		bits = 7;
	}
	else
	{
		mask = 0;
		for (i=0; i<bits; i++)
		{
			mask = mask | (0x01 << i);
		}

		constructed = ((b & mask) << (7-bits)) | (kept>>(bits+1)) ;
		ret = ret + add_constructed_byte(buf,constructed,&len,data_length);
		kept = b & ~mask;
	
		bits--;

		/*
		 * after processing seven octets (and storing the septets)
		 * we have again another septet available. Add this septet
		 * and reset the bits and "kept" values
		 */
		if (bits == 0)
		{
			ret = ret + add_constructed_byte(buf,(unsigned char)(kept >> 1),&len,data_length);
			bits = 7;
			kept= 0;
		}
	}
	return ret;
}

/***********************************************************************************
 * 
 *  FUNCTION:		add_constructed_byte
 *
 *  ENVIRONMENT:	
 *			
 *
 *  REQUIREMENTS:	
 *
 *  RETURN VALUE:	number of added bytes
 *
 *  DESCRIPTION:	
 *
 */
int add_constructed_byte(char * buf,unsigned char constructed,int * len,int data_length)
{
	int ret = 0;

	if (*len<data_length)
	{
		/* only add char from 7 bit alphabet */
		if (constructed <128)
		{

			buf[*len] = charset[constructed];
			*len = (*len) +1;
			buf[*len] = 0;
			ret=1;
		}
		else
		{
			sms_log( 1, "ERROR: invalid char value 0x2.2X found in decoder.\n",constructed);
		}
	}
	return ret;
}

/***********************************************************************************
 * 
 *  FUNCTION:		decode_response
 *
 *  ENVIRONMENT:	str,		entire PDU response from modem
 *			sms_message,	structure to store SMS properties
 *
 *  REQUIREMENTS:	str and sms_message are valid
 *
 *  RETURN VALUE:	void
 *
 *  DESCRIPTION:	process an entire PDU received from modem and store SMS data
 *
 */
void decode_response(char * str, SMS_T * sms_message)
{
	char * rsp;
	unsigned int pos;
	unsigned char c;
	char hi_nibble,lo_nibble;

	rsp = str;

	/* reset state machine */
	sms_decode_state(0, RESET_BIT_DECODER, sms_message);

	/* rest message structure */
	strcpy(sms_message->timestamp,"");
	strcpy(sms_message->from,"");
	strcpy(sms_message->message,"");

	/* assume reply is pdu */
	pos = 0;
	while ((rsp[pos] != 0x0d) && (pos<strlen(rsp)))
	{
		c = 0;
		hi_nibble = toupper(rsp[pos]);
		pos++;
		lo_nibble = toupper(rsp[pos]);
		pos++;
	
		if ((isxdigit(hi_nibble)) && (isxdigit(lo_nibble)))
		{
			if (isdigit(hi_nibble))
			{
				c = (hi_nibble - '0') << 4;
			}
			else
			{
				c = ((hi_nibble - 'A')+10) << 4;
			}

			if (isdigit(lo_nibble))
			{
				c = c + (lo_nibble - '0');
			}
			else
			{
				c = c + ((lo_nibble - 'A') + 10);
			}
		}
		sms_decode_state(c,FEED_BIT_DECODER,sms_message);
	}

}

/***********************************************************************************
 * 
 *  FUNCTION:		sms_decode_state
 *
 *  ENVIRONMENT:	c,	char to feed to the state machine
 *			reset, reset the state machine ?
 *
 *  REQUIREMENTS:	
 *
 *  RETURN VALUE:	void
 *
 *  DESCRIPTION:	state machine for decoding SMS PDU message
 *			
 *			SMS PDU message globally is like:
 *
 *	Length_of_SMSC_info - SMSC_info - first_SMS_octet - sender_address_length
 *			- [sender_address] - protocol+coding - timestamp
 *			- user-data-length - user-data
 *
 */
void sms_decode_state(unsigned char c, BIT_DECODER_MODE mode, SMS_T * sms_message)
{
	static int dec_state = 0;
	static int smsc_len = 0;
	static int smsc_info_count = 0;
	static int addr_len = 0;
	static int addr_count = 0;
	static int msg_len = 0;
	static int msg_count = 0;
	static int timestamp_count = 0;
	static char buf [BUF_SIZE] = "";
	
	int tmp;

	if (mode == RESET_BIT_DECODER)
	{
		dec_state = 0;
		smsc_len = 0;
		smsc_info_count = 0;
		addr_len = 0;
		addr_count = 0;
		msg_len = 0;
		msg_count = 0;
		timestamp_count = 0;
		strcpy(buf,"");
		decode_byte(0,RESET_BIT_DECODER,0,buf);
		strcpy(buf,"");
	}
	else
	{
		switch (dec_state)
		{
		case 0:
			/* get SMSC info length */
			smsc_len = c;
			if (smsc_len!=0)
			{
				dec_state = 1;
			}
			else
			{
				dec_state = 2; /* no smsc info available */
			}
			break;
		case 1:
			/* get SMSC info */
			smsc_info_count++;
			if (smsc_info_count== smsc_len)
			{
				dec_state = 2; /* info done, goto next state */
			}
			break;
		case 2:
			/* process first octet and go to next state */
			dec_state = 3;
			break;
		case 3:
			/* get address length */
			addr_len = c;
			if (addr_len!=0)
			{
				/* correct for odd/even */
				addr_len = addr_len + addr_len % 2;
	
				/* correct address length for semi octets and extra info */
				/* type-byte, semi-octets, tp_pid and tp-dcs */
				addr_len = addr_len/2 + 3;
				strcpy(buf,"");
				addr_count = 0;
				dec_state = 4;
			}
			else
			{
				dec_state = 5; /* no addr available */
			}
			break;
		case 4:
			/* get addr */
			addr_count++;

			if (addr_count==1)
			{
				sms_message->addr_type = (unsigned char)c;
			}
			else
			{
				if (addr_count <= addr_len-2)
				{
					add_hex(buf,c,SMALL_SIZE);
				}
			}

			if (addr_count == addr_len)
			{
				strcpy(sms_message->from,buf);
				
				strcpy(buf,"");
				dec_state = 5;
			}
	
			break;
		case 5:
			/* get timestamp */
			timestamp_count++;

			tmp = ((c & 0x00F0) >> 4) | ((c & 0x000F) << 4);
			add_hex(buf,(char)tmp,SMALL_SIZE);

			if (timestamp_count == 7)
			{
				strcpy(sms_message->timestamp,buf);
				strcpy(buf,"");
				dec_state = 6;
			}
			break;
		case 6:
			/* get message length (depends on tpdcs field !!!) */
			msg_len = c;
			dec_state=7;
			break;
		case 7:
			/* process message data */
			msg_count = msg_count+decode_byte(c,FEED_BIT_DECODER,msg_len,buf);
			if (msg_count>=msg_len)
			{
				strcpy(sms_message->message,buf);
				strcpy(buf,"");
				
				dec_state = 0;
			}
			break;
		default:
			dec_state = 0;
			break;
	
		}
	}
}

