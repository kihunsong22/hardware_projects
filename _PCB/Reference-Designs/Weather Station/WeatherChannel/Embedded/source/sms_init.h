/*************************************************************************
**
**  VERSION CONTROL:    $Revision:   1.1  $
**          $Date:   Sep 21 2001 11:29:16  $
**
**  IN PACKAGE:     TEA SMS engine
**
**  COPYRIGHT:      Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:    SMS initialization (pin code, connection etc.)
**
**************************************************************************/
#ifndef _SMS_INIT_H_
#define _SMS_INIT_H_

char check_for_pin(const char * pin_code);
char check_for_network(void);
char check_for_pdu_mode(void);
char check_for_gsm_reception(void);

#endif
