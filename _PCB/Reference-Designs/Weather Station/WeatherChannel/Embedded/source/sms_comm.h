/*************************************************************************
**
**  VERSION CONTROL:    $Revision:   1.1  $
**          $Date:   Sep 21 2001 11:27:44  $
**
**  IN PACKAGE:     TEA SMS engine
**
**  COPYRIGHT:      Copyright (c) 2001 TASKING, Inc.
**
**  DESCRIPTION:    SERIAL COMMUNUCATION
**
**************************************************************************/
#ifndef _SMS_COMM_H_
#define _SMS_COMM_H_

typedef unsigned long DWORD;

void exec_cmd(const char * cmd, char * response, int timeout_sec, const char * expected, char new_line);

#endif
