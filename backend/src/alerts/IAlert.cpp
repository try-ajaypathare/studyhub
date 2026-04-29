#include "alerts/IAlert.hpp"

#include <algorithm>

namespace ssaas {

IAlert::IAlert(AlertSeverity s, std::string t, Date d)
    : severity(s), title(std::move(t)), date(d) {}

int IAlert::severityBaseScore() const {
    switch (severity) {
        case AlertSeverity::CRITICAL: return 60;
        case AlertSeverity::WARNING:  return 30;
        case AlertSeverity::INFO:     return 10;
    }
    return 10;
}

// Default polymorphic priority: severity alone. Concrete subclasses
// override and add their own urgency/magnitude bonuses.
int IAlert::getPriorityScore() const {
    return std::min(100, severityBaseScore());
}

PriorityTier IAlert::getTier() const {
    int s = getPriorityScore();
    if (s >= 80) return PriorityTier::P0;
    if (s >= 60) return PriorityTier::P1;
    if (s >= 35) return PriorityTier::P2;
    return PriorityTier::P3;
}

nlohmann::json IAlert::toJson() const {
    nlohmann::json j;
    j["severity"]      = severityName(severity);
    j["color"]         = severityColor(severity);
    j["title"]         = title;
    j["category"]      = getCategory();
    j["message"]       = getMessage();
    j["actionHint"]    = getActionHint();
    j["date"]          = date.toIso();
    j["priorityScore"] = getPriorityScore();
    j["tier"]          = tierName(getTier());
    return j;
}

}
