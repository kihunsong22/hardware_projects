#ifndef _HTTP_H_
#define _HTTP_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    INVALID_PROTOCOL    = 0,
    PROTOCOL_HTTP       = 1,
} url_protocol_t;

typedef struct url_s
{
    url_protocol_t protocol;
    uint16_t       port;
    char*          host;
    char*          file;
} url_t;

#define HTTP_ERR_OK                          0
#define HTTP_ERR_UNKNOWN_HOST               -1
#define HTTP_ERR_CONNECT                    -2
#define HTTP_ERR_SEND                       -3
#define HTTP_ERR_READ                       -4
#define HTTP_ERR_HTTP                       -5
#define HTTP_ERR_RESOURCE                   -6

#define HTTP_STATUS_NONE                   0
#define HTTP_STATUS_READ_OK                200
#define HTTP_STATUS_CREATE_OK              201
#define HTTP_STATUS_MOVED                  301
#define HTTP_STATUS_FOUND                  302
#define HTTP_STATUS_INVALID                400
#define HTTP_STATUS_FORBIDDEN              403
#define HTTP_STATUS_NOT_FOUND              404
#define HTTP_STATUS_TIMEOUT                408
#define HTTP_STATUS_SERVER                 500
#define HTTP_STATUS_NOT_IMPLEMENTED        501
#define HTTP_STATUS_OVERLOADED             503

typedef enum
{
    HTTP_METHOD_GET,
    HTTP_METHOD_POST
} http_method_t;

#define HTTP_HEADER_BUFFER_SIZE        256
#define HTTP_EXTRA_HEADERS_BUFFER_SIZE 512
typedef struct _http
{
    // Response info
    int           response_status;
    char          response_transfer_encoding[HTTP_HEADER_BUFFER_SIZE];
    char          response_content_type[HTTP_HEADER_BUFFER_SIZE];
    char          response_location[HTTP_HEADER_BUFFER_SIZE];
    int           response_timeout_ms;
    // Is there data available to read?
    bool          data_available;
    // BBB move these internal fields into hidden data type
    // internal fields
    int           socket;
    http_method_t request_method;
    url_t*        request_url;
    char*         request_content;
    char*         request_content_type;
    int           request_content_length;
    char          request_extra_headers[HTTP_EXTRA_HEADERS_BUFFER_SIZE];
    bool          chunked;
    int           length_left;
    char          header_buffer[HTTP_HEADER_BUFFER_SIZE];
} http_t;

// http service functions
extern void    http_print_error      ( int error );
extern void    http_print_status     ( int status );

// http handle functions
//extern http_t* http_create           ( void );
//extern void    http_destroy          ( http_t*        http );

// http request functions
extern int     http_begin_request    ( http_t*        http,
                                       url_t*         url,
                                       http_method_t  method );
extern int     http_set_header       ( http_t*        http,
                                       char*          header_name,
                                       char*          header_value );
extern int     http_set_content      ( http_t*        http,
                                       char*          content_type,
                                       int            content_length,
                                       char*          content );
extern int     http_send_request     ( http_t*        http );
extern int     http_read             ( http_t*        http,
                                       char*          content,
                                       int            max_length,
                                       int*           length_got );
extern void    http_close            ( http_t*        http );

#endif // _HTTP_H_
