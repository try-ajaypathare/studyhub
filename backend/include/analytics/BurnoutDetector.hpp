#ifndef SSAAS_ANALYTICS_BURNOUT_HPP
#define SSAAS_ANALYTICS_BURNOUT_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Detects burnout-style risk by combining attendance velocity (how fast %
// is falling) with consistency loss (variance jump in marks) and exam load
// in the next 14 days. Burnout is high when the student is missing more
// recent classes AND their marks variance is rising AND many exams approach.
class BurnoutDetector : public IAnalyzer {
public:
    enum class Level { CALM, WATCH, BURNOUT };

    std::string getName() const override { return "BurnoutDetector"; }
    nlohmann::json analyze(const Student& student) const override;

    static std::string levelName(Level l);
    static std::string levelColor(Level l);
};

}

#endif
