#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "include/http_p.h"

#define HTTP_DELIMETER "\r\n"

static std::string 
trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n");
    size_t end = str.find_last_not_of(" \t\n");
    
    if (start == std::string::npos) {
        return "";
    }
    
    return str.substr(start, end - start + 1);
}

static std::string
get_status_message(enum http_status_code c) 
{
    switch (c) 
    {
    case SWITCHING_PROTOCOL:
        return "Switching Protocols";
    case OK:
        return "OK";
    default:
        return "What?";
    }
}

static void
resize_headers_array(struct http_headers_array* harr)
{
    size_t new_cap = harr->capacity * 2;
    auto new_arr = new http_header[new_cap];

    std::memcpy(new_arr, harr->arr, harr->length * sizeof(http_header));

    delete[] harr->arr;
    harr->arr = new_arr;
    harr->capacity = new_cap;
}

static void 
parse_request_line(struct http_request& req, std::string line) 
{
    std::vector<std::string> reqs;
    std::stringstream ss(line);

    std::string token;

    while (std::getline(ss, token, ' ')) {
        reqs.push_back(token);
    }

    req.method = strdup(reqs[0].c_str());
    req.path = strdup(reqs[1].c_str());
}

static void
parse_header_line(struct http_request& req, std::string line) 
{
    size_t colon_pos;
    size_t length = req.headers.length;
    
    std::string header_name;
    std::string header_content;

    colon_pos = line.find(':');
    header_name = line.substr(0, colon_pos);
    header_content = line.substr(colon_pos+1, std::string::npos);

    header_name = trim(header_name);
    header_content = trim(header_content);

    if (req.headers.length + 1 > req.headers.capacity) {
        resize_headers_array(&req.headers);
    }

    req.headers.arr[length].name = strdup(header_name.c_str());
    req.headers.arr[length].data = strdup(header_content.c_str());
    req.headers.length++;
}

const char*
http_get_header_value(struct http_request req, const char* name)
{
    // I realy don't want to put a hashmap 
    // so this will be just a for loop
    for (size_t i = 0; i < req.headers.length ; i++) {
        if (strcmp(req.headers.arr[i].name, name) == 0) {
            return req.headers.arr[i].data;
        }
    }

    return NULL;
}

struct http_request 
http_parse_request(char* message) 
{
    struct http_request req;

    req.headers.arr = new http_header[DEFAULT_HEADERS_SIZE];
    req.headers.capacity = DEFAULT_HEADERS_SIZE;
    req.headers.length = 0;

    char* line = std::strtok(message, HTTP_DELIMETER);

    // Parsing request line (GET /home HTTP/1.1)
    parse_request_line(req, line);

    // Skipping the line
    line = std::strtok(nullptr, HTTP_DELIMETER);
 
    while (line != nullptr) {
        parse_header_line(req, line);
        line = std::strtok(nullptr, HTTP_DELIMETER);
    }

    return req;
}

struct http_response
http_new_response(http_status_code_t status) 
{
    struct http_response res;

    res.headers.arr = new http_header[DEFAULT_HEADERS_SIZE];
    res.headers.capacity = DEFAULT_HEADERS_SIZE;
    res.headers.length = 0;

    res.status = status;

    return res;
}

void
http_response_append_header(
    struct http_response* res, const char* name, const char* data) 
{
    size_t length = res->headers.length;

    if (res->headers.length + 1 > res->headers.capacity) {
        resize_headers_array(&res->headers);
    }

    res->headers.arr[length].name = strdup(name);
    res->headers.arr[length].data = strdup(data);
    res->headers.length++;
}

const char*
http_compose_response(struct http_response res) 
{
    std::stringstream ss;

    ss << HTTP_PROTOCOLV << " ";
    ss << res.status << " ";
    ss << get_status_message(res.status) << HTTP_DELIMETER;

    for (size_t i = 0; i < res.headers.length; i++) {
        auto header = res.headers.arr[i];

        ss << header.name << ": " << header.data
           << HTTP_DELIMETER;
    }

    // ss << HTTP_DELIMETER;

    std::string out = ss.str();
    return strdup(out.c_str());
}

static void
destroy_headers(struct http_headers_array harr) 
{
    for (size_t i = 0; i < harr.length; i++) {
        std::free(harr.arr[i].name);
        std::free(harr.arr[i].data);
    }
}

void 
http_destroy_request(struct http_request req) 
{
    destroy_headers(req.headers);

    delete [] req.headers.arr;
    std::free(req.method);
    std::free(req.path);
}

void 
http_destroy_response(struct http_response res) 
{
    destroy_headers(res.headers);
}