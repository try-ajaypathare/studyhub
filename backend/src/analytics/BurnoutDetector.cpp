#include "analytics/BurnoutDetector.hpp"
#include "core/Student.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ssaas {

std::string BurnoutDetector::levelName(Level l) {
    switch (l) {
        case Level::CALM:    return "Calm";
        case Level::WATCH:   return "Watch";
        case Level::BURNOUT: return "Burnout Risk";
    }
    return "?";
}

std::string BurnoutDetector::levelColor(Level l) {
    switch (l) {
        case Level::CALM:    return "#10b981";
        case Level::WATCH:   return "#f59e0b";
        case Level::BURNOUT: return "#ef4444";
    }
    return "#6b7280";
}

namespace {
double variance(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    double sum = 0.0;
    for (double x : v) sum += (x - mean) * (x - mean);
    return sum / (v.size() - 1);
}
}

nlohmann::json BurnoutDetector::analyze(const Student& student) const {
    // Signal 1 — Attendance velocity (recent vs older window).
    double attVelocity = 0.0;
    {
        auto records = student.getAttendanceRecords();
        std::sort(records.begin(), records.end());
        if (records.size() >= 10) {
            size_t half = records.size() / 2;
            int firstP = 0, secondP = 0;
            for (size_t i = 0; i < half; ++i)              if (records[i].isPresent()) firstP++;
            for (size_t i = half; i < records.size(); ++i) if (records[i].isPresent()) secondP++;
            double firstRate  = (100.0 * firstP)  / half;
            double secondRate = (100.0 * secondP) / (records.size() - half);
            attVelocity = firstRate - secondRate; // positive => declining attendance
        }
    }

    // Signal 2 — Marks variance jump.
    double varianceJump = 0.0;
    {
        std::vector<double> early, late;
        for (const auto& s : student.getSubjects()) {
            auto m = student.getMarksFor(s.getCode());
            if (m.size() < 2) continue;
            size_t half = m.size() / 2;
            for (size_t i = 0; i < half; ++i)        early.push_back(m[i].getPercentage());
            for (size_t i = half; i < m.size(); ++i) late.push_back(m[i].getPercentage());
        }
        if (!early.empty() && !late.empty())
            varianceJump = variance(late) - variance(early);
    }

    // Signal 3 — Exam load in next 14 days.
    int examLoad = 0;
    for (const auto& ex : student.getUpcomingExams())
        if (ex.daysUntil() <= 14) examLoad++;

    // Normalize each signal to 0..1.
    double s1 = std::max(0.0, std::min(1.0, attVelocity / 30.0));     // 30pp drop = full
    double s2 = std::max(0.0, std::min(1.0, varianceJump / 200.0));   // var jump 200 = full
    double s3 = std::max(0.0, std::min(1.0, examLoad / 4.0));         // 4 exams = full

    double composite = 0.45 * s1 + 0.30 * s2 + 0.25 * s3;             // 0..1
    double score = composite * 100.0;

    Level level;
    if (score >= 60.0)      level = Level::BURNOUT;
    else if (score >= 30.0) level = Level::WATCH;
    else                    level = Level::CALM;

    nlohmann::json out;
    out["score"]            = score;
    out["level"]            = levelName(level);
    out["color"]            = levelColor(level);
    out["attendanceVelocity"]= attVelocity;
    out["marksVarianceJump"]= varianceJump;
    out["examLoad14d"]      = examLoad;
    out["components"] = {
        {"attendanceDecline", s1 * 100.0},
        {"inconsistency",     s2 * 100.0},
        {"examLoad",          s3 * 100.0}
    };
    out["advice"] =
        level == Level::BURNOUT ? "Take a recovery day; rebuild a sustainable schedule."
      : level == Level::WATCH   ? "Watch your sleep & take micro-breaks every 50 minutes."
                                : "You're pacing well — keep the rhythm.";
    return out;
}

}
