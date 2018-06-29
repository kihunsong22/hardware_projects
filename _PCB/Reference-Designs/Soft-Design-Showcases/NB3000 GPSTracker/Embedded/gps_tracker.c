#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "swplatform.h"

#include "gps_tracker.h"
#include "http/http.h"
#include "csv.h"
#include <lwip/err.h>

#define NSEC_PER_MSEC 1000000
#define MSEC_PER_SEC  1000
#define MSEC_PER_MIN  (60 * MSEC_PER_SEC)
#define MSEC_PER_HOUR (60 * MSEC_PER_MIN)
#define MSEC_PER_DAY  (24 * MSEC_PER_HOUR)

#define LWIP_START_RETRIES  10
#define LWIP_STOP_RETRIES   10
#define SMSC_INTERNATIONAL  0x91
#define GPS_TIMEOUT_MS (5 * MSEC_PER_MIN)

#define GOOGLE_MAPS_API_KEY "ABQIAAAA8RP6gtPDgyBhfqq-DgIjFBRPQPfriPa7DOeKzuGxtvPxARhl4xRmrlM0xOJ0lDbakPbO9QMfNAetuw"

typedef enum
{
    G_GEO_SUCCESS             = 200,
    G_GEO_SERVER_ERROR        = 500,
    G_GEO_MISSING_QUERY       = 601,
    G_GEO_UNKNOWN_ADDRESS     = 602,
    G_GEO_UNAVAILABLE_ADDRESS = 603,
    G_GEO_BAD_KEY             = 610,
    G_GEO_TOO_MANY_QUERIES    = 620,
} google_result_t;

#define GPS_TRACKER 1
#define TARGET_ADDRESS "Minna"
#include "statistics.h"

#define MAX_TARGETS 10
sms_target_t targets[MAX_TARGETS+2];
static int modem_restarts = 0;
static int gps_restarts = 0;
static int sms_resends = 0;

#define RUNNING_LED STATUS_LED6_B
#define GPS_LOCK_LED STATUS_LED7_G

static void refresh_screen()
{
    static uint64_t start_ms = 0;
    uint64_t uptime_ms;
    uint32_t days,hours,mins,secs;
    if (start_ms == 0) start_ms = clock_ms();
    uptime_ms = clock_ms() - start_ms;

    days  =  (uint32_t)  (uptime_ms / MSEC_PER_DAY);
    hours =  (uint32_t) ((uptime_ms % MSEC_PER_DAY)  / MSEC_PER_HOUR);
    mins  =  (uint32_t) ((uptime_ms % MSEC_PER_HOUR) / MSEC_PER_MIN);
    secs  =  (uint32_t) ((uptime_ms % MSEC_PER_MIN)  / MSEC_PER_SEC);

    print_statistics(targets);
    if (modem_restarts) printf("Other modem restarts: %u\n",modem_restarts);
    if (gps_restarts)   printf("Other GPS restarts: %u\n",gps_restarts);
    if (sms_resends)    printf("SMS resends: %u\n",sms_resends);
    printf("Uptime: %ud %uh %um %us\n", days, hours, mins, secs);
}

static const char * accuracy_to_str(int accuracy)
{
    switch (accuracy)
    {
    case 0:  return "Unknown accuracy.";
    case 1:  return "Country level accuracy.";
    case 2:  return "Region (state, province, prefecture, etc.) level accuracy.";
    case 3:  return "Sub-region (county, municipality, etc.) level accuracy.";
    case 4:  return "Town (city, village) level accuracy.";
    case 5:  return "Post code (zip code) level accuracy.";
    case 6:  return "Street level accuracy.";
    case 7:  return "Intersection level accuracy.";
    case 8:  return "Address level accuracy.";
    case 9:  return "Premise (building name, property name, shopping center, etc.) level accuracy.";
    }
    return "";
}

static const char * google_result_to_str(google_result_t result)
{
    switch (result)
    {
    case G_GEO_SUCCESS             :  return "SUCCESS";
    case G_GEO_SERVER_ERROR        :  return "A geocoding or directions request could not be successfully processed, yet the exact reason for the failure is unknown.";
    case G_GEO_MISSING_QUERY       :  return "An empty address was specified in the HTTP q parameter.";
    case G_GEO_UNKNOWN_ADDRESS     :  return "No corresponding geographic location could be found for the specified address, possibly because the address is relatively new, or because it may be incorrect.";
    case G_GEO_UNAVAILABLE_ADDRESS :  return "The geocode for the given address or the route for the given directions query cannot be returned due to legal or contractual reasons.";
    case G_GEO_BAD_KEY             :  return "The given key is either invalid or does not match the domain for which it was given.";
    case G_GEO_TOO_MANY_QUERIES    :  return "The given key has gone over the requests limit in the 24 hour period or has submitted too many requests in too short a period of time. If you're sending multiple requests in parallel or in a tight loop, use a timer or pause in your code to make sure you don't send the requests too quickly.";
    default                        :  return "UNKNOWN";
    }
}

#define MAX_TOKENS 10
static char * token[MAX_TOKENS];
static int token_count = MAX_TOKENS;
static http_t http;
static url_t url;

static bool get_street_address (lwip_t * lwip, double latitude, double longitude, char * buf, size_t bufsiz)
{
    int err       = 0;
    int bytes_read  = 0;
    int total_bytes = 0;
    int accuracy;
    int result;

    url.protocol = PROTOCOL_HTTP;
    url.port     = 80;
    url.host     = "maps.google.com";
    url.file     = buf;
    sprintf(buf, "/maps/geo?q=%lf,%lf&output=csv&oe=utf8&sensor=true&key=" GOOGLE_MAPS_API_KEY, latitude, longitude);

    printf ( "Getting address of %.3lf,%.3lf.\n",latitude,longitude );
    err = http_begin_request ( &http, &url, HTTP_METHOD_GET );
    if (err) goto final;

    err = http_send_request ( &http );
    if (err) goto final;

    total_bytes = 0;
    err = http_read(&http, buf, bufsiz, &bytes_read);
    while (!err && (bytes_read > 0))
    {
        total_bytes += bytes_read;
        err = http_read(&http, buf+total_bytes, bufsiz-total_bytes, &bytes_read);
    }
    if (err) goto final;

    tokenize_csv_line(buf,bufsiz,token,&token_count);
    if (token_count < 3)
    {
        err = HTTP_ERR_HTTP;
        goto final;
    }

    result = atoi(token[0]);
    if (result==G_GEO_SUCCESS)
    {
        if (strlen(token[2]) >= bufsiz)
            token[2][bufsiz-1] = '\0';
        strcpy(buf,token[2]);
        accuracy = atoi(token[1]);
        if (accuracy != 0)
            snprintf(buf+strlen(buf),bufsiz-strlen(buf), " (%s)", accuracy_to_str(accuracy));
    }
    else
        snprintf(buf,bufsiz,"Failed to resolve (%0.2lf,%0.2lf): %s",latitude,longitude,google_result_to_str(result));
final:
    http_close(&http);
    if (err) http_print_error(err);
    return err==0;
}

bool get_sms(modem_t *modem, char * from, char * message)
{
    int     msg_index;
    char    pdu_message[SMS_MSG_SIZE];
    smsg_t  sms_message;

    msg_index = modem_read_sms(modem, pdu_message,SMS_UNREAD);
    if (msg_index < 0) return false;

    sms_pdu_decode(pdu_message, &sms_message);
    strncpy(from, sms_message.from,PHONE_NUM_LEN);
    strncpy(message, sms_message.message,SMS_MSG_SIZE);
    modem_delete_sms(modem,msg_index);
    return (sms_message.addr_type == SMSC_INTERNATIONAL);
}

int send_sms(modem_t *modem, char * buf, char * to)
{
    char    pdu_message[SMS_MSG_SIZE];

    printf("Sending SMS to %s.\n",to);
    if (strlen(buf)>=160) buf[159] = '\0';

    sms_pdu_encode(buf, to, pdu_message);
    return modem_send_sms(modem, pdu_message);
}

static bool restart_modem(modem_t * modem)
{
    uint64_t timeout = clock_ms() + 2 * MSEC_PER_MIN;
    while (clock_ms() < timeout)
    {                                                          
         if (modem_set_mode(modem,MODEM_CMD))
             return true;
         if (modem_start(modem))
             return true;
    }
    return false;
}

static bool restart_location(gps_tracker_t * tracker)
{
    static uint64_t next_restart = 0;
    struct timespec five_seconds = { 5, 0};

    if (next_restart < clock_ms())
    {
        next_restart = clock_ms() + GPS_TIMEOUT_MS;

        location_stop(tracker->location);
        nanosleep(&five_seconds,NULL);
        return location_start(tracker->location);
    }
    return false;
}

char buf[BUFSIZ];

static bool restart_lwip(gps_tracker_t * tracker)
{
    lwip_stop(tracker->lwip);
    while (modem_read(tracker->modem,(uint8_t *)buf,BUFSIZ)); // clear the buffers
    return lwip_start(tracker->lwip);
}

static void test_status(gps_tracker_t * tracker)
{
    location_info_t   data;

    if (!is_modem_ready(tracker->modem))
    {
            ++modem_restarts;
            refresh_screen();
            restart_modem(tracker->modem);
            lwip_start(tracker->lwip);
    }

    if (location_get_info(tracker->location,&data))
        led_turn_on(tracker->status,GPS_LOCK_LED);
    else
    {
        led_turn_off(tracker->status,GPS_LOCK_LED);
        if (restart_location(tracker))
            ++gps_restarts;
    }
}

#define GPS_TRACKER_RETRIES 3
void gps_tracker(gps_tracker_t * tracker)
{
    struct timespec one_second  = { 1, 0};
    struct timespec ten_seconds = { 10, 0};
    location_info_t   data;
    int success, r=0, i=0, j=0;
    char requester[PHONE_NUM_LEN];
    sms_target_t * target;
    char * response;
    int response_len;

    puts("Waiting for PPP session to suspend...");
    modem_set_mode(tracker->modem,MODEM_CMD);

    puts("Starting GPS...");
    while (!location_start(tracker->location));

    puts("Clearing SMS memory.");
    modem_sms_init(tracker->modem,SMS_STORAGE_SIM);
    modem_delete_multiple_sms(tracker->modem,SMS_ALL);

    puts("\nSend an SMS to the nanoboard \n"
         "to retrieve its street address.\n");

    memset(targets,0,sizeof(targets));
    strncpy(targets[MAX_TARGETS].number, "        Oth",PHONE_NUM_LEN);

    for(;;)
    {
        led_turn_on(tracker->status,RUNNING_LED);
        if (j++ % 10 == 0) refresh_screen();
        test_status(tracker);

        success = get_sms(tracker->modem,requester,buf);
        led_turn_off(tracker->status,RUNNING_LED);
        if (!success)
        {
            nanosleep(&one_second,NULL);
            continue;
        }

        strncat(buf, tracker->is_2g ? " (2g) ==>\n" : " (3g) ==>\n",BUFSIZ);
        response = buf + strlen(buf);
        response_len = BUFSIZ - strlen(buf);

        target = get_target(targets,MAX_TARGETS+1,requester);
        ++target->sms_sent;
        refresh_screen();

        r = 0;
        while (!(success = location_get_info(tracker->location,&data)) && r++ < GPS_TRACKER_RETRIES)
        {
            if (restart_location(tracker))
                ++target->gps_restarts;
            nanosleep(&ten_seconds,NULL);
        }
        if (!success)
        {
            strncpy(response,GPS_FAILURE_MESSAGE,response_len);
            goto final;
        }                                               

        r = 0;
        success = modem_set_mode(tracker->modem,MODEM_DATA);
        if (!success)
            while (!(success = restart_lwip(tracker)) && r++ < GPS_TRACKER_RETRIES)
                ++target->lwip_restarts;
        if (!success)
        {
            strncpy(response,LWIP_FAILURE_MESSAGE,response_len);
            goto final;
        }
        led_turn_on(tracker->status,RUNNING_LED);

        r=0;
        while (!(success = get_street_address(tracker->lwip,data.latitude,data.longitude,response,response_len)) && r++ < GPS_TRACKER_RETRIES)
        {
            ++target->lwip_restarts;
            restart_lwip(tracker);
        }
        if (!success)
        {
            strncpy(response,INTERNET_FAILURE_MESSAGE,response_len);
        }
final:
        success = modem_set_mode(tracker->modem,MODEM_CMD);
        if (!success)
        {
            ++modem_restarts;
            strncpy(response,SUSPEND_FAILURE_MESSAGE,response_len);
            restart_modem(tracker->modem);
        }
        led_turn_off(tracker->status,RUNNING_LED);

        r=0;
        while (!(success = send_sms(tracker->modem,buf,requester)) && r++ < GPS_TRACKER_RETRIES)
        {
            ++sms_resends;
            restart_modem(tracker->modem);
        }
        if (!success)
        {
            strncpy(response,SMS_SEND_FAILURE_MESSAGE,response_len);
        }
        update_statistics(buf,target);
        refresh_screen();
    }
}


