/*************************************************************************
**
**  VERSION CONTROL:    $Revision:   1.2  $
**          $Date:   Jun 22 2001 14:37:36  $
**
**  IN PACKAGE:     TEA SMS engine
**
**  COPYRIGHT:      Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:    SMS engine
**
**************************************************************************/
#ifndef _SMS_H_
#define _SMS_H_

#define BUF_SIZE 512
#define SMALL_SIZE 32

typedef struct sms_t
{
    int index;
    char timestamp[SMALL_SIZE];
    unsigned char addr_type;
    char from[SMALL_SIZE];
    char message[BUF_SIZE];
} SMS_T;

void add_hex(char * buf, char c, int max_len);

// iso 8859-1 default 7-bit charset
extern const char charset[128];

/* prototype callback_function for "walking" through all keywords in specified section */
typedef void (* sms_callback_function)(SMS_T sms_message, void * data);

#endif
