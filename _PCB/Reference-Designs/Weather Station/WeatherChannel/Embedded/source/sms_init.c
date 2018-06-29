/*************************************************************************
**
**  VERSION CONTROL:
**
**  IN PACKAGE:     APS SMS engine
**
**  COPYRIGHT:      Copyright (c) 2003, Altium BV
**
**  DESCRIPTION:    SMS initialization (pin code, connection etc.)
**
**************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sms.h"
#include "sms_init.h"
#include "sms_comm.h"


/***********************************************************************************
 *
 *  FUNCTION:       check_for_pin
 *
 *  ENVIRONMENT:    pin_code,   string containing the pincode
 *
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   1 on success, 0 otherwise
 *
 *  DESCRIPTION:    checks (and passes if needed) for PIN code
 *
 */
char check_for_pin(const char * pin_code)
{
    char cmd[BUF_SIZE];
    char rsp[BUF_SIZE];
    char ret = 0;

    /* check if PIN is needed */
    exec_cmd("ATE0+CPIN?\r",rsp,10,"+CPIN:",1);
    if (strlen(rsp)==0)
    {
//        sms_log( 1, "ERROR: no response.\n");
    }
    else
    {
        if (strstr(rsp,"+CPIN: SIM PIN") != NULL)
        {
            /* yes, we do need PIN */
            strcpy(cmd,"AT+CPIN=");
            strcat(cmd,pin_code);
            strcat(cmd,"\r");
            exec_cmd(cmd,rsp,30,"OK",1);
//            sms_log( 3, "PIN rsp: %s\n",rsp);

            /* refresh CPIN query to check if we passed the pin... */
            exec_cmd("AT+CPIN?\r",rsp,15,"+CPIN:",1);
//            sms_log( 3, "PIN ready ?: %s\n",rsp);
        }

        if (strstr(rsp,"+CPIN: READY") != NULL)
        {
//            sms_log( 1, "INFO: pin passed.\n");
            ret = 1;
        }
        else
        {
//            sms_log( 1, "ERROR: pin NOT passed.\n");
        }
    }
    return ret;
}

/***********************************************************************************
 *
 *  FUNCTION:       check_for_network
 *
 *  ENVIRONMENT:    void
 *
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   0 if network was found, 1 otherwise
 *
 *  DESCRIPTION:    checks if GSM-network connection is OK
 *          reply is like : AT+REG: <n>,<stat>[,....]
 *          stat should be 1 (checked in)
 */
char check_for_network(void)
{
    char ret = 1;
    static char prev_ret = -1;
    char * tok;
    char rsp[BUF_SIZE];

    /* check if connection to GSM network is ok */
    exec_cmd("ATE0+CREG?\r",rsp,5,"1",1);

    /* get the <stat> parameter from the response */
    tok = strtok (rsp, ",\r");
    if (tok != NULL)
    {
        tok = strtok (NULL, ",\r");
        if (tok != NULL)
        {
            /* not sure if "5' (registered, roaming) could be
                           a valid response as well with some providers? */
                        if (strcmp(tok,"1")==0)
            {
                ret = 0;
            }
        }
    }

    if (ret != prev_ret)
        {
                if (ret == 1)
                {
//                        sms_log( 1, "ERROR: no GSM network.\n");
                }
                else
                {
//                        sms_log( 1, "INFO: GSM network found.\n");
                }
            prev_ret = ret;
    }

    return ret;
}


/***********************************************************************************
 *
 *  FUNCTION:       check_for_pdu_mode
 *
 *  ENVIRONMENT:    void
 *          
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   0 if PDU mode is supported, 1 otherwise
 *
 *  DESCRIPTION:    checks if modem supports PDU
 *          try setting PDU mode and check for "OK"
 *
 */
char check_for_pdu_mode(void)
{
    char ret = 1;
    char rsp[BUF_SIZE];

    /* check if PDU mode is supported */
    exec_cmd("ATE0+CMGF=0\r",rsp,5,"OK",1);
    if (strstr(rsp,"OK") != NULL)
    {
//        sms_log( 3, "INFO: PDU mode set.\n");
        ret = 0;
    }
    else
    {
//        sms_log( 1, "ERROR: PDU mode not set.\n");
    }
    return ret;
}

/***********************************************************************************
 *
 *  FUNCTION:       check_for_gsm_reception
 *
 *  ENVIRONMENT:    void
 *
 *  REQUIREMENTS:   communication connection is ready
 *
 *  RETURN VALUE:   0 - 5 of reception level (0 = very bad reception or n.a.)
 *
 *  DESCRIPTION:    checks the gsm reception level and convert
 *          value on a scale 0-5
 *
 */
char check_for_gsm_reception(void)
{
    char ret = 0;
    int i,level;
    char rsp[BUF_SIZE];
    char * buf;

    /* check GSM reception level */
    exec_cmd("ATE0+CSQ\r",rsp,5,"OK",1);
    if (strstr(rsp,"OK") != NULL)
    {
        buf = strstr(rsp,"+CSQ:");
        if (buf != NULL)
        {
            strcpy(rsp,buf+6);
            i=0;
            while (i < (int)strlen(rsp))
            {
                if (rsp[i]==',')
                {
                    rsp[i]=0;
                }
                i++;
            }
            level = atoi(rsp);
            ret = 0;                 /* -113 dBm or less */
            if (level > 0 ) ret = 1; /* -111 dBm         */
            if (level > 1 ) ret = 2; /*  -109            */
            if (level > 20) ret = 3; /*      ...         */
            if (level > 27) ret = 4; /*         -53 dBm  */
            if (level > 30) ret = 5; /* -51 dBm or more  */
            if (level > 31) ret = 0; /* level unknown    */
        }
    }
    return ret;

}
