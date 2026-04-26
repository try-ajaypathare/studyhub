#include "analytics/PressureAnalyzer.hpp"
#include "core/Student.hpp"

#include <algorithm>

namespace ssaas {

std::string PressureAnalyzer::levelName(Level l) {
    switch (l) {
        case Level::LOW:    return "Low";
        case Level::MEDIUM: return "Medium";
        case Level::HIGH:   return "High";
    }
    return "?";
}

std::string PressureAnalyzer::levelColor(Level l) {
    switch (l) {
        case Level::LOW:    return "#10b981";
        case Level::MEDIUM: return "#f59e0b";
        case Level::HIGH:   return "#ef4444";
    }
    return "#6b7280";
}

nlohmann::json PressureAnalyzer::analyze(const Student& student) const {
    double att   = student.getOverallAttendancePercentage();
    double perf  = student.getCreditWeightedAveragePercentage();

    // Each signal contributes a 0..40 score.
    double attendanceGap  = std::max(0.0, 75.0 - att);          // 0..75
    double performanceGap = std::max(0.0, 60.0 - perf);          // 0..60
    double attScore  = std::min(40.0, attendanceGap * 0.8);     // ~ up to 40
    double perfScore = std::min(40.0, performanceGap * 0.8);

    auto upcoming = student.getUpcomingExams();
    int nearExams = 0;
    int nearestDays = 999;
    for (const auto& ex : upcoming) {
        int d = ex.daysUntil();
        if (d <= 14) nearExams++;
        nearestDays = std::min(nearestDays, d);
    }
    double examScore = std::min(20.0, nearExams * 6.0);
    if (nearestDays <= 3) examScore = std::min(20.0, examScore + 6.0);

    double total = attScore + perfScore + examScore;   // 0..100

    Level level;
    if (total >= 60.0)      level = Level::HIGH;
    else if (total >= 30.0) level = Level::MEDIUM;
    else                    level = Level::LOW;

    nlohmann::json out;
    out["score"]      = total;
    out["level"]      = levelName(level);
    out["color"]      = levelColor(level);
    out["components"] = {
        {"attendance",  attScore},
        {"performance", perfScore},
        {"exams",       examScore}
    };
    out["nearestExamDays"] = nearestDays == 999 ? -1 : nearestDays;
    out["upcomingExamCount"] = (int)upcoming.size();
    return out;
}

}
