#ifndef _HTTP_PARSER_H
#include <stdlib.h>
#define MAX_HEADER_NAME_SIZE 256

enum http_method {
    GET,
    POST,
    DELETE,
    UNRECOGNIZED
};

struct http_header {
    char  name[MAX_HEADER_NAME_SIZE];
    char* data;
};

struct http_request {
    const char* path;
    const char* protocol_v;
    enum http_method  method;
    struct http_header* headers;
    size_t headers_length;
};

struct http_response {

};

// To identify the error, see if protocol_v != NULL
struct http_request parse_http_request(char* req);

#endif // _HTTP_PARSER_H