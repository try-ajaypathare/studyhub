#ifndef SSAAS_ANALYTICS_ATTENDANCE_RISK_HPP
#define SSAAS_ANALYTICS_ATTENDANCE_RISK_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Per-subject risk classification: SAFE / WARNING / CRITICAL.
// Uses both current % and a forward projection (will the trend dip below 75%?).
class AttendanceRiskAnalyzer : public IAnalyzer {
public:
    enum class RiskLevel { SAFE, WARNING, CRITICAL };

    static std::string riskLevelName(RiskLevel l);
    static std::string riskColor(RiskLevel l);

    std::string getName() const override { return "AttendanceRisk"; }
    nlohmann::json analyze(const Student& student) const override;

private:
    static RiskLevel classify(double pct);
};

}

#endif
