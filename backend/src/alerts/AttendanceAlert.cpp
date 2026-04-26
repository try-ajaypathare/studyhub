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

nlohmann::json AttendanceAlert::toJson() const {
    auto j = IAlert::toJson();
    j["subjectCode"]   = subjectCode;
    j["currentPercent"]= currentPercent;
    j["targetPercent"] = targetPercent;
    return j;
}

}
