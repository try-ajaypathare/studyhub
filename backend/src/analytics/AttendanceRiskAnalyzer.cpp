#include "analytics/AttendanceRiskAnalyzer.hpp"
#include "core/Student.hpp"

#include <algorithm>

namespace ssaas {

std::string AttendanceRiskAnalyzer::riskLevelName(RiskLevel l) {
    switch (l) {
        case RiskLevel::SAFE:     return "Safe";
        case RiskLevel::WARNING:  return "Warning";
        case RiskLevel::CRITICAL: return "Critical";
    }
    return "?";
}

std::string AttendanceRiskAnalyzer::riskColor(RiskLevel l) {
    switch (l) {
        case RiskLevel::SAFE:     return "#10b981";
        case RiskLevel::WARNING:  return "#f59e0b";
        case RiskLevel::CRITICAL: return "#ef4444";
    }
    return "#6b7280";
}

AttendanceRiskAnalyzer::RiskLevel
AttendanceRiskAnalyzer::classify(double pct) {
    if (pct >= 80.0) return RiskLevel::SAFE;
    if (pct >= 70.0) return RiskLevel::WARNING;
    return RiskLevel::CRITICAL;
}

nlohmann::json AttendanceRiskAnalyzer::analyze(const Student& student) const {
    nlohmann::json out;
    out["subjects"] = nlohmann::json::array();
    int safeCount = 0, warnCount = 0, critCount = 0;

    for (const auto& s : student.getSubjects()) {
        double pct = s.getAttendancePercentage();
        RiskLevel rl = classify(pct);
        switch (rl) {
            case RiskLevel::SAFE:     safeCount++;  break;
            case RiskLevel::WARNING:  warnCount++;  break;
            case RiskLevel::CRITICAL: critCount++;  break;
        }

        // Trend: take last 10 attendance entries for this subject.
        auto records = student.getAttendanceFor(s.getCode());
        std::sort(records.begin(), records.end());
        size_t window = std::min<size_t>(records.size(), 10);
        int recentPresent = 0;
        for (size_t i = records.size() - window; i < records.size(); ++i)
            if (records[i].isPresent()) recentPresent++;
        double recentRate = window == 0 ? pct : (100.0 * recentPresent) / window;

        nlohmann::json sj;
        sj["code"]        = s.getCode();
        sj["name"]        = s.getName();
        sj["percentage"]  = pct;
        sj["risk"]        = riskLevelName(rl);
        sj["color"]       = riskColor(rl);
        sj["recentRate"]  = recentRate;
        sj["trending"]    = recentRate < pct - 5.0 ? "down"
                          : recentRate > pct + 5.0 ? "up"
                          : "stable";
        out["subjects"].push_back(sj);
    }

    double overall = student.getOverallAttendancePercentage();
    RiskLevel overallRisk = classify(overall);
    out["overall"] = {
        {"percentage", overall},
        {"risk",  riskLevelName(overallRisk)},
        {"color", riskColor(overallRisk)}
    };
    out["counts"] = {
        {"safe",     safeCount},
        {"warning",  warnCount},
        {"critical", critCount}
    };
    return out;
}

}
