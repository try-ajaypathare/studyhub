#include "analytics/HealthScoreAnalyzer.hpp"
#include "core/Student.hpp"

namespace ssaas {

// C++14 requires explicit out-of-class definitions for odr-used static constexpr members.
constexpr double HealthScoreAnalyzer::WEIGHT_ATTENDANCE;
constexpr double HealthScoreAnalyzer::WEIGHT_PERFORMANCE;

std::string HealthScoreAnalyzer::statusFor(double s) {
    if (s >= 85.0) return "Excellent";
    if (s >= 70.0) return "Good";
    if (s >= 50.0) return "Average";
    return "Poor";
}

std::string HealthScoreAnalyzer::colorFor(double s) {
    if (s >= 85.0) return "#10b981";   // emerald
    if (s >= 70.0) return "#3b82f6";   // blue
    if (s >= 50.0) return "#f59e0b";   // amber
    return "#ef4444";                  // red
}

nlohmann::json HealthScoreAnalyzer::analyze(const Student& student) const {
    double att = student.getOverallAttendancePercentage();
    double perf = student.getCreditWeightedAveragePercentage();
    double score = WEIGHT_ATTENDANCE * att + WEIGHT_PERFORMANCE * perf;

    nlohmann::json out;
    out["score"]        = score;
    out["attendance"]   = att;
    out["performance"]  = perf;
    out["weights"]      = { {"attendance", WEIGHT_ATTENDANCE},
                            {"performance", WEIGHT_PERFORMANCE} };
    out["status"]       = statusFor(score);
    out["color"]        = colorFor(score);
    out["explanation"]  = "Health = 0.4 × attendance + 0.6 × performance";
    return out;
}

}
