#include "http_message.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

using namespace std;

string to_string(HttpMethod method) {
    switch (method) {
        case HttpMethod::GET:
            return "GET";
        case HttpMethod::HEAD:
            return "HEAD";
        case HttpMethod::POST:
            return "POST";
        case HttpMethod::PUT:
            return "PUT";
        case HttpMethod::DELETE:
            return "DELETE";
        case HttpMethod::CONNECT:
            return "CONNECT";
        case HttpMethod::OPTIONS:
            return "OPTIONS";
        case HttpMethod::TRACE:
            return "TRACE";
        case HttpMethod::PATCH:
            return "PATCH";
        default:
            return string();
    }
}

string to_string(HttpVersion version) {
    switch (version) {
        case HttpVersion::HTTP_0_9:
            return "HTTP/0.9";
        case HttpVersion::HTTP_1_0:
            return "HTTP/1.0";
        case HttpVersion::HTTP_1_1:
            return "HTTP/1.1";
        case HttpVersion::HTTP_2_0:
            return "HTTP/2.0";
        default:
            return string();
    }
}

string to_string(HttpStatusCode status_code) {
    switch (status_code) {
        case HttpStatusCode::Continue:
            return "Continue";
        case HttpStatusCode::Ok:
            return "OK";
        case HttpStatusCode::Accepted:
            return "Accepted";
        case HttpStatusCode::MovedPermanently:
            return "Moved Permanently";
        case HttpStatusCode::Found:
            return "Found";
        case HttpStatusCode::BadRequest:
            return "Bad Request";
        case HttpStatusCode::Forbidden:
            return "Forbidden";
        case HttpStatusCode::NotFound:
            return "Not Found";
        case HttpStatusCode::MethodNotAllowed:
            return "Method Not Allowed";
        case HttpStatusCode::ImATeapot:
            return "I'm a Teapot";
        case HttpStatusCode::InternalServerError:
            return "Internal Server Error";
        case HttpStatusCode::NotImplemented:
            return "Not Implemented";
        case HttpStatusCode::BadGateway:
            return "Bad Gateway";
        default:
            return string();
    }
}

HttpMethod string_to_method(const string &method_string) {
    string method_string_uppercase;
    transform(method_string.begin(), method_string.end(),
                   back_inserter(method_string_uppercase),
                   [](char c) { return toupper(c); });
    if (method_string_uppercase == "GET") {
        return HttpMethod::GET;
    } else if (method_string_uppercase == "HEAD") {
        return HttpMethod::HEAD;
    } else if (method_string_uppercase == "POST") {
        return HttpMethod::POST;
    } else if (method_string_uppercase == "PUT") {
        return HttpMethod::PUT;
    } else if (method_string_uppercase == "DELETE") {
        return HttpMethod::DELETE;
    } else if (method_string_uppercase == "CONNECT") {
        return HttpMethod::CONNECT;
    } else if (method_string_uppercase == "OPTIONS") {
        return HttpMethod::OPTIONS;
    } else if (method_string_uppercase == "TRACE") {
        return HttpMethod::TRACE;
    } else if (method_string_uppercase == "PATCH") {
        return HttpMethod::PATCH;
    } else {
        throw invalid_argument("Unexpected HTTP method");
    }
}

HttpVersion string_to_version(const string &version_string) {
    string version_string_uppercase;
    transform(version_string.begin(), version_string.end(),
                   back_inserter(version_string_uppercase),
                   [](char c) { return toupper(c); });
    if (version_string_uppercase == "HTTP/0.9") {
        return HttpVersion::HTTP_0_9;
    } else if (version_string_uppercase == "HTTP/1.0") {
        return HttpVersion::HTTP_1_0;
    } else if (version_string_uppercase == "HTTP/1.1") {
        return HttpVersion::HTTP_1_1;
    } else if (version_string_uppercase == "HTTP/2" ||
               version_string_uppercase == "HTTP/2.0") {
        return HttpVersion::HTTP_2_0;
    } else {
        throw invalid_argument("Unexpected HTTP version");
    }
}

string to_string(const HttpRequest &request) {
    ostringstream oss;

    oss << to_string(request.method()) << ' ';
    oss << request.uri().path() << ' ';
    oss << to_string(request.version()) << "\r\n";
    for (const auto &p: request.headers())
        oss << p.first << ": " << p.second << "\r\n";
    oss << "\r\n";
    oss << request.content();

    return oss.str();
}

string to_string(const HttpResponse &response, bool send_content) {
    ostringstream oss;

    oss << to_string(response.version()) << ' ';
    oss << static_cast<int>(response.status_code()) << ' ';
    oss << to_string(response.status_code()) << "\r\n";
    for (const auto &p: response.headers())
        oss << p.first << ": " << p.second << "\r\n";
    oss << "\r\n";
    if (send_content) oss << response.content();

    return oss.str();
}

HttpRequest string_to_request(const string &request_string) {
    string start_line, header_lines, message_body;
    istringstream iss;
    HttpRequest request;
    string line, method, path, version;  // used for first line
    string key, value;                   // used for header fields
    Uri uri;
    size_t lpos = 0, rpos = 0;

    rpos = request_string.find("\r\n", lpos);
    if (rpos == string::npos) {
        throw invalid_argument("Could not find request start line");
    }

    start_line = request_string.substr(lpos, rpos - lpos);
    lpos = rpos + 2;
    rpos = request_string.find("\r\n\r\n", lpos);
    if (rpos != string::npos) {  // has header
        header_lines = request_string.substr(lpos, rpos - lpos);
        lpos = rpos + 4;
        rpos = request_string.length();
        if (lpos < rpos) {
            message_body = request_string.substr(lpos, rpos - lpos);
        }
    }

    iss.clear();  // parse the start line
    iss.str(start_line);
    iss >> method >> path >> version;
    if (!iss.good() && !iss.eof()) {
        throw invalid_argument("Invalid start line format");
    }
    request.SetMethod(string_to_method(method));
    request.SetUri(Uri(path));
    if (string_to_version(version) != request.version()) {
        throw logic_error("HTTP version not supported");
    }

    iss.clear();  // parse header fields
    iss.str(header_lines);
    while (getline(iss, line)) {
        istringstream header_stream(line);
        getline(header_stream, key, ':');
        getline(header_stream, value);

        // remove whitespaces from the two strings
        key.erase(remove_if(key.begin(), key.end(),
                                 [](char c) { return isspace(c); }),
                  key.end());
        value.erase(remove_if(value.begin(), value.end(),
                                   [](char c) { return isspace(c); }),
                    value.end());
        request.SetHeader(key, value);
    }

    request.SetContent(message_body);

    return request;
}

HttpResponse string_to_response(const string &response_string) {
    throw logic_error("Method not implemented");
}