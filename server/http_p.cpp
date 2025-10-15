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
    content = line.substr(colon_pos + 2, std::string::npos);

    req->headers[req->headers_length].name = strdup(header.c_str());
    req->headers[req->headers_length].data = strdup(content.c_str());
    req->headers_length++;
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