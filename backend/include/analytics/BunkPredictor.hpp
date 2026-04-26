#ifndef SSAAS_ANALYTICS_BUNK_PREDICTOR_HPP
#define SSAAS_ANALYTICS_BUNK_PREDICTOR_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

class Subject;

// Calculates per-subject "safe bunk" budget given a target attendance %.
// If currently above target  -> how many more lectures can be missed.
// If currently below target  -> how many consecutive presents needed to recover.
class BunkPredictor : public IAnalyzer {
public:
    explicit BunkPredictor(double targetPercent = 75.0)
        : target(targetPercent) {}

    std::string getName() const override { return "BunkPredictor"; }
    nlohmann::json analyze(const Student& student) const override;

    // Static helpers — exposed for unit-test friendliness.
    static int safeBunks(int attended, int total, double targetPct);
    static int recoveryPresents(int attended, int total, double targetPct);

private:
    double target;
};

}

#endif
