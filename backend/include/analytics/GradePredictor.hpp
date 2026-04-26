#ifndef SSAAS_ANALYTICS_GRADE_PREDICTOR_HPP
#define SSAAS_ANALYTICS_GRADE_PREDICTOR_HPP

#include "analytics/IAnalyzer.hpp"
#include <vector>

namespace ssaas {

// Linear regression over chronological assessment percentages per subject.
// Predicts the next assessment outcome and a final-exam score required to hit
// a target average. Returns slope, intercept, R², CI band, and "needed in finals".
class GradePredictor : public IAnalyzer {
public:
    explicit GradePredictor(double targetPercent = 75.0)
        : target(targetPercent) {}

    std::string getName() const override { return "GradePredictor"; }
    nlohmann::json analyze(const Student& student) const override;

    // OLS fit: y = slope*x + intercept. Returns r² as third value.
    struct Fit {
        double slope;
        double intercept;
        double rSquared;
        double residualStdDev;
        bool   valid;
    };

    static Fit fitLinear(const std::vector<double>& xs,
                         const std::vector<double>& ys);

    // Required score on a future assessment to hit target average.
    static double requiredForTarget(const std::vector<double>& history,
                                    double targetAvg,
                                    int   futureCount = 1);

private:
    double target;
};

}

#endif
