#include <errno.h>
#include <string.h>
#include <lwip.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/inet.h>
#include <lwip/tcpip.h>
#include <lwip/api.h>
#include "util/lwip_utils.h"
#include "util/string_utils.h"
#include "http.h"

#define HTTP_USER_AGENT "AltiumHTTPService 1.0"

#define HTTP_LOCATION_HEADER          "location"
#define HTTP_CONTENT_TYPE_HEADER      "content-type"
#define HTTP_CONTENT_LENGTH_HEADER    "content-length"
#define HTTP_TRANSFER_ENCODING_HEADER "transfer-encoding"
#define HTTP_ENCODING_CHUNKED         "chunked"

#define HTTP_DEBUG_ERRORS
//#define HTTP_DEBUG_TRACE

#define HTTP_DNS_ATTEMPTS 3
#define HTTP_CONNECT_ATTEMPTS 4
#define HTTP_INITIAL_BACKOFF_S 1

#ifdef HTTP_DEBUG_TRACE
#define HTTP_TRACE(x) fputs (x, stdout)
#define HTTP_TRACE_ARGS(x, ...) printf (x, __VA_ARGS__)
#else
#define HTTP_TRACE(x)
#define HTTP_TRACE_ARGS(x, ...)
#endif

#ifdef HTTP_DEBUG_ERRORS
#define HTTP_ERROR(x) fputs (x, stdout)
#define HTTP_ERROR_ARGS(x, ...) printf (x, __VA_ARGS__)
#else
#define HTTP_ERROR(x)
#define HTTP_ERROR_ARGS(x, ...)
#endif

#define MSEC_PER_SEC 1000

void http_print_error      ( int error )
{
    switch ( error )
    {
        case HTTP_ERR_OK          : printf("No error, all OK"); break;
        case HTTP_ERR_UNKNOWN_HOST: printf("Unknown host"); break;
        case HTTP_ERR_CONNECT     : printf("Connect error"); break;
        case HTTP_ERR_SEND        : printf("Send error"); break;
        case HTTP_ERR_READ        : printf("Read error"); break;
        case HTTP_ERR_HTTP        : printf("Could not understand response from server"); break;
        case HTTP_ERR_RESOURCE    : printf("Could not acquire resources required (sockets, memory, etc.)"); break;
        default                   : printf("Unknown error %d", error );
    }
    printf(", errno = %d\n", errno );
}

void http_print_status ( int status )
{
    switch ( status )
    {
        case HTTP_STATUS_NONE            : printf("No status received"); break;
        case HTTP_STATUS_READ_OK         : printf("OK"); break;
        case HTTP_STATUS_CREATE_OK       : printf("Created"); break;
        case HTTP_STATUS_MOVED           : printf("Moved Permanently"); break;
        case HTTP_STATUS_FOUND           : printf("Found"); break;
        case HTTP_STATUS_INVALID         : printf("Bad Request"); break;
        case HTTP_STATUS_FORBIDDEN       : printf("Forbidden"); break;
        case HTTP_STATUS_NOT_FOUND       : printf("Not Found"); break;
        case HTTP_STATUS_TIMEOUT         : printf("Request Timeout"); break;
        case HTTP_STATUS_SERVER          : printf("Internal Server Error"); break;
        case HTTP_STATUS_NOT_IMPLEMENTED : printf("Not Implemented"); break;
        case HTTP_STATUS_OVERLOADED      : printf("Service Unavailable"); break;
        default                          : printf("Unknown code"); break;
    }
    printf(", status = %d\n", status );
}

void http_initialise ( http_t* http )
{
    http->response_status = HTTP_STATUS_NONE;
    http->response_transfer_encoding[0] = 0;
    http->response_content_type[0] = 0;
    http->response_location[0] = 0;
    http->response_timeout_ms = 60 * MSEC_PER_SEC;
    http->data_available = false;
    http->request_method = HTTP_METHOD_GET;
    http->request_url = NULL;
    http->request_content = NULL;
    http->request_content_type = NULL;
    http->request_content_length = 0;
    http->chunked = false;
    http->length_left = 0;
    http->request_extra_headers[0] = 0;
    http->header_buffer[0] = 0;
}

static void backoff ( int* backoff_delay )
{
    struct timespec ts = { *backoff_delay, 0 };
    nanosleep(&ts, NULL);
    *backoff_delay *= 2;
}

static int http_connect ( http_t* http )
{
    struct hostent* hp;
    struct sockaddr_in server;
    int backoff_delay;
    int attempts;
    int err;

    HTTP_TRACE_ARGS("HTTP Trace: Connecting to port %d on %s\n", http->request_url->port, http->request_url->host);
    attempts = 0;
    backoff_delay = HTTP_INITIAL_BACKOFF_S;
    hp = NULL;
    while ( attempts < HTTP_DNS_ATTEMPTS )
    {
        attempts++;
        HTTP_TRACE_ARGS("HTTP Trace: Attempting DNS lookup, attempt %d of %d\n", attempts, HTTP_DNS_ATTEMPTS);
        hp = lwip_gethostbyname(http->request_url->host);
        if ( !hp && attempts < HTTP_DNS_ATTEMPTS )
        {
            HTTP_ERROR_ARGS("HTTP Error: DNS lookup attempt %d failed (%d), backing off\n", attempts, errno);
            backoff ( &backoff_delay );
        }
        else if ( hp )
            break;
    }
    if (hp)
    {
        memcpy((char *) &server.sin_addr, hp->h_addr_list[0], hp->h_length);
        server.sin_family = (unsigned char) hp->h_addrtype;
        server.sin_port   = htons( http->request_url->port );
        HTTP_TRACE("HTTP Trace: IP Address to connect to: ");
        #ifdef HTTP_DEBUG_TRACE
            print_ip_address(false, (struct ip_addr*) &server.sin_addr);
            printf("\n");
        #endif
    }
    else
    {
        HTTP_ERROR_ARGS("HTTP Error: Could not resolve host name %s (%d)\n", http->request_url->host, errno);
        return HTTP_ERR_UNKNOWN_HOST;
    }

    attempts = 0;
    backoff_delay = HTTP_INITIAL_BACKOFF_S;
    while ( attempts < HTTP_CONNECT_ATTEMPTS )
    {
        http->socket = socket(AF_INET, SOCK_STREAM, 0);
        if (http->socket < 0)
        {
            HTTP_ERROR("HTTP Error: Could not get socket\n");
            return HTTP_ERR_RESOURCE;
        }

        err = setsockopt(http->socket,SOL_SOCKET,SO_RCVTIMEO,&http->response_timeout_ms,sizeof(int));
        if (err != ERR_OK)
            HTTP_ERROR("WARNING: Failed to set HTTP response timeout");

        // Connect to server
        HTTP_TRACE_ARGS("HTTP Trace: Connecting, attempt %d of %d\n", attempts+1, HTTP_CONNECT_ATTEMPTS);
        if ( connect(http->socket, (const struct sockaddr *) &server, sizeof(server)) >= 0 )
            break;
        attempts++;
        if ( attempts < HTTP_CONNECT_ATTEMPTS )
        {
            HTTP_ERROR_ARGS("HTTP Error: Connect attempt %d failed (%d), backing off\n", attempts, errno);
            http_close ( http );
            backoff ( &backoff_delay );
        }
    }
    if ( attempts >= HTTP_CONNECT_ATTEMPTS )
    {
        http_close(http);
        HTTP_ERROR_ARGS("HTTP Error: Could not connect to server (%d)\n", errno);
        return HTTP_ERR_CONNECT;
    }

    return HTTP_ERR_OK;
}

static int http_send_line ( http_t* http, const char* line )
{
    int line_len;
    int sent_len;

    if ( !line[0] ) return HTTP_ERR_OK;

    HTTP_TRACE_ARGS("HTTP Trace: Sending line: %s", line);
    line_len = strlen ( line );
    sent_len   = write ( http->socket, line, line_len );
    if ( sent_len != line_len)
    {
        HTTP_ERROR("HTTP Error: Failed to send line\n");
        return HTTP_ERR_SEND;
    }
    return HTTP_ERR_OK;
}

static int http_send_string_header ( http_t*     http,
                                     const char* header_name,
                                     const char* header_value )
{
    sprintf ( http->header_buffer, "%s: %s\r\n", header_name, header_value );
    return http_send_line ( http, http->header_buffer );
}

static int http_send_integer_header ( http_t*     http,
                                      const char* header_name,
                                      int         header_value )
{
    sprintf ( http->header_buffer, "%s: %d\r\n", header_name, header_value );
    return http_send_line ( http, http->header_buffer );
}

static int http_send_headers_and_data ( http_t* http )
{
    int sent_len;
    int error;
    char* method;

    // Write request and headers
    switch ( http->request_method )
    {
        case HTTP_METHOD_GET : method = "GET" ; break;
        case HTTP_METHOD_POST: method = "POST"; break;
        default              : method = "GET" ; break;
    }
    sprintf ( http->header_buffer, "%s %s HTTP/1.1\r\n", method, http->request_url->file );
    error = http_send_line ( http, http->header_buffer );
    if ( !error ) error = http_send_string_header ( http, "User-Agent", HTTP_USER_AGENT );
    if ( !error ) error = http_send_string_header ( http, "Host", http->request_url->host );
    if ( http->request_content )
    {
        if ( !error && http->request_content_type) error = http_send_string_header ( http, "Content-Type", http->request_content_type );
        if ( !error ) error = http_send_integer_header ( http, "Content-Length", http->request_content_length );
    }
    if ( !error ) error = http_send_string_header ( http, "Connection", "Close" );
    if ( !error ) error = http_send_line ( http, http->request_extra_headers );
    if ( !error ) error = http_send_line ( http, "\r\n" );

    if ( !error && http->request_content )
    {
        HTTP_TRACE_ARGS("HTTP Trace: Sending %d bytes of content\n", http->request_content_length);
        sent_len = write ( http->socket, http->request_content, http->request_content_length );
        if ( sent_len != http->request_content_length )
        {
            HTTP_ERROR("HTTP Error: Failed to send all the content\n");
            error = HTTP_ERR_SEND;
        }
    }

    return error;
}

static int http_read_status ( http_t* http )
{
    int status_len;
    HTTP_TRACE("HTTP Trace: Reading status\n");
    status_len = socket_read_line( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
    if ( status_len < 0 )
    {
        HTTP_ERROR("HTTP Error: Failed to read status\n");
        return HTTP_ERR_READ;
    }
    HTTP_TRACE("HTTP Trace: Status from server: ");
    HTTP_TRACE(http->header_buffer);
    HTTP_TRACE("\n");
    if ( sscanf ( http->header_buffer, "HTTP/1.%*d %03d", &http->response_status) != 1)
    {
        HTTP_ERROR_ARGS("HTTP Error: Could not understand status reply from server: %s\n", http->header_buffer);
        return HTTP_ERR_HTTP;
    }
    return HTTP_ERR_OK;
}

static int http_parse_headers ( http_t* http )
{
    int header_len;
    char* seperator;

    // Set data_available true here, if no content length is received it means
    // data *might* be available. We only know for sure that no data is available
    // if we get a content length of zero
    http->data_available = true;

    HTTP_TRACE("HTTP Trace: Parsing headers\n");
    header_len = socket_read_line ( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
    if ( header_len < 0 )
    {
        HTTP_ERROR("HTTP Error: Could not read first header\n");
        return HTTP_ERR_READ;
    }
    while ( header_len > 2 )
    {
        HTTP_TRACE_ARGS("HTTP Trace: Got header of length %3d: %s\n", header_len, http->header_buffer);
        seperator = strchr ( http->header_buffer, ':' );
        if ( seperator == NULL)
        {
            HTTP_ERROR("HTTP Error: Could not find ':' in non-empty header\n");
            return HTTP_ERR_HTTP;
        }
        *seperator++ = 0;
        while ( *seperator == ' ' )
            seperator++;
        if ( strcasecmp ( http->header_buffer, HTTP_LOCATION_HEADER ) == 0 )
        {
            strcpy ( http->response_location, seperator );
            HTTP_TRACE_ARGS("HTTP Trace: Found location: %s\n", http->response_location);
        }
        else if ( strcasecmp ( http->header_buffer, HTTP_TRANSFER_ENCODING_HEADER ) == 0 )
        {
            strcpy ( http->response_transfer_encoding, seperator );
            HTTP_TRACE_ARGS("HTTP Trace: Found transfer encoding: %s\n", http->response_transfer_encoding);
            http->chunked = ( strncasecmp ( http->response_transfer_encoding, HTTP_ENCODING_CHUNKED, strlen(HTTP_ENCODING_CHUNKED) ) == 0 );
            HTTP_TRACE_ARGS("HTTP Trace: chunked is %d\n", http->chunked);
        }
        else if ( strcasecmp ( http->header_buffer, HTTP_CONTENT_TYPE_HEADER ) == 0 )
        {
            strcpy ( http->response_content_type, seperator );
            HTTP_TRACE_ARGS("HTTP Trace: Found content-type: %s\n", http->response_content_type);
        }
        else if ( strcasecmp ( http->header_buffer, HTTP_CONTENT_LENGTH_HEADER ) == 0 )
        {
            if ( sscanf ( seperator, "%d", &http->length_left ) != 1)
            {
                HTTP_ERROR_ARGS("HTTP Error: Could not understand content-length: %s\n", seperator);
                return HTTP_ERR_HTTP;
            }
            else
            {
                HTTP_TRACE_ARGS("HTTP Trace: Found content-length: %d\n", http->length_left);
                http->data_available = http->length_left > 0;
            }
        }
        header_len = socket_read_line ( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
        if ( header_len < 0 )
        {
            HTTP_ERROR("HTTP Error: Could not read header\n");
            return HTTP_ERR_READ;
        }
    }
    return HTTP_ERR_OK;
}

int http_begin_request ( http_t*       http,
                         url_t*        url,
                         http_method_t method )
{
    http_close ( http );
    http_initialise ( http );
    http->request_url = url;
    http->request_method = method;
    return HTTP_ERR_OK;
}

int http_set_header ( http_t*   http,
                      char*     header_name,
                      char*     header_value )
{
    char* extra_headers_ptr;
    int extra_headers_length;

    extra_headers_length = strlen ( http->request_extra_headers );
    extra_headers_ptr = &http->request_extra_headers[extra_headers_length];

    sprintf ( http->request_extra_headers, "%s: %s\r\n", header_name, header_value );
    return HTTP_ERR_OK;
}

int http_set_content ( http_t* http,
                       char*   content_type,
                       int     content_length,
                       char*   content )
{
    http->request_content_type   = content_type;
    http->request_content        = content;
    http->request_content_length = content_length;
    return HTTP_ERR_OK;
}

int http_send_request ( http_t* http )
{
    int error;

                error = http_connect                ( http );
    if (!error) error = http_send_headers_and_data  ( http );
    if (!error) error = http_read_status            ( http );
    if (!error) error = http_parse_headers          ( http );

    return error;
}

static int http_read_length ( http_t*   http,
                              char*     content,
                              int       max_length,
                              int*      length_got )
{
    int length_to_read;
    int got_this_read;
    length_to_read = http->length_left;
    if ( length_to_read > max_length )
        length_to_read = max_length;
    HTTP_TRACE_ARGS("HTTP Trace: Reading %d\n", length_to_read);
    got_this_read = socket_read_count ( http->socket, content, length_to_read );
    if ( got_this_read > 0 )
    {
        *length_got = got_this_read;
        content[*length_got] = 0;
        HTTP_TRACE_ARGS("HTTP Trace: Got %d: %s\n", *length_got, content);
        http->length_left -= *length_got;
        if ( length_to_read != *length_got )
        {
            HTTP_ERROR_ARGS("HTTP Error: Expected to read %d, Got %d\n", length_to_read, *length_got);
            http->data_available = false;
            return HTTP_ERR_READ;
        }
    }
    else
    {
        HTTP_ERROR("HTTP Error: Could not read any data");
        return HTTP_ERR_READ;
    }
    return HTTP_ERR_OK;
}

int http_read ( http_t*        http,
                char*          content,
                int            max_length,
                int*           length_got )
{
    int got_this_read;
    char* content_ptr;
    int error = HTTP_ERR_OK;
    if ( http->chunked )
    {
        HTTP_TRACE("HTTP Trace: Reading chunks\n");
        *length_got = 0;
        content_ptr = content;
        if ( http->length_left > 0 )
        {
            HTTP_TRACE_ARGS("HTTP Trace: Reading chunk residue %d bytes\n", http->length_left);
            error = http_read_length ( http, content, max_length, &got_this_read );
            content_ptr += got_this_read;
            *length_got += got_this_read;
            if ( !error && http->length_left == 0 )
            {
                // Read the post-chunk CRLF
                socket_read_line ( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
            }
        }
        while ( !error && http->data_available && http->length_left == 0 )
        {
            HTTP_TRACE("HTTP Trace: Reading next chunk size\n");
            got_this_read = socket_read_line ( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
            if ( got_this_read == 0 )
            {
                HTTP_ERROR("HTTP Error: Chunk size not found\n");
                http->data_available = false;
                error = HTTP_ERR_READ;
            }
            if ( !error )
            {
                HTTP_TRACE_ARGS("HTTP Trace: Chunk size: %s\n", http->header_buffer);
                if ( sscanf ( http->header_buffer, "%x", &http->length_left ) != 1 )
                {
                    HTTP_ERROR("HTTP Error: Could not parse chunk size\n");
                    http->data_available = false;
                    error = HTTP_ERR_HTTP;
                }
                else
                {
                    http->data_available = http->length_left > 0;
                    if ( http->length_left > 0 )
                    {
                        error = http_read_length ( http, content_ptr, max_length - *length_got, &got_this_read );
                        content_ptr += got_this_read;
                        *length_got += got_this_read;
                    }
                    if ( !error && http->length_left == 0 )
                    {
                        // Read the post-chunk CRLF
                        socket_read_line ( http->socket, http->header_buffer, HTTP_HEADER_BUFFER_SIZE );
                    }
                }
            }
        }
    }
    else if ( http->length_left > 0 )
    {
        HTTP_TRACE_ARGS("HTTP Trace: Non-zero content length, %d remaining\n", http->length_left);
        error = http_read_length ( http, content, max_length, length_got );
        http->data_available = http->data_available && http->length_left > 0;
    }
    else
    {
        HTTP_TRACE_ARGS("HTTP Trace: Content length unknown, reading %d bytes or until connection closed\n", max_length);
        got_this_read = socket_read_count ( http->socket, content, max_length );
        if ( got_this_read > 0 )
        {
            HTTP_TRACE_ARGS("HTTP Trace: Read %d bytes\n", got_this_read);
            http->data_available = ( got_this_read == max_length );
            *length_got = got_this_read;
        }
        else
        {
            HTTP_TRACE("HTTP Trace: Got no data\n");
            http->data_available = false;
            *length_got = 0;
        }
    }
    content[*length_got] = 0;
    return error;
}

void http_close ( http_t* http )
{
    if (http && http->socket != LWIP_INVALID_SOCKET)
    {

        close(http->socket);
        http->socket = LWIP_INVALID_SOCKET;
    }
}

