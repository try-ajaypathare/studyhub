#ifndef SSAAS_UTILS_LOGGER_HPP
#define SSAAS_UTILS_LOGGER_HPP

#include <string>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace ssaas {

enum class LogLevel { DEBUG, INFO, WARN, ERR };

// Singleton logger. Single-threaded — server accepts one request at a time.
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void setMinLevel(LogLevel level) { minLevel = level; }

    void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    void info(const std::string& msg)  { log(LogLevel::INFO,  msg); }
    void warn(const std::string& msg)  { log(LogLevel::WARN,  msg); }
    void error(const std::string& msg) { log(LogLevel::ERR,   msg); }

private:
    Logger() : minLevel(LogLevel::INFO) {}

    void log(LogLevel level, const std::string& msg) {
        if (static_cast<int>(level) < static_cast<int>(minLevel)) return;
        std::cout << "[" << timestamp() << "] "
                  << "[" << levelName(level) << "] "
                  << msg << std::endl;
    }

    std::string timestamp() const {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        std::ostringstream oss;
        if (tm) oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string levelName(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::ERR:   return "ERROR";
        }
        return "?";
    }

    LogLevel minLevel;
};

}

#endif
