#include "alerts/PerformanceAlert.hpp"
#include <sstream>
#include <iomanip>

namespace ssaas {

PerformanceAlert::PerformanceAlert(AlertSeverity s,
                                   std::string code,
                                   double drop,
                                   double hist,
                                   double latest)
    : IAlert(s, "Performance — " + code),
      subjectCode(std::move(code)),
      dropDelta(drop),
      historicalAvg(hist),
      latestPercent(latest) {}

std::string PerformanceAlert::getMessage() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    if (dropDelta < 0) {
        oss << subjectCode << " latest score is " << latestPercent
            << "% — dropped " << -dropDelta << " pts from your "
            << historicalAvg << "% average.";
    } else {
        oss << subjectCode << " bounced up by " << dropDelta
            << " pts vs your " << historicalAvg << "% average.";
    }
    return oss.str();
}

std::string PerformanceAlert::getActionHint() const {
    if (dropDelta >= 0) return "Keep the streak going.";
    if (-dropDelta > 15.0)
        return "Schedule a doubt-clearing session this week; revisit fundamentals.";
    return "Add 30 min/day of focused practice on weak topics.";
}

nlohmann::json PerformanceAlert::toJson() const {
    auto j = IAlert::toJson();
    j["subjectCode"]   = subjectCode;
    j["delta"]         = dropDelta;
    j["historicalAvg"] = historicalAvg;
    j["latestPercent"] = latestPercent;
    return j;
}

}
