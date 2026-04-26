#include "server/HttpResponse.hpp"

#include <sstream>

namespace ssaas {

HttpResponse::HttpResponse() : statusCode(200), statusText("OK") {
    headers["Server"] = "SSAAS-CppServer/1.0";
    headers["Connection"] = "close";
    // CORS-friendly defaults so frontend can hit the API from file://.
    headers["Access-Control-Allow-Origin"]  = "*";
    headers["Access-Control-Allow-Headers"] = "Content-Type";
    headers["Access-Control-Allow-Methods"] = "GET,POST,PUT,DELETE,OPTIONS";
}

HttpResponse& HttpResponse::status(int code) {
    statusCode = code;
    statusText = defaultStatusText(code);
    return *this;
}

HttpResponse& HttpResponse::header(const std::string& k, const std::string& v) {
    headers[k] = v;
    return *this;
}

HttpResponse& HttpResponse::contentType(const std::string& m) {
    headers["Content-Type"] = m;
    return *this;
}

HttpResponse& HttpResponse::body(const std::string& b) {
    responseBody = b;
    return *this;
}

HttpResponse& HttpResponse::json(const nlohmann::json& j) {
    responseBody = j.dump();
    headers["Content-Type"] = "application/json; charset=utf-8";
    return *this;
}

HttpResponse& HttpResponse::text(const std::string& msg) {
    responseBody = msg;
    headers["Content-Type"] = "text/plain; charset=utf-8";
    return *this;
}

HttpResponse& HttpResponse::html(const std::string& html) {
    responseBody = html;
    headers["Content-Type"] = "text/html; charset=utf-8";
    return *this;
}

HttpResponse& HttpResponse::binary(const std::string& bytes,
                                   const std::string& mime) {
    responseBody = bytes;
    headers["Content-Type"] = mime;
    return *this;
}

HttpResponse HttpResponse::ok(const nlohmann::json& j) {
    HttpResponse r; r.status(200).json(j); return r;
}

HttpResponse HttpResponse::notFound(const std::string& msg) {
    HttpResponse r;
    nlohmann::json e = { {"error", msg}, {"status", 404} };
    r.status(404).json(e); return r;
}

HttpResponse HttpResponse::badRequest(const std::string& msg) {
    HttpResponse r;
    nlohmann::json e = { {"error", msg}, {"status", 400} };
    r.status(400).json(e); return r;
}

HttpResponse HttpResponse::internalError(const std::string& msg) {
    HttpResponse r;
    nlohmann::json e = { {"error", msg}, {"status", 500} };
    r.status(500).json(e); return r;
}

std::string HttpResponse::serialize() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    oss << "Content-Length: " << responseBody.size() << "\r\n";
    for (const auto& kv : headers)
        oss << kv.first << ": " << kv.second << "\r\n";
    oss << "\r\n" << responseBody;
    return oss.str();
}

std::string HttpResponse::defaultStatusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        default:  return "Unknown";
    }
}

}
