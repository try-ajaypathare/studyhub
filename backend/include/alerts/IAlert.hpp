#ifndef SSAAS_ALERTS_IALERT_HPP
#define SSAAS_ALERTS_IALERT_HPP

#include "third_party/json.hpp"
#include "core/Date.hpp"

#include <string>
#include <memory>

namespace ssaas {

enum class AlertSeverity { INFO, WARNING, CRITICAL };

// Priority tier — derived from a 0-100 score.
//   P0  >= 80  → critical · take action now
//   P1  >= 60  → high     · this week
//   P2  >= 35  → medium   · on your radar
//   P3  <  35  → info     · heads up
enum class PriorityTier { P0, P1, P2, P3 };

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

inline const char* tierName(PriorityTier t) {
    switch (t) {
        case PriorityTier::P0: return "P0";
        case PriorityTier::P1: return "P1";
        case PriorityTier::P2: return "P2";
        case PriorityTier::P3: return "P3";
    }
    return "P3";
}

// Polymorphic alert base. Every concrete alert overrides:
//   • getCategory() / getMessage() / getActionHint() — content
//   • getPriorityScore()                              — 0-100 score
//
// The base supplies a default getTier() that maps score → tier band, and
// toJson() that bundles everything for the wire format. Subclasses can
// extend toJson() to attach their own structured fields.
class IAlert {
public:
    IAlert(AlertSeverity sev, std::string title, Date when = Date::today());
    virtual ~IAlert() = default;

    AlertSeverity      getSeverity() const { return severity; }
    const std::string& getTitle()    const { return title; }
    const Date&        getDate()     const { return date; }

    virtual std::string getCategory()   const = 0;
    virtual std::string getMessage()    const = 0;
    virtual std::string getActionHint() const { return ""; }

    // Polymorphic priority — each subclass folds its own state into a 0-100
    // score. Default base contribution is severity-only; overrides typically
    // add time-urgency and magnitude bonuses.
    virtual int          getPriorityScore() const;
    virtual PriorityTier getTier()          const;

    virtual nlohmann::json toJson() const;

protected:
    // Base score from severity alone (10 / 30 / 60). Subclasses use this as
    // their starting point and add domain-specific signals on top.
    int severityBaseScore() const;

    AlertSeverity severity;
    std::string   title;
    Date          date;
};

using AlertPtr = std::unique_ptr<IAlert>;

}

#endif
