#ifndef _HTTP_PARSER_H
#include <stdlib.h>

#define DEFAULT_HEADERS_SIZE 40
#define MAX_HEADER_NAME 256
#define MAX_HEADER_DATA 512
#define HTTP_PROTOCOL "HTTP/1.1"

enum http_method {
    GET,
    POST,
    DELETE,
    UNRECOGNIZED
};

enum http_status_code {
    SWITCHING_PROTOCOL = 101,
    OK = 200, 
};

struct http_header {
    char* name;
    char* data;
};

struct http_request {
    const char* path;
    const char* protocol_v;
    enum http_method method;
    struct http_header* headers; // Array
    size_t headers_length;
};

struct http_response {
    const char* protocol_v;
    enum http_status_code code;
    struct http_header* headers; // Array
    size_t headers_length;
};

// To identify the error, see if protocol_v != NULL
struct http_request parse_http_request(char* req);

struct http_response new_http_response(enum http_status_code code); 

void http_response_append_header(struct http_response* res, const char* name, const char* data);

const char* http_compose_response(struct http_response res);

const char* http_get_request_param(struct http_request req, const char* name);

// Frees request
void destroy_request(struct http_request req);

#endif // _HTTP_PARSER_H