#include "server/HttpRequest.hpp"
#include "utils/Exceptions.hpp"

#include <sstream>
#include <algorithm>
#include <cctype>

namespace ssaas {

std::string HttpRequest::toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string HttpRequest::urlDecode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out.push_back(' ');
        } else if (s[i] == '%' && i + 2 < s.size()) {
            std::string hex = s.substr(i + 1, 2);
            char c = static_cast<char>(std::stoi(hex, nullptr, 16));
            out.push_back(c);
            i += 2;
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::unordered_map<std::string, std::string>
HttpRequest::parseQuery(const std::string& q) {
    std::unordered_map<std::string, std::string> out;
    std::string token;
    std::stringstream ss(q);
    while (std::getline(ss, token, '&')) {
        if (token.empty()) continue;
        auto eq = token.find('=');
        if (eq == std::string::npos) {
            out[urlDecode(token)] = "";
        } else {
            out[urlDecode(token.substr(0, eq))] = urlDecode(token.substr(eq + 1));
        }
    }
    return out;
}

bool HttpRequest::hasHeader(const std::string& name) const {
    return headers.count(toLower(name)) > 0;
}

std::string HttpRequest::getHeader(const std::string& name,
                                   const std::string& def) const {
    auto it = headers.find(toLower(name));
    return it == headers.end() ? def : it->second;
}

bool HttpRequest::hasQueryParam(const std::string& name) const {
    return queryParams.count(name) > 0;
}

std::string HttpRequest::getQueryParam(const std::string& name,
                                       const std::string& def) const {
    auto it = queryParams.find(name);
    return it == queryParams.end() ? def : it->second;
}

HttpRequest HttpRequest::parse(const std::string& raw) {
    HttpRequest r;
    auto headerEnd = raw.find("\r\n\r\n");
    std::string head = headerEnd == std::string::npos ? raw : raw.substr(0, headerEnd);
    if (headerEnd != std::string::npos)
        r.body = raw.substr(headerEnd + 4);

    std::istringstream ss(head);
    std::string line;
    if (!std::getline(ss, line))
        throw HttpException(400, "Empty request line");
    if (!line.empty() && line.back() == '\r') line.pop_back();

    // Request line: METHOD PATH HTTP/x.y
    std::istringstream rl(line);
    std::string fullPath;
    if (!(rl >> r.method >> fullPath >> r.version))
        throw HttpException(400, "Malformed request line: " + line);

    auto qpos = fullPath.find('?');
    if (qpos == std::string::npos) {
        r.path = fullPath;
    } else {
        r.path  = fullPath.substr(0, qpos);
        r.query = fullPath.substr(qpos + 1);
        r.queryParams = parseQuery(r.query);
    }

    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        auto colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = toLower(line.substr(0, colon));
        std::string val = line.substr(colon + 1);
        size_t start = val.find_first_not_of(" \t");
        if (start != std::string::npos) val = val.substr(start);
        r.headers[key] = val;
    }
    return r;
}

}
