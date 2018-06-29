/*************************************************************************
**
**  VERSION CONTROL:    %W% %E%
**
**  IN PACKAGE:         Weatherstation Demo
**
**  COPYRIGHT:          Copyright (c) 2003 Altium
**
**  DESCRIPTION:    IP-over-RS232 enabled weather station
**                      Main
**
**************************************************************************/


#ifndef MAIN_H
#define MAIN_H

#define LCD_RATE            100
#define T_MINMAX_TIME       8640000UL
#define T_MINMAX_INTERVAL   60000
#define T_MINMAX_ARRAY_SIZE T_MINMAX_TIME/T_MINMAX_INTERVAL

#define S_MAXAVR_TIME       60000U
#define S_MAXAVR_INTERVAL   100
#define S_MAXAVR_ARRAY_SIZE S_MAXAVR_TIME/S_MAXAVR_INTERVAL

extern unsigned int speed_knots;
extern unsigned int max_speed;
extern unsigned int avg_speed;
extern int temp_celcius;
extern int max_temp;
extern int min_temp;
extern unsigned int wind_direction;

extern unsigned short uptime_seconds;
extern unsigned short uptime_minutes;
extern unsigned short uptime_hours;
extern unsigned short uptime_days;

char *compass_card(unsigned int dir);
char *printval( long val, char prec );
void read_temp_dir( void );

#endif
