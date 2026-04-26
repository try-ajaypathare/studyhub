#include "server/Router.hpp"
#include "utils/Logger.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace ssaas {

void Router::addRoute(const std::string& method,
                      const std::string& path,
                      Handler handler) {
    routes.push_back({method, path, std::move(handler)});
}

void Router::serveStatic(const std::string& prefix, const std::string& dir) {
    staticMounts.push_back({prefix, dir});
}

bool Router::matches(const std::string& pattern, const std::string& path) {
    if (!pattern.empty() && pattern.back() == '*') {
        return path.compare(0, pattern.size() - 1, pattern, 0, pattern.size() - 1) == 0;
    }
    return pattern == path;
}

HttpResponse Router::dispatch(const HttpRequest& req) const {
    // CORS preflight: short-circuit any OPTIONS request.
    if (req.getMethod() == "OPTIONS") {
        HttpResponse r;
        return r.status(204);
    }

    for (const auto& route : routes) {
        if (route.method != req.getMethod()) continue;
        if (!matches(route.pattern, req.getPath())) continue;
        try {
            return route.handler(req);
        } catch (const std::exception& e) {
            Logger::getInstance().error(
                std::string("Handler threw: ") + e.what() +
                " on " + req.getMethod() + " " + req.getPath());
            return HttpResponse::internalError(e.what());
        }
    }

    // Fall back to static file mounts.
    for (const auto& m : staticMounts) {
        if (req.getMethod() != "GET") continue;
        if (req.getPath().compare(0, m.prefix.size(), m.prefix) == 0)
            return handleStatic(m, req.getPath());
    }

    return HttpResponse::notFound("No route for " + req.getMethod() + " " + req.getPath());
}

namespace {
std::string mimeFor(const std::string& path) {
    auto endsWith = [&](const std::string& suf) {
        return path.size() >= suf.size() &&
            std::equal(suf.rbegin(), suf.rend(), path.rbegin());
    };
    if (endsWith(".html")) return "text/html; charset=utf-8";
    if (endsWith(".css"))  return "text/css; charset=utf-8";
    if (endsWith(".js"))   return "application/javascript; charset=utf-8";
    if (endsWith(".json")) return "application/json; charset=utf-8";
    if (endsWith(".svg"))  return "image/svg+xml";
    if (endsWith(".png"))  return "image/png";
    if (endsWith(".jpg") || endsWith(".jpeg")) return "image/jpeg";
    if (endsWith(".ico"))  return "image/x-icon";
    if (endsWith(".woff2"))return "font/woff2";
    return "application/octet-stream";
}
}

HttpResponse Router::handleStatic(const StaticMount& mount,
                                  const std::string& path) const {
    std::string rel = path.substr(mount.prefix.size());
    if (rel.empty() || rel == "/" ) rel = "/index.html";
    if (rel.front() != '/') rel = "/" + rel;

    // basic traversal guard
    if (rel.find("..") != std::string::npos)
        return HttpResponse::badRequest("Path traversal blocked");

    std::string fullPath = mount.dir + rel;
    std::ifstream in(fullPath, std::ios::binary);
    if (!in) {
        Logger::getInstance().warn("Static 404: " + fullPath);
        return HttpResponse::notFound("Static file missing: " + rel);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    HttpResponse r;
    r.binary(ss.str(), mimeFor(rel));
    return r;
}

}
