#ifndef FORM1_H
#define FORM1_H
#include <agui.h>
//#include <audio.h>

extern form_t form1;
extern form_t form2;
extern char *gui_playfile;

extern bool avi_parseheader(char *filename, char *info);
extern void dirlisting( const char *path );

#endif
