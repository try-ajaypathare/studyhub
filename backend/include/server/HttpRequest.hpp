#ifndef SSAAS_SERVER_HTTP_REQUEST_HPP
#define SSAAS_SERVER_HTTP_REQUEST_HPP

#include <string>
#include <unordered_map>

namespace ssaas {

// Parsed HTTP/1.1 request — method, path, query string, headers, body.
class HttpRequest {
public:
    HttpRequest() = default;

    static HttpRequest parse(const std::string& raw);

    const std::string& getMethod()  const { return method; }
    const std::string& getPath()    const { return path; }
    const std::string& getQuery()   const { return query; }
    const std::string& getVersion() const { return version; }
    const std::string& getBody()    const { return body; }

    bool hasHeader(const std::string& name) const;
    std::string getHeader(const std::string& name, const std::string& def = "") const;

    // Convenience: query parameters
    bool hasQueryParam(const std::string& name) const;
    std::string getQueryParam(const std::string& name, const std::string& def = "") const;

    // For controller use
    const std::unordered_map<std::string, std::string>& getQueryParams() const { return queryParams; }

private:
    static std::string toLower(std::string s);
    static std::unordered_map<std::string, std::string> parseQuery(const std::string& q);
    static std::string urlDecode(const std::string& s);

    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::string body;
    std::unordered_map<std::string, std::string> headers;       // lowercase keys
    std::unordered_map<std::string, std::string> queryParams;
};

}

#endif
