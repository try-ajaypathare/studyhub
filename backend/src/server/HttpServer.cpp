#include "server/HttpServer.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

// ---------- Cross-platform socket abstraction ----------
#ifdef _WIN32
  #ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0600
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  using socket_t = SOCKET;
  static const socket_t kInvalidSocket = INVALID_SOCKET;
  static const int      kSocketError   = SOCKET_ERROR;
  static int  ssaas_close(socket_t s) { return closesocket(s); }
  static void ssaas_shutdown(socket_t s) { ::shutdown(s, SD_BOTH); }
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <signal.h>
  #include <cerrno>
  #include <cstring>
  using socket_t = int;
  static const socket_t kInvalidSocket = -1;
  static const int      kSocketError   = -1;
  static int  ssaas_close(socket_t s) { return ::close(s); }
  static void ssaas_shutdown(socket_t s) { ::shutdown(s, SHUT_RDWR); }
  using BOOL = int;
  #ifndef TRUE
  #define TRUE 1
  #endif
#endif

#include <vector>
#include <string>
#include <cstring>

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
#ifdef _WIN32
    WSADATA wsa;
    int err = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (err != 0)
        throw HttpException(500, "WSAStartup failed: " + std::to_string(err));
#else
    // Ignore SIGPIPE — write to a half-closed peer should fail with
    // EPIPE on send(), not kill the process.
    signal(SIGPIPE, SIG_IGN);
#endif
}

void HttpServer::cleanupWinsock() {
    if (listenSocket) {
        socket_t s = static_cast<socket_t>(reinterpret_cast<uintptr_t>(listenSocket));
        ssaas_close(s);
        listenSocket = nullptr;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::run() {
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == kInvalidSocket)
        throw HttpException(500, "socket() failed");
    listenSocket = reinterpret_cast<void*>(static_cast<uintptr_t>(sock));

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&yes), sizeof(yes));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(static_cast<uint16_t>(port));
    if (host == "0.0.0.0" || host.empty()) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
#ifdef _WIN32
        unsigned long ipNum = inet_addr(host.c_str());
        if (ipNum == INADDR_NONE)
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
        else
            addr.sin_addr.s_addr = ipNum;
#else
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1)
            addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
    }

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == kSocketError) {
        throw HttpException(500, "bind() failed on port " + std::to_string(port));
    }
    if (::listen(sock, SOMAXCONN) == kSocketError) {
        throw HttpException(500, "listen() failed");
    }

    Logger::getInstance().info("SSAAS server listening on http://" +
                               (host.empty() ? "0.0.0.0" : host) + ":" +
                               std::to_string(port));

    running = true;
    while (running) {
        sockaddr_in cli;
#ifdef _WIN32
        int cliSize = sizeof(cli);
#else
        socklen_t cliSize = sizeof(cli);
#endif
        socket_t conn = ::accept(sock, reinterpret_cast<sockaddr*>(&cli), &cliSize);
        if (conn == kInvalidSocket) {
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
        ssaas_shutdown(conn);
        ssaas_close(conn);
    }

    ssaas_close(sock);
    listenSocket = nullptr;
    Logger::getInstance().info("Server stopped");
}

}
