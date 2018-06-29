#ifndef _URL_H_
#define _URL_H_

#include <stdint.h>
#include "http.h"

int  url_parse(url_t* url, const char* url_text, int max_host, int max_file);

#endif // _URL_H_
