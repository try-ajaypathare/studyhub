#include "analytics/StudyPlanAnalyzer.hpp"
#include "core/Student.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

namespace ssaas {

namespace {
struct ScoredSubject {
    std::string code;
    std::string name;
    double      priorityScore;   // higher => study first
    double      avgPercent;
    int         daysToExam;      // 999 if no upcoming exam
    int         attendancePct;
};

double computePriority(double avgPercent, int daysToExam, int attPct) {
    double weakness = std::max(0.0, 75.0 - avgPercent);          // 0..75
    double urgency  = daysToExam < 999 ? std::max(0.0, 30.0 - daysToExam) * 2.0 : 0.0; // 0..60
    double attDrag  = std::max(0.0, 75.0 - attPct);              // 0..75
    return weakness + urgency + 0.4 * attDrag;
}
}

nlohmann::json StudyPlanAnalyzer::analyze(const Student& student) const {
    std::vector<ScoredSubject> ranked;

    for (const auto& s : student.getSubjects()) {
        auto m = student.getMarksFor(s.getCode());
        double avg = 0.0;
        if (!m.empty()) {
            for (const auto& r : m) avg += r.getPercentage();
            avg /= m.size();
        }

        int dayMin = 999;
        for (const auto& ex : student.getExamsFor(s.getCode())) {
            if (ex.isUpcoming()) dayMin = std::min(dayMin, ex.daysUntil());
        }

        ranked.push_back({
            s.getCode(), s.getName(),
            computePriority(avg, dayMin, static_cast<int>(s.getAttendancePercentage())),
            avg, dayMin, static_cast<int>(s.getAttendancePercentage())
        });
    }

    std::sort(ranked.begin(), ranked.end(),
        [](const ScoredSubject& a, const ScoredSubject& b) {
            return a.priorityScore > b.priorityScore;
        });

    auto buildSession = [&](const std::string& slot,
                            const std::string& time,
                            size_t startIdx, size_t cnt,
                            const std::string& goal) {
        nlohmann::json sess;
        sess["slot"]      = slot;
        sess["time"]      = time;
        sess["goal"]      = goal;
        sess["subjects"]  = nlohmann::json::array();
        for (size_t i = 0; i < cnt && (startIdx + i) < ranked.size(); ++i) {
            const auto& r = ranked[startIdx + i];
            sess["subjects"].push_back({
                {"code", r.code},
                {"name", r.name},
                {"priority", r.priorityScore},
                {"avgPercent", r.avgPercent},
                {"nextExamDays", r.daysToExam == 999 ? -1 : r.daysToExam}
            });
        }
        return sess;
    };

    nlohmann::json out;
    out["sessions"] = nlohmann::json::array();
    out["sessions"].push_back(buildSession(
        "Morning",   "07:00 - 09:30", 0, 1, "Hardest concept first — fresh mind"));
    out["sessions"].push_back(buildSession(
        "Afternoon", "14:00 - 16:00", 1, 1, "Practice problems & active recall"));
    out["sessions"].push_back(buildSession(
        "Evening",   "19:30 - 21:00", 2, 2, "Light revision & flashcards"));
    out["totalSubjects"] = static_cast<int>(ranked.size());
    out["topPriority"]   = ranked.empty() ? "" : ranked.front().name;
    return out;
}

}
