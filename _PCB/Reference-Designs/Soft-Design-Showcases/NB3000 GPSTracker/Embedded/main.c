#include <stdio.h>
#include <time.h>
#include <string.h>

#include "swplatform.h"
#include "gps_tracker.h"

extern bool init(modem_t ** modem, lwip_t ** lwip);

void main(void)
{
    gps_tracker_t tracker;

    puts("2G/3G Mobile GPS Tracker Example.");

    tracker.is_2g    = init(&tracker.modem,&tracker.lwip);
    tracker.location = location_open(tracker.is_2g ? LOCATION_2G : LOCATION_3G);
    tracker.status   = led_open(STATUS_LEDS);

    gps_tracker(&tracker);
}


