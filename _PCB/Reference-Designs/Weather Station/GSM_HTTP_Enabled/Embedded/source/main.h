/*************************************************************************
**
**  VERSION CONTROL:    %W%	%E%
**
**  IN PACKAGE:     	Weatherstation Demo
**
**  COPYRIGHT:      	Copyright (c) 2003 Altium
**
**  DESCRIPTION:   	IP-over-RS232 enabled weather station
**                  	Main
**
**************************************************************************/


#ifndef MAIN_H
#define MAIN_H

#define LCD_RATE    100

extern unsigned int speed_knots;
extern int temp_celcius;
extern unsigned int wind_direction;

__rom char *compass_card(unsigned int dir);
char *printval( long val, char prec );
void read_temp_dir( void );

#endif
