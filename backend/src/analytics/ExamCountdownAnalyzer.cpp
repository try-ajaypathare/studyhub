#include "analytics/ExamCountdownAnalyzer.hpp"
#include "core/Student.hpp"

namespace ssaas {

nlohmann::json ExamCountdownAnalyzer::analyze(const Student& student) const {
    nlohmann::json out;
    out["upcoming"] = nlohmann::json::array();

    auto upcoming = student.getUpcomingExams();
    int nearest = -1;
    nlohmann::json nearestExam;

    for (const auto& ex : upcoming) {
        int days = ex.daysUntil();

        std::string urgency;
        if (days <= 3)       urgency = "imminent";
        else if (days <= 7)  urgency = "this week";
        else if (days <= 14) urgency = "next two weeks";
        else                  urgency = "later";

        std::string subjName = student.hasSubject(ex.getSubjectCode())
            ? student.getSubject(ex.getSubjectCode()).getName()
            : ex.getSubjectCode();

        nlohmann::json e;
        e["subjectCode"]    = ex.getSubjectCode();
        e["subjectName"]    = subjName;
        e["type"]           = assessmentTypeName(ex.getType());
        e["date"]           = ex.getDate().toIso();
        e["dateReadable"]   = ex.getDate().toReadable();
        e["time"]           = ex.getTime();
        e["venue"]          = ex.getVenue();
        e["maxMarks"]       = ex.getMaxMarks();
        e["daysUntil"]      = days;
        e["urgency"]        = urgency;
        e["preparation"]    = ex.getPreparationProgress();
        out["upcoming"].push_back(e);

        if (nearest == -1 || days < nearest) {
            nearest = days;
            nearestExam = e;
        }
    }
    out["nearest"]  = nearestExam;
    out["count"]    = static_cast<int>(upcoming.size());
    return out;
}

}
