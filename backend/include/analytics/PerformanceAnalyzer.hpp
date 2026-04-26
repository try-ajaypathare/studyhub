#ifndef SSAAS_ANALYTICS_PERFORMANCE_HPP
#define SSAAS_ANALYTICS_PERFORMANCE_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Detects per-subject performance drops by comparing latest assessment with
// the historical mean of the prior assessments. Flags subjects where the
// drop is statistically interesting (>= 8 percentage points).
class PerformanceAnalyzer : public IAnalyzer {
public:
    static constexpr double DROP_THRESHOLD = 8.0;   // pct points

    std::string getName() const override { return "PerformanceDrop"; }
    nlohmann::json analyze(const Student& student) const override;
};

}

#endif
