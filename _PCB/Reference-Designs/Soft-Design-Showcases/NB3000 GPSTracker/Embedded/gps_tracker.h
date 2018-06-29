#ifndef _GPS_TRACKER_H
#define _GPS_TRACKER_H


#define NSEC_PER_MSEC 1000000

extern struct timespec half_second;

typedef struct gps_tracker_s
{
   bool              is_2g;
   modem_t         * modem;
   lwip_t          * lwip;
   location_t      * location;
   led_t           * status;
} gps_tracker_t;

void gps_tracker(gps_tracker_t * tracker);

#if 0

#undef location_open
#undef location_start
#undef location_get_info

#define location_open(x) NULL
#define location_start(x) true
#define location_get_info(x,y) true

#endif

#endif // _GPS_TRACKER_H
