#include "alerts/IAlert.hpp"

namespace ssaas {

IAlert::IAlert(AlertSeverity s, std::string t, Date d)
    : severity(s), title(std::move(t)), date(d) {}

nlohmann::json IAlert::toJson() const {
    nlohmann::json j;
    j["severity"]   = severityName(severity);
    j["color"]      = severityColor(severity);
    j["title"]      = title;
    j["category"]   = getCategory();
    j["message"]    = getMessage();
    j["actionHint"] = getActionHint();
    j["date"]       = date.toIso();
    return j;
}

}
