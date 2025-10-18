#ifndef _HTTP_PARSER_H
#include <stdlib.h>

#define DEFAULT_HEADERS_SIZE 40
#define HTTP_PROTOCOLV "HTTP/1.1"

typedef char* http_method_t;

typedef enum http_status_code {
    SWITCHING_PROTOCOL = 101,
    OK = 200, 
} http_status_code_t;

struct http_header {
    char* name;
    char* data;
};

struct http_headers_array {
    struct http_header* arr;
    size_t length;
    size_t capacity;
};

struct http_request {
    http_method_t method;
    char* path;
    struct http_headers_array headers;
};

struct http_response {
    http_status_code_t status;
    struct http_headers_array headers;
};

/*
 * Returns value of the header with given name.
 */
const char* http_get_header_value(struct http_request req, const char* name);

/*
 * Returns new http_request based on the given raw message
 */
struct http_request http_parse_request(char* message);

/*
 * Initializes new http_response with provided status code
 * Sets only response status to new response. Without any headers
 */
struct http_response http_new_response(http_status_code_t status);

/*
 * Appends new header to response based on name
 * and its coresponding data.
 */
void http_response_append_header(struct http_response* res, const char* name, const char* data);

/*
 * Returns a string representing the HTTP response
 * based on the given http_response structure 
 */
char* http_compose_response(struct http_response res);

/*
 * Frees the given request
 */
void http_destroy_request(struct http_request req);

/*
 * Frees the given response
 */
void http_destroy_response(struct http_response res);

#endif // _HTTP_PARSER_H