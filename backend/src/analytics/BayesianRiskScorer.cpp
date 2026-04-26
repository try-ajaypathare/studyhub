#include "analytics/BayesianRiskScorer.hpp"
#include "analytics/GradePredictor.hpp"
#include "core/Student.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ssaas {

constexpr double BayesianRiskScorer::PRIOR_AT_RISK;

namespace {
// Each indicator returns a likelihood ratio LR = P(E|risk) / P(E|safe).
// We accumulate logs: log_posterior_odds = log_prior_odds + sum log(LR).
struct Indicator {
    std::string name;
    bool        active;
    double      strength;      // 0..1 — how strong is the evidence
    double      likelihoodRatio;
};

// Map a sigmoid-scaled severity to a likelihood ratio.
//   strength=0   ->  LR = 1   (no info)
//   strength=1   ->  LR = maxRatio
double scaledLR(double strength, double maxRatio) {
    strength = std::max(0.0, std::min(1.0, strength));
    return std::pow(maxRatio, strength);
}
}

nlohmann::json BayesianRiskScorer::analyze(const Student& student) const {
    std::vector<Indicator> indicators;

    // Indicator 1 — attendance below 75
    double att = student.getOverallAttendancePercentage();
    {
        Indicator i{"low_attendance", false, 0.0, 1.0};
        if (att < 75.0) {
            i.active   = true;
            i.strength = std::min(1.0, (75.0 - att) / 25.0);  // 75->0, 50->1
            i.likelihoodRatio = scaledLR(i.strength, 6.0);
        }
        indicators.push_back(i);
    }

    // Indicator 2 — performance below 60
    double perf = student.getCreditWeightedAveragePercentage();
    {
        Indicator i{"low_performance", false, 0.0, 1.0};
        if (perf < 60.0 && perf > 0.0) {
            i.active   = true;
            i.strength = std::min(1.0, (60.0 - perf) / 30.0);
            i.likelihoodRatio = scaledLR(i.strength, 5.0);
        }
        indicators.push_back(i);
    }

    // Indicator 3 — latest assessment drop
    int droppedSubjects = 0;
    for (const auto& s : student.getSubjects()) {
        auto m = student.getMarksFor(s.getCode());
        if (m.size() < 2) continue;
        double histAvg = 0.0;
        for (size_t i = 0; i < m.size() - 1; ++i) histAvg += m[i].getPercentage();
        histAvg /= (m.size() - 1);
        if (m.back().getPercentage() < histAvg - 8.0) droppedSubjects++;
    }
    {
        Indicator i{"recent_drop", false, 0.0, 1.0};
        if (droppedSubjects > 0 && !student.getSubjects().empty()) {
            i.active   = true;
            i.strength = std::min(1.0, droppedSubjects / static_cast<double>(student.getSubjects().size()));
            i.likelihoodRatio = scaledLR(i.strength, 4.0);
        }
        indicators.push_back(i);
    }

    // Indicator 4 — declining slope on credit-weighted regression
    double overallSlope = 0.0;
    int    slopeCount = 0;
    for (const auto& s : student.getSubjects()) {
        auto m = student.getMarksFor(s.getCode());
        if (m.size() < 2) continue;
        std::vector<double> xs, ys;
        for (size_t i = 0; i < m.size(); ++i) {
            xs.push_back(static_cast<double>(i + 1));
            ys.push_back(m[i].getPercentage());
        }
        auto f = GradePredictor::fitLinear(xs, ys);
        if (f.valid) { overallSlope += f.slope; slopeCount++; }
    }
    if (slopeCount > 0) overallSlope /= slopeCount;
    {
        Indicator i{"declining_trend", false, 0.0, 1.0};
        if (overallSlope < -0.5) {
            i.active   = true;
            i.strength = std::min(1.0, -overallSlope / 5.0);
            i.likelihoodRatio = scaledLR(i.strength, 3.0);
        }
        indicators.push_back(i);
    }

    // Indicator 5 — imminent exam with poor prep
    int riskyExams = 0;
    for (const auto& ex : student.getUpcomingExams()) {
        if (ex.daysUntil() <= 7 && ex.getPreparationProgress() < 50)
            riskyExams++;
    }
    {
        Indicator i{"unprepared_exam", false, 0.0, 1.0};
        if (riskyExams > 0) {
            i.active   = true;
            i.strength = std::min(1.0, riskyExams / 3.0);
            i.likelihoodRatio = scaledLR(i.strength, 4.0);
        }
        indicators.push_back(i);
    }

    // Combine in log-odds space.
    double priorOdds = PRIOR_AT_RISK / (1.0 - PRIOR_AT_RISK);
    double logOdds   = std::log(priorOdds);
    for (const auto& i : indicators) {
        if (i.active) logOdds += std::log(i.likelihoodRatio);
    }
    double posteriorOdds = std::exp(logOdds);
    double posterior     = posteriorOdds / (1.0 + posteriorOdds);

    std::string band;
    if (posterior >= 0.70) band = "Critical";
    else if (posterior >= 0.45) band = "High";
    else if (posterior >= 0.25) band = "Moderate";
    else band = "Low";

    nlohmann::json out;
    out["prior"]         = PRIOR_AT_RISK;
    out["posterior"]     = posterior;
    out["riskScore"]     = posterior * 100.0;
    out["band"]          = band;
    out["indicators"]    = nlohmann::json::array();
    for (const auto& i : indicators) {
        out["indicators"].push_back({
            {"name", i.name},
            {"active", i.active},
            {"strength", i.strength},
            {"likelihoodRatio", i.likelihoodRatio}
        });
    }
    out["model"] = "Naive-Bayes log-odds aggregation of academic indicators";
    return out;
}

}
