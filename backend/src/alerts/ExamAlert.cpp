#include "alerts/ExamAlert.hpp"
#include <sstream>

namespace ssaas {

ExamAlert::ExamAlert(AlertSeverity s,
                     std::string code,
                     std::string type,
                     int days,
                     int prep)
    : IAlert(s, "Exam — " + code),
      subjectCode(std::move(code)),
      examType(std::move(type)),
      daysUntil(days),
      preparationProgress(prep) {}

std::string ExamAlert::getMessage() const {
    std::ostringstream oss;
    oss << examType << " for " << subjectCode << " in " << daysUntil
        << " day(s). Preparation: " << preparationProgress << "%.";
    return oss.str();
}

std::string ExamAlert::getActionHint() const {
    if (preparationProgress >= 80) return "Final revision + mock test.";
    if (preparationProgress >= 50) return "Time-box weak topics across remaining days.";
    if (daysUntil <= 3) return "Triage: focus on highest-weight topics only.";
    return "Block 3 hours of deep work today on this subject.";
}

// Severity base + days-until urgency + low-prep bonus.
//   ≤1 day: +30 · ≤3 days: +20 · ≤7 days: +10
//   prep <30%: +10 (cumulative)
// e.g. critical + 2 days + 25% prep → 60 + 20 + 10 = 90 (P0)
int ExamAlert::getPriorityScore() const {
    int s = severityBaseScore();
    if (daysUntil <= 1)      s += 30;
    else if (daysUntil <= 3) s += 20;
    else if (daysUntil <= 7) s += 10;
    if (preparationProgress < 30) s += 10;
    return s > 100 ? 100 : s;
}

nlohmann::json ExamAlert::toJson() const {
    auto j = IAlert::toJson();
    j["subjectCode"] = subjectCode;
    j["examType"]    = examType;
    j["daysUntil"]   = daysUntil;
    j["preparation"] = preparationProgress;
    return j;
}

}
