#ifndef SSAAS_ANALYTICS_BAYESIAN_RISK_HPP
#define SSAAS_ANALYTICS_BAYESIAN_RISK_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Naive-Bayes-style aggregation of independent risk indicators into a single
// posterior P(at_risk | evidence). Each indicator contributes a likelihood
// ratio; we combine them in log-space to avoid underflow, then map back to
// a probability in [0, 1].
//
// Indicators considered:
//   1. Attendance below 75 (severity-scaled)
//   2. Performance below 60 (severity-scaled)
//   3. Latest assessment dropped vs historical
//   4. Negative trend slope from regression
//   5. Exam within 7 days while preparation < 50
class BayesianRiskScorer : public IAnalyzer {
public:
    static constexpr double PRIOR_AT_RISK = 0.20;   // 20% baseline

    std::string getName() const override { return "BayesianRisk"; }
    nlohmann::json analyze(const Student& student) const override;
};

}

#endif
