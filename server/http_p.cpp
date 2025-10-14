#include "http_p.h"
#include <cstddef>
#include <cstring>
#include <iostream>
#include <ostream>
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

    std::string item;

    while (std::getline(ss, item, ' ')) {
        reqs.push_back(item);
    }

    if (reqs.size() != 3) {
        return 1;
    }

    hr->method = get_method_enum(reqs[0]);
    hr->path = reqs[1].c_str();
    hr->protocol_v = reqs[2].c_str(); // We don't really care about protocoll

    return 0;
}

struct http_request 
parse_http_request(char* req) 
{
    struct http_request hr = {0};
    char*  line = std::strtok(req, HTTP_DELIMETER);

    // Parsing request line (GET / HTTP/1.1)
    if (parse_request_line(&hr, line) != 0) {
        hr.protocol_v = NULL;
        return hr;
    }

    size_t line_count = 0;

    while (line != nullptr) {
        ++line_count;
        std::cout << line << std::endl;

        line = strtok(nullptr, HTTP_DELIMETER);
    }

    return hr;
}