#include "alerts/BurnoutAlert.hpp"
#include <sstream>
#include <iomanip>

namespace ssaas {

BurnoutAlert::BurnoutAlert(AlertSeverity s, double sc, std::string lv)
    : IAlert(s, "Wellbeing"), score(sc), level(std::move(lv)) {}

std::string BurnoutAlert::getMessage() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(0);
    oss << "Burnout score " << score << "/100 (" << level << ").";
    return oss.str();
}

std::string BurnoutAlert::getActionHint() const {
    if (score >= 60.0)
        return "Cut one commitment this week. Sleep, walks, real meals.";
    if (score >= 30.0)
        return "Take 10-min breaks every 50 min and protect 7+h of sleep.";
    return "Healthy pace — keep it up.";
}

// Severity base + (score above 30) / 4, clamped to 15.
// e.g. warning at 49 → 30 + 4 = 34 (P3 boundary). Critical at 75 → 60 + 11 = 71 (P1).
int BurnoutAlert::getPriorityScore() const {
    double bonus = score > 30 ? (score - 30) / 4.0 : 0.0;
    if (bonus > 15) bonus = 15;
    int s = severityBaseScore() + static_cast<int>(bonus);
    return s > 100 ? 100 : s;
}

nlohmann::json BurnoutAlert::toJson() const {
    auto j = IAlert::toJson();
    j["score"] = score;
    j["level"] = level;
    return j;
}

}
