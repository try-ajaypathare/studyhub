#ifndef SSAAS_UTILS_EXCEPTIONS_HPP
#define SSAAS_UTILS_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace ssaas {

class SSAASException : public std::runtime_error {
public:
    explicit SSAASException(const std::string& msg)
        : std::runtime_error(msg) {}
};

class NotFoundException : public SSAASException {
public:
    explicit NotFoundException(const std::string& msg)
        : SSAASException("[NotFound] " + msg) {}
};

class ValidationException : public SSAASException {
public:
    explicit ValidationException(const std::string& msg)
        : SSAASException("[Validation] " + msg) {}
};

class StorageException : public SSAASException {
public:
    explicit StorageException(const std::string& msg)
        : SSAASException("[Storage] " + msg) {}
};

class AnalyticsException : public SSAASException {
public:
    explicit AnalyticsException(const std::string& msg)
        : SSAASException("[Analytics] " + msg) {}
};

class HttpException : public SSAASException {
public:
    HttpException(int code, const std::string& msg)
        : SSAASException("[Http " + std::to_string(code) + "] " + msg),
          statusCode(code) {}
    int getStatusCode() const { return statusCode; }
private:
    int statusCode;
};

}

#endif
