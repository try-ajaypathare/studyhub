#ifndef SSAAS_SERVER_HTTP_SERVER_HPP
#define SSAAS_SERVER_HTTP_SERVER_HPP

#include "server/Router.hpp"

#include <string>

namespace ssaas {

// Single-threaded blocking HTTP/1.1 server backed by Winsock2.
// Accepts connections, parses one request, dispatches via the Router,
// writes the response, closes the socket. Sufficient for an OOP demo.
class HttpServer {
public:
    HttpServer(std::string host, int port);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    Router& router() { return routes; }

    // Blocks until the user kills the process (Ctrl+C).
    void run();

    void requestStop() { running = false; }

private:
    void initWinsock();
    void cleanupWinsock();

    std::string host;
    int         port;
    bool        running;
    Router      routes;
    void*       listenSocket;   // SOCKET stored as void* to avoid leaking winsock types
};

}

#endif
