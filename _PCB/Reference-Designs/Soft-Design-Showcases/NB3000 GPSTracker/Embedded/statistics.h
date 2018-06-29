#ifndef _STATISTICS_H
#define _STATISTICS_H

#define GPS_FAILURE_MESSAGE        "Failed to get GPS location."
#define LWIP_FAILURE_MESSAGE       "Failed to reconnect to internet."
#define INTERNET_FAILURE_MESSAGE   "Failed to get street address from google."
#define SUSPEND_FAILURE_MESSAGE        "Failed to suspend PPP."
#define SMS_SEND_FAILURE_MESSAGE   "Failed to send return SMS."
#define RETRY_MESSAGE              "..retrying"


typedef struct sms_target {
    char   number[PHONE_NUM_LEN];
    int    sms_sent;
    int    sms_received;
    int    addr_success;
    int    gps_failure;
    int    modem_failure;
    int    internet_failure;
    int    suspend_failure;
    int    sms_send_failure;
    int    unknown_failure;

    int    gps_restarts;
    int    modem_restarts;
    int    lwip_restarts;
} sms_target_t;


inline void print_statistics(sms_target_t targets[])
{
    int i;

    fputs("\x1B[2J", stdout);
#if GPS_TRACKER
    puts("GPS Restarts ------------------------------|");
    puts("LWIP restarts --------------------------|  |");
    puts("Modem restarts ----------------------|  |  |");
    puts("SMS Send failure -----------------|  |  |  |");
#endif
    puts("Unknown failure --------------|   |  |  |  |");
    puts("Internet failure ---------|   |   |  |  |  |");
    puts("Lwip failure ---------|   |   |   |  |  |  |");
    puts("GPS failure ------|   |   |   |   |  |  |  |");
    puts("Minna Cl. ----|   |   |   |   |   |  |  |  |");
#if GPS_TRACKER
    puts("Sent -----|   |   |   |   |   |   |  |  |  |");
    puts("Recvd-|   |   |   |   |   |   |   |  |  |  |");
#else
    puts("Received -|   |   |   |   |   |   |  |  |  |");
    puts("Sent -|   |   |   |   |   |   |   |  |  |  |");
#endif
    //puts("-------------------------------------------------");

    for (i=0;targets[i].number[0];i++)
    {
        printf("%s: %03u %03u %03u %03u %03u %03u %03u"
#if GPS_TRACKER
                  " %03u %02u %02u %02u"
#endif
                  "\n",
            targets[i].number + 8,
            targets[i].sms_sent,
            targets[i].sms_received,
            targets[i].addr_success,
            targets[i].gps_failure,
            targets[i].modem_failure,
            targets[i].internet_failure,
            targets[i].unknown_failure
#if GPS_TRACKER
            ,
            targets[i].sms_send_failure,
            targets[i].modem_restarts,
            targets[i].lwip_restarts,
            targets[i].gps_restarts
#endif
            );
    }
}

inline void update_statistics(char * message, sms_target_t * target)
{
    target->sms_received++;
    if(strstr(message,GPS_FAILURE_MESSAGE))
        target->gps_failure++;
    else if(strstr(message,LWIP_FAILURE_MESSAGE))
        target->modem_failure++;
    else if(strstr(message,INTERNET_FAILURE_MESSAGE))
        target->internet_failure++;
    else if(strstr(message,SUSPEND_FAILURE_MESSAGE))
        target->suspend_failure++;
    else if(strstr(message,SMS_SEND_FAILURE_MESSAGE))
        target->sms_send_failure++;
    else if(strstr(message,TARGET_ADDRESS))
        target->addr_success++;
    else
        target->unknown_failure++;
}

inline sms_target_t * get_target(sms_target_t targets[], size_t targets_len, char * number)
{
    int i;

    for(i=0;i<targets_len-1;i++)
    {
        if(strlen(targets[i].number) == 0)
        {
           strncpy(targets[i].number,number,PHONE_NUM_LEN);
           break;
        }
        else if (strncmp(targets[i].number,number,PHONE_NUM_LEN)==0)
        {
            break;
        }
    }
    return &targets[i]; // NB targets[target_len-1] is used when all others are full
}

#endif // _STATISTICS_H
