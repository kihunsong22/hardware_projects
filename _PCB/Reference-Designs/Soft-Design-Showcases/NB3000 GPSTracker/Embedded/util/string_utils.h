/*
 * File     : $RCSfile: _string.h,v $
 * Created  : Feb 2007
 * Updated  : $Date: 27/07/2009 2:04:25 PM $
 * Author   : Joseph Thomas-Kerr
 * Synopsis : zzzz
 *
 * Audinate Platform Copyright Header Version 1
 */

#ifndef __STRING_H_
#define __STRING_H_


#ifdef __cplusplus
extern "C" {
#endif

    char *
    strdup (const char *s);

    int
    strncasecmp(const char *s1, const char *s2, unsigned char n);

    int
    strcasecmp( const char * s1, const char * s2 );

#ifdef __cplusplus
}
#endif

#endif /*_ _STRING_H_ */
