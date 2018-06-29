#include <assert.h>
#include <string.h>
#include "url.h"

#define HTTP_PROTOCOL_MAX_LEN 10

typedef struct known_protocol
{
    url_protocol_t type;
    char           string[HTTP_PROTOCOL_MAX_LEN];
    int            string_length;
} known_protocol_t;

#define KNOWN_PROTOCOL_COUNT 1
known_protocol_t known_protocols[KNOWN_PROTOCOL_COUNT] =
{
    { PROTOCOL_HTTP, "http", 4 }
};

int url_parse( url_t* url, const char* url_text, int max_host, int max_file )
{
    int i;
    char* seperator  = NULL;
    char* host_start = NULL;
    char* file_start = NULL;
    char* port_start = NULL;
    int host_len;
    int file_len;

    assert(url && url_text);

    url->port = 80;

    // find protocol
    seperator = strchr(url_text,':');
    if (seperator && *(seperator+1)=='/' && *(seperator+2)=='/')
    {
        int protocol_len = seperator - url_text;
        url->protocol = INVALID_PROTOCOL;
        for ( i=0; i<KNOWN_PROTOCOL_COUNT; i++ )
        {
            if (known_protocols[i].string_length == protocol_len
                && !strncmp(url_text,known_protocols[i].string, known_protocols[i].string_length))
            {
                url->protocol = known_protocols[i].type;
                host_start = seperator + 3;
                break;
            }
        }
        if (url->protocol == INVALID_PROTOCOL)
            return -1;
    }
    else
    {
        url->protocol = PROTOCOL_HTTP;
        host_start = (char *) url_text;
    }

    // is there a file?
    seperator = strchr(host_start, '/');
    if (seperator)
        file_start = seperator;

    // is there a port?
    seperator = strchr(host_start, ':');
    if (seperator)
    {
        port_start = seperator+1;
        if (sscanf(seperator+1, "%hi", &url->port) != 1)
        {
            return -1;
        }
    }

    // copy out host and file portions
    if (!(file_start || port_start))
    {
        strncpy(url->host, host_start, max_host);
        url->host[max_host-1] = 0;
    }
    else
    {
        host_len = 0;
        if (file_start)
            host_len = file_start - host_start;
        else if (port_start)
            host_len = port_start - host_start - 1;
        if ( host_len > max_host )
            host_len = max_host-1;
        strncpy(url->host, host_start, host_len);
        url->host[host_len] = 0;
    }

    if (!file_start)
    {
        strcpy(url->file, "/");
    }
    else if (!port_start)
    {
        strncpy(url->file, file_start, max_file);
        url->file[max_file-1] = 0;
    }
    else
    {
        file_len = port_start - file_start - 1;
        if ( file_len > max_file )
            file_len = max_file-1;
        strncpy(url->file, file_start, file_len);
        url->file[file_len] = 0;
    }

    return 0;
}
