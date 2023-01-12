// Defines objects that represents HTTP request and response
// and some utility functions to manipulate HTTP data

#ifndef HTTP_MESSAGE_H_
#define HTTP_MESSAGE_H_

#include <map>
#include <string>
#include <utility>

#include "uri.h"

using namespace std;

enum class HttpMethod {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH
};
enum class HttpVersion {
    HTTP_0_9 = 9,
    HTTP_1_0 = 10,
    HTTP_1_1 = 11,
    HTTP_2_0 = 20
};
enum class HttpStatusCode {
    Continue = 100,
    SwitchingProtocols = 101,
    EarlyHints = 103,
    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    NotModified = 304,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    RequestTimeout = 408,
    ImATeapot = 418,
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnvailable = 503,
    GatewayTimeout = 504,
    HttpVersionNotSupported = 505
};

// Utility functions to convert between string or integer to enum classes
string to_string(HttpMethod method);
string to_string(HttpVersion version);
string to_string(HttpStatusCode status_code);

HttpMethod string_to_method(const string &method_string);
HttpVersion string_to_version(const string &version_string);

class HttpMessageInterface {
public:
    HttpMessageInterface() : version_(HttpVersion::HTTP_1_1) {}
    virtual ~HttpMessageInterface() = default;

    void SetHeader(const string &key, const string &value) {
        headers_[key] = move(value);
    }
    void RemoveHeader(const string &key) { headers_.erase(key); }
    void ClearHeader() { headers_.clear(); }
    void SetContent(const string &content) {
        content_ = move(content);
        SetContentLength();
    }
    void ClearContent(const string &content) {
        content_.clear();
        SetContentLength();
    }

    HttpVersion version() const { return version_; }
    string header(const string &key) const {
        if (headers_.count(key) > 0) return headers_.at(key);
        return string();
    }
    map <string, string> headers() const { return headers_; }
    string content() const { return content_; }
    size_t content_length() const { return content_.length(); }

protected:
    HttpVersion version_;
    map <string, string> headers_;
    string content_;

    void SetContentLength() {
        SetHeader("Content-Length", to_string(content_.length()));
    }
};
class HttpRequest : public HttpMessageInterface {
public:
    HttpRequest() : method_(HttpMethod::GET) {}
    ~HttpRequest() = default;

    void SetMethod(HttpMethod method) { method_ = method; }
    void SetUri(const Uri &uri) { uri_ = move(uri); }

    HttpMethod method() const { return method_; }
    Uri uri() const { return uri_; }

    friend string to_string(const HttpRequest &request);
    friend HttpRequest string_to_request(const string &request_string);

private:
    HttpMethod method_;
    Uri uri_;
};
class HttpResponse : public HttpMessageInterface {
public:
    HttpResponse() : status_code_(HttpStatusCode::Ok) {}
    HttpResponse(HttpStatusCode status_code) : status_code_(status_code) {}
    ~HttpResponse() = default;
    void SetStatusCode(HttpStatusCode status_code) { status_code_ = status_code; }
    HttpStatusCode status_code() const { return status_code_; }
    friend string to_string(const HttpResponse &request, bool send_content);
    friend HttpResponse string_to_response(const string &response_string);

private:
    HttpStatusCode status_code_;
};

// Utility functions to convert HTTP message objects to string
string to_string(const HttpRequest &request);
string to_string(const HttpResponse &response, bool send_content = true);
HttpRequest string_to_request(const string &request_string);
HttpResponse string_to_response(const string &response_string);

#endif  // HTTP_MESSAGE_H_