#pragma once

#include <string>

namespace NUI {

static const std::string HTTP_INDEX = R"(
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Content-Length: )";

static const std::string HTTP_LIST = R"(
HTTP/1.1 200 OK
Content-Type: application/json; charset=UTF-8
Content-Length: )";

static const std::string HTTP_STATUS = R"(
HTTP/1.1 200 OK
Content-Type: text/plain; charset=UTF-8
Content-Length: )";

static const std::string HTTP_OK = R"(
HTTP/1.1 200 OK
Content-Length: " )";

}
