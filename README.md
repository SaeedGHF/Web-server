
# Web-server

## Project structure

### http_message.h + http_message.cpp
#### Enums:
+ **HttpMethod**
 GET, POST, ...
+ **HttpVersion**
 Here we only support HTTP/1.1
+ **HttpStatusCode**
 HTTP response status codes as listed in:
 https://developer.mozilla.org/en-US/docs/Web/HTTP/Status  
 Note that not all of them are present in this enum class
#### Classes:
+ **HttpMessageInterface**
 Defines the common interface of an HTTP request and HTTP response.  
 Each message will have an HTTP version, collection of header fields, and message content.
 The collection of headers and content can be empty.
+ **HttpRequest**
 An HttpRequest object represents a single HTTP request  
 It has a HTTP method and URI so that the server can identify the corresponding resource and action
+ **HttpResponse**
 An HTTPResponse object represents a single HTTP response  
 The HTTP server sends a HTTP response to a client that include a HTTP status code, headers, and (optional) content

### uri.h

Define object that can be used to represent a Unique Resource Identifier which is useful for parsing and request handling
A Uri object will contain a valid scheme (for example: HTTP), host, port, and the actual URI path

#### Classes:
+ Uri

<hr>

### http_server.h + http_server.cpp

Defines the HTTP server object with some constants and structs useful for request handling and improving performance

A request handler should expect a request as argument and returns a response.

### Data structures:
- EventData: every action in our system is defined as an event that has these values:
> {fd, length, cursor, buffer}

### Classes:
- HttpServer

The server consists of:
- main thread
- listener thread that is responsible for accepting new connections
- Possibly many threads that process HTTP messages and communicate with clients via socket.
- The number of workers is defined by a constant