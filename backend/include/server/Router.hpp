#ifndef SSAAS_SERVER_ROUTER_HPP
#define SSAAS_SERVER_ROUTER_HPP

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <functional>
#include <vector>
#include <string>

namespace ssaas {

// Lightweight path-prefix matcher. Supports exact paths and a single optional
// "*" suffix (for serving static directories).
class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    void addRoute(const std::string& method,
                  const std::string& path,
                  Handler handler);

    // Convenience overloads
    void get(const std::string& p, Handler h)    { addRoute("GET",    p, std::move(h)); }
    void post(const std::string& p, Handler h)   { addRoute("POST",   p, std::move(h)); }
    void put(const std::string& p, Handler h)    { addRoute("PUT",    p, std::move(h)); }
    void del(const std::string& p, Handler h)    { addRoute("DELETE", p, std::move(h)); }

    // Static file serving (mountPrefix maps requests to a directory).
    void serveStatic(const std::string& mountPrefix, const std::string& dir);

    HttpResponse dispatch(const HttpRequest& req) const;

private:
    struct Route {
        std::string method;
        std::string pattern;       // exact or ends with '*'
        Handler     handler;
    };

    static bool matches(const std::string& pattern, const std::string& path);

    std::vector<Route> routes;

    struct StaticMount {
        std::string prefix;
        std::string dir;
    };
    std::vector<StaticMount> staticMounts;

    HttpResponse handleStatic(const StaticMount& mount,
                              const std::string& path) const;
};

}

#endif
