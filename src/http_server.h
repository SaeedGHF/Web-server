#ifndef HTTP_SERVER_H_
#define HTTP_SERVER_H_

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <utility>

#include "http_message.h"
#include "uri.h"

using namespace std;

// Maximum size of an HTTP message is limited by how much bytes
// we can read or send via socket each time
constexpr size_t kMaxBufferSize = 1024;

struct EventData {
    EventData() : fd(0), length(0), cursor(0), buffer() {}
    int fd;
    size_t length;
    size_t cursor;
    char buffer[kMaxBufferSize];
};

using HttpRequestHandler_t = function<
HttpResponse(const HttpRequest &)
>;

class HttpServer {
public:
    explicit HttpServer(const string &host, uint16_t port);
    ~HttpServer() = default;

    HttpServer() = default;
    HttpServer(HttpServer &&) = default;
    HttpServer &operator=(HttpServer &&) = default;

    void Start();
    void Stop();
    void RegisterHttpRequestHandler(const string &path, HttpMethod method,
                                    const HttpRequestHandler_t callback) {
        Uri uri(path);
        request_handlers_[uri].insert(make_pair(method, move(callback)));
    }
    void RegisterHttpRequestHandler(const Uri &uri, HttpMethod method,
                                    const HttpRequestHandler_t callback) {
        request_handlers_[uri].insert(make_pair(method, move(callback)));
    }

    string host() const { return host_; }
    uint16_t port() const { return port_; }
    bool running() const { return running_; }

private:
    static constexpr int kBacklogSize = 1000;
    static constexpr int kMaxConnections = 10000;
    static constexpr int kMaxEvents = 10000;
    static constexpr int kThreadPoolSize = 5;

    string host_;
    uint16_t port_;
    int sock_fd_;
    bool running_;
    thread listener_thread_;
    thread worker_threads_[kThreadPoolSize];
    int worker_epoll_fd_[kThreadPoolSize];
    epoll_event worker_events_[kThreadPoolSize][kMaxEvents];
    map <Uri, map<HttpMethod, HttpRequestHandler_t>> request_handlers_;

    void CreateSocket();
    void SetUpEpoll();
    void Listen();
    void ProcessEvents(int worker_id);
    void HandleEpollEvent(int epoll_fd, EventData *event, uint32_t events);
    void HandleHttpData(const EventData &request, EventData *response);
    HttpResponse HandleHttpRequest(const HttpRequest &request);

    void control_epoll_event(int epoll_fd, int op, int fd,
                             uint32_t events = 0, void *data = nullptr);
};

#endif  // HTTP_SERVER_H_