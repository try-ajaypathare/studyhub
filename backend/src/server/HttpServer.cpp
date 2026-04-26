#include "server/HttpServer.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

// Force a Vista+ Windows SDK so inet_pton & friends are visible.
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>
#include <string>

namespace ssaas {

namespace {
const size_t  RECV_BUF_SIZE     = 8192;
const size_t  MAX_REQUEST_BYTES = 1 * 1024 * 1024;   // 1 MB hard cap
}

HttpServer::HttpServer(std::string h, int p)
    : host(std::move(h)),
      port(p),
      running(false),
      listenSocket(nullptr) {
    initWinsock();
}

HttpServer::~HttpServer() {
    cleanupWinsock();
}

void HttpServer::initWinsock() {
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err != 0)
        throw HttpException(500, "WSAStartup failed: " + std::to_string(err));
}

void HttpServer::cleanupWinsock() {
    if (listenSocket) {
        closesocket(reinterpret_cast<SOCKET>(listenSocket));
        listenSocket = nullptr;
    }
    WSACleanup();
}

void HttpServer::run() {
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        throw HttpException(500, "socket() failed");
    listenSocket = reinterpret_cast<void*>(sock);

    BOOL yes = TRUE;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&yes), sizeof(yes));

    sockaddr_in addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<u_short>(port));
    if (host == "0.0.0.0" || host.empty()) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        // inet_addr is deprecated but available in any MinGW. Fine for an OOP demo.
        unsigned long ipNum = inet_addr(host.c_str());
        if (ipNum == INADDR_NONE) {
            // Fallback: bind to all interfaces if hostname can't be resolved.
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            addr.sin_addr.s_addr = ipNum;
        }
    }

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        int e = WSAGetLastError();
        throw HttpException(500, "bind() failed: " + std::to_string(e));
    }
    if (::listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        throw HttpException(500, "listen() failed");
    }

    Logger::getInstance().info("SSAAS server listening on http://" +
                               (host.empty() ? "0.0.0.0" : host) + ":" +
                               std::to_string(port));

    running = true;
    while (running) {
        sockaddr_in cli;
        int cliSize = sizeof(cli);
        SOCKET conn = ::accept(sock, reinterpret_cast<sockaddr*>(&cli), &cliSize);
        if (conn == INVALID_SOCKET) {
            Logger::getInstance().warn("accept() returned invalid socket");
            continue;
        }

        // Read request — blocking until we see "\r\n\r\n" or hit a bound.
        std::string buffer;
        buffer.reserve(RECV_BUF_SIZE);
        char tmp[RECV_BUF_SIZE];
        size_t headerEnd = std::string::npos;

        for (;;) {
            int n = ::recv(conn, tmp, sizeof(tmp), 0);
            if (n <= 0) break;
            buffer.append(tmp, tmp + n);
            if (buffer.size() > MAX_REQUEST_BYTES) break;
            headerEnd = buffer.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                // If POST, may need full body (Content-Length). Inspect header.
                std::string head = buffer.substr(0, headerEnd);
                size_t clPos = head.find("Content-Length:");
                if (clPos == std::string::npos)
                    clPos = head.find("content-length:");
                size_t needed = headerEnd + 4;
                if (clPos != std::string::npos) {
                    size_t lineEnd = head.find("\r\n", clPos);
                    std::string lenStr = head.substr(clPos + 15,
                        (lineEnd == std::string::npos ? head.size() : lineEnd) - clPos - 15);
                    try {
                        size_t cl = static_cast<size_t>(std::stoul(lenStr));
                        needed += cl;
                    } catch (...) { /* ignore */ }
                }
                if (buffer.size() >= needed) break;
            }
        }

        HttpResponse resp;
        try {
            HttpRequest req = HttpRequest::parse(buffer);
            Logger::getInstance().info(
                req.getMethod() + " " + req.getPath() +
                (req.getQuery().empty() ? "" : "?" + req.getQuery()));
            resp = routes.dispatch(req);
        } catch (const HttpException& he) {
            resp = HttpResponse::badRequest(he.what());
        } catch (const std::exception& e) {
            resp = HttpResponse::internalError(e.what());
        }

        std::string out = resp.serialize();
        ::send(conn, out.c_str(), static_cast<int>(out.size()), 0);
        ::shutdown(conn, SD_BOTH);
        ::closesocket(conn);
    }

    ::closesocket(sock);
    listenSocket = nullptr;
    Logger::getInstance().info("Server stopped");
}

}
