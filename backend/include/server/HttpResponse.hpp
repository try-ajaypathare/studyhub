#ifndef SSAAS_SERVER_HTTP_RESPONSE_HPP
#define SSAAS_SERVER_HTTP_RESPONSE_HPP

#include "third_party/json.hpp"

#include <string>
#include <unordered_map>

namespace ssaas {

class HttpResponse {
public:
    HttpResponse();

    HttpResponse& status(int code);
    HttpResponse& header(const std::string& key, const std::string& value);
    HttpResponse& contentType(const std::string& mime);
    HttpResponse& body(const std::string& b);
    HttpResponse& json(const nlohmann::json& j);
    HttpResponse& text(const std::string& msg);
    HttpResponse& html(const std::string& html);
    HttpResponse& binary(const std::string& bytes, const std::string& mime);

    // Pre-built helpers
    static HttpResponse ok(const nlohmann::json& j);
    static HttpResponse notFound(const std::string& msg = "Not Found");
    static HttpResponse badRequest(const std::string& msg);
    static HttpResponse internalError(const std::string& msg);

    std::string serialize() const;

    int getStatus() const { return statusCode; }

private:
    int statusCode;
    std::string statusText;
    std::unordered_map<std::string, std::string> headers;
    std::string responseBody;

    static std::string defaultStatusText(int code);
};

}

#endif
