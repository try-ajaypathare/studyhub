#ifndef SSAAS_ANALYTICS_PRESSURE_HPP
#define SSAAS_ANALYTICS_PRESSURE_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Aggregates academic stress signals into a single Low/Medium/High meter.
// Inputs:  attendance gap below 75, performance gap below 60, exam proximity (<=14d).
class PressureAnalyzer : public IAnalyzer {
public:
    enum class Level { LOW, MEDIUM, HIGH };

    std::string getName() const override { return "AcademicPressure"; }
    nlohmann::json analyze(const Student& student) const override;

    static std::string levelName(Level l);
    static std::string levelColor(Level l);
};

}

#endif
