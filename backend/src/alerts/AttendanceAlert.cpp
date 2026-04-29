#include "alerts/AttendanceAlert.hpp"

#include <sstream>
#include <iomanip>

namespace ssaas {

AttendanceAlert::AttendanceAlert(AlertSeverity s,
                                 std::string code,
                                 double cur,
                                 double tgt)
    : IAlert(s, "Attendance — " + code),
      subjectCode(std::move(code)),
      currentPercent(cur),
      targetPercent(tgt) {}

std::string AttendanceAlert::getMessage() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << subjectCode << " attendance is " << currentPercent
        << "% (target " << targetPercent << "%).";
    return oss.str();
}

std::string AttendanceAlert::getActionHint() const {
    if (currentPercent >= targetPercent)
        return "Stay consistent — you're above the threshold.";
    if (currentPercent >= 65.0)
        return "Attend the next 4-6 classes without break to recover.";
    return "Talk to faculty — recovery via attendance alone may be impossible.";
}

// Severity base + (target − current) clamped to 20.
// e.g. critical at 50% → 60 + 20 = 80 (P0). Warning at 70% → 30 + 5 = 35 (P2).
int AttendanceAlert::getPriorityScore() const {
    double gap = targetPercent - currentPercent;
    if (gap < 0) gap = 0;
    if (gap > 20) gap = 20;
    int s = severityBaseScore() + static_cast<int>(gap);
    return s > 100 ? 100 : s;
}

nlohmann::json AttendanceAlert::toJson() const {
    auto j = IAlert::toJson();
    j["subjectCode"]   = subjectCode;
    j["currentPercent"]= currentPercent;
    j["targetPercent"] = targetPercent;
    return j;
}

}
