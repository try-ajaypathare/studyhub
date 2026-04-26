#include "analytics/PerformanceAnalyzer.hpp"
#include "core/Student.hpp"

namespace ssaas {

constexpr double PerformanceAnalyzer::DROP_THRESHOLD;

nlohmann::json PerformanceAnalyzer::analyze(const Student& student) const {
    nlohmann::json out;
    out["subjects"] = nlohmann::json::array();
    int dropCount = 0;
    int riseCount = 0;

    for (const auto& s : student.getSubjects()) {
        auto subjMarks = student.getMarksFor(s.getCode());  // sorted by date
        if (subjMarks.empty()) continue;

        double avgAll = 0.0;
        for (const auto& m : subjMarks) avgAll += m.getPercentage();
        avgAll /= subjMarks.size();

        double latest = subjMarks.back().getPercentage();
        double historicalAvg = avgAll;
        if (subjMarks.size() >= 2) {
            historicalAvg = 0.0;
            for (size_t i = 0; i < subjMarks.size() - 1; ++i)
                historicalAvg += subjMarks[i].getPercentage();
            historicalAvg /= (subjMarks.size() - 1);
        }
        double delta = latest - historicalAvg;
        bool dropped = delta < -DROP_THRESHOLD;
        bool rose    = delta >  DROP_THRESHOLD;

        if (dropped) dropCount++;
        if (rose)    riseCount++;

        nlohmann::json sj;
        sj["code"]            = s.getCode();
        sj["name"]            = s.getName();
        sj["latestPercent"]   = latest;
        sj["historicalAvg"]   = historicalAvg;
        sj["overallAvg"]      = avgAll;
        sj["delta"]           = delta;
        sj["dropped"]         = dropped;
        sj["improved"]        = rose;
        sj["assessmentCount"] = (int)subjMarks.size();
        sj["recommendation"]  = dropped
            ? "Revise core concepts; allocate extra study time."
            : rose ? "Strong upward trend — keep momentum."
                   : "Stable performance.";
        out["subjects"].push_back(sj);
    }

    out["dropCount"]    = dropCount;
    out["improveCount"] = riseCount;
    return out;
}

}
