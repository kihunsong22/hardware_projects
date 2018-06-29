/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    IP-over-RS232 enabled weather station
**                  Interface between main program and TCPIP library
**
**************************************************************************/

#ifndef _TCPIP_MAIN_H_
#define _TCPIP_MAIN_H_

//**************************************************************************

//#define SIM_PIN				"1234"

#define SELF_IP             {192, 168, 100, 1}  // our IP
#define DNS_SELF_NAME       "weatherdemo.test"     // name for us a caller can use
#define PPP_IN_USER         "user"
#define PPP_IN_PASSWORD     "password"
#define PPP_CLIENT_IP       {192, 168, 100, 2}  // IP we give our client
#ifdef SIM_PIN
 #define MODEM_INIT          "AT+CPIN="##SIM_PIN##";+CSNS=4\r"
#else
 #define MODEM_INIT          "AT+CSNS=4\r"
#endif

//**************************************************************************

void tcpipmain_init(void);
int tcpipmain(void);

void tcpipmain_setstation(int stationnr);

//**************************************************************************

#endif
