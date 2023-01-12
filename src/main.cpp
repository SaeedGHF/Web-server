#include <sys/resource.h>
#include <sys/time.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "http_message.cpp"
#include "http_server.cpp"
#include "uri.h"

using namespace std;

void ensure_enough_resource(int resource, uint32_t soft_limit,
                            uint32_t hard_limit) {
    rlimit new_limit, old_limit;

    new_limit.rlim_cur = soft_limit;
    new_limit.rlim_max = hard_limit;
    getrlimit(resource, &old_limit);

    cout << "Old limit: " << old_limit.rlim_cur << " (soft limit), "
              << old_limit.rlim_cur << " (hard limit)." << endl;
    cout << "New limit: " << new_limit.rlim_cur << " (soft limit), "
              << new_limit.rlim_cur << " (hard limit)." << endl;

    if (setrlimit(resource, &new_limit)) {
        cerr << "Warning: Could not update resource limit ";
        cerr << "(" << strerror(errno) << ")." << endl;
        cerr << "Consider setting the limit manually with ulimit" << endl;
        exit(-1);
    }
}

int main(int argc, char **argv) {
    string host = "0.0.0.0";
    int port = stoi(argv[1]);
    HttpServer server(host, port);
    // Register a few endpoints for demo and benchmarking
    auto redirectToIndex = [](const HttpRequest &request) -> HttpResponse {
        HttpResponse response(HttpStatusCode::MovedPermanently);
        response.SetHeader("Location", "index.html");
        return response;
    };
    auto index = [](const HttpRequest &request) -> HttpResponse {
        HttpResponse response(HttpStatusCode::Ok);
        string content;
        content += "<!doctype html>\n";
        content += "<html>\n<body>\n\n";
        content += "<h1>Hello, world in an Html page</h1>\n";
        content += "<p>A Paragraph</p>\n\n";
        content += "</body>\n</html>\n";
        response.SetHeader("Content-Type", "text/html");
        response.SetContent(content);
        return response;
    };
    server.RegisterHttpRequestHandler("/", HttpMethod::HEAD, redirectToIndex);
    server.RegisterHttpRequestHandler("/", HttpMethod::GET, redirectToIndex);
    server.RegisterHttpRequestHandler("/index.html", HttpMethod::HEAD, index);
    server.RegisterHttpRequestHandler("/index.html", HttpMethod::GET, index);
    try {
        // cout << "Setting new limits for file descriptor count.." <<
        // endl; ensure_enough_resource(RLIMIT_NOFILE, 15000, 15000);
        // cout << "Setting new limits for number of threads.." << endl;
        // ensure_enough_resource(RLIMIT_NPROC, 60000, 60000);
        cout << "Starting the web server.." << endl;
        server.Start();
        cout << "Server listening on " << host << ":" << port << endl;
        cout << "Enter [quit] to stop the server" << endl;
        string command;
        while (cin >> command, command != "quit");
        cout << "'quit' command entered. Stopping the web server.."
                  << endl;
        server.Stop();
        cout << "Server stopped" << endl;
    } catch (exception &e) {
        cerr << "An error occurred: " << e.what() << endl;
        return -1;
    }
    return 0;
}