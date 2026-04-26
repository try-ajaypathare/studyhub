#ifndef SSAAS_ANALYTICS_HEALTH_SCORE_HPP
#define SSAAS_ANALYTICS_HEALTH_SCORE_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Health = 0.4 * attendance% + 0.6 * credit-weighted average%
// Status thresholds: Excellent (>=85), Good (70-84), Average (50-69), Poor (<50).
class HealthScoreAnalyzer : public IAnalyzer {
public:
    static constexpr double WEIGHT_ATTENDANCE = 0.4;
    static constexpr double WEIGHT_PERFORMANCE = 0.6;

    std::string getName() const override { return "HealthScore"; }
    nlohmann::json analyze(const Student& student) const override;

    static std::string statusFor(double score);
    static std::string colorFor(double score);
};

}

#endif
