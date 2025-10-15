#include "http_p.h"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#define HTTP_DELIMETER "\r\n"

static enum http_method 
get_method_enum(std::string m) 
{
    if (m == "GET") {
        return GET;
    } 
    else if (m == "POST") {
        return POST;
    } 
    else if (m == "DELETE") {
        return DELETE;
    }
    return UNRECOGNIZED;
}

static std::string
get_status_message(enum http_status_code c) 
{
    switch (c) {
    case SWITCHING_PROTOCOL:
        return "Switching Protocols";
    case OK:
        return "OK";
    }

    return "What?";
}

static int 
parse_request_line(struct http_request* hr, std::string line) 
{
    // Request line must contain 3 elements
    std::vector<std::string> reqs;
    std::stringstream ss(line);

    std::string token;

    while (std::getline(ss, token, ' ')) {
        reqs.push_back(token);
    }

    if (reqs.size() != 3) {
        return 1;
    }

    hr->method = get_method_enum(reqs[0]);
    hr->path = reqs[1].c_str();
    hr->protocol_v = reqs[2].c_str();

    return 0;
}

static void
parse_header_line(struct http_request* req, std::string line) 
{
    size_t colon_pos;

    std::string header;
    std::string content;

    colon_pos = line.find(':');
    header = line.substr(0, colon_pos);
    content = line.substr(colon_pos+1, std::string::npos);

    // WARNING!!! Here is no resizing yet
    req->headers[req->headers_length].name = strdup(header.c_str());
    req->headers[req->headers_length].data = strdup(content.c_str());
    req->headers_length++;
}

const char*
http_get_request_param(struct http_request req, const char* name) 
{
    // I realy don't want to put a hashmap 
    // instead of struct http_header array, so this will be just a for loop
    for (size_t i = 0; i < req.headers_length; i++) {
        if (strcmp(req.headers[i].name, name) == 0) {
            return req.headers[i].data;
        }
    }

    return NULL;
}

struct http_request 
parse_http_request(char* req) 
{
    struct http_request hr = {0};
    hr.headers = new http_header[DEFAULT_HEADERS_SIZE];
    hr.headers_length = 0;

    char* line = std::strtok(req, HTTP_DELIMETER);

    // Parsing request line (GET /home HTTP/1.1)
    if (parse_request_line(&hr, line) != 0) {
        hr.protocol_v = NULL;
        return hr;
    }
    // Skipping the line
    line = std::strtok(nullptr, HTTP_DELIMETER);
 
    while (line != nullptr) {
        parse_header_line(&hr, line);
        line = std::strtok(nullptr, HTTP_DELIMETER);
    }

    return hr;
}

struct http_response
new_http_response(enum http_status_code code) 
{
    struct http_response res = {0};

    res.headers = new http_header[DEFAULT_HEADERS_SIZE];
    res.protocol_v = HTTP_PROTOCOL;
    res.code = code;

    return res;
}

void
http_response_append_header(
    struct http_response* res, const char* name, const char* data) 
{
    res->headers[res->headers_length].name = strdup(name);
    res->headers[res->headers_length].data = strdup(data);
    res->headers_length++;
}

const char*
http_compose_response(struct http_response res) 
{
    std::stringstream ss;

    ss << res.protocol_v << " ";
    ss << res.code << " ";
    ss << get_status_message(res.code) << HTTP_DELIMETER;

    for (size_t i = 0; i < res.headers_length; i++) {
        auto header = res.headers[i];

        ss << header.name << ": " << header.data
           << HTTP_DELIMETER;
    }

    ss << HTTP_DELIMETER;

    std::string out = ss.str();
    return strdup(out.c_str());
}

void 
destroy_request(struct http_request req) 
{
    // TODO, make all the request object be dynamic
    delete req.headers;
}

void 
destroy_response(struct http_response res) 
{
    delete res.headers;
}