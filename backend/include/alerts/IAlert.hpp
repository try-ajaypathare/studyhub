#ifndef SSAAS_ALERTS_IALERT_HPP
#define SSAAS_ALERTS_IALERT_HPP

#include "third_party/json.hpp"
#include "core/Date.hpp"

#include <string>
#include <memory>

namespace ssaas {

enum class AlertSeverity { INFO, WARNING, CRITICAL };

inline const char* severityName(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::INFO:     return "Info";
        case AlertSeverity::WARNING:  return "Warning";
        case AlertSeverity::CRITICAL: return "Critical";
    }
    return "?";
}

inline const char* severityColor(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::INFO:     return "#3b82f6";
        case AlertSeverity::WARNING:  return "#f59e0b";
        case AlertSeverity::CRITICAL: return "#ef4444";
    }
    return "#6b7280";
}

// Polymorphic alert base — every alert overrides getMessage() and toJson().
class IAlert {
public:
    IAlert(AlertSeverity sev, std::string title, Date when = Date::today());
    virtual ~IAlert() = default;

    AlertSeverity      getSeverity() const { return severity; }
    const std::string& getTitle()    const { return title; }
    const Date&        getDate()     const { return date; }

    virtual std::string getCategory() const = 0;
    virtual std::string getMessage()  const = 0;
    virtual std::string getActionHint() const { return ""; }

    virtual nlohmann::json toJson() const;

protected:
    AlertSeverity severity;
    std::string   title;
    Date          date;
};

using AlertPtr = std::unique_ptr<IAlert>;

}

#endif
