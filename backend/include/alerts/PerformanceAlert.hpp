#ifndef SSAAS_ALERTS_PERFORMANCE_ALERT_HPP
#define SSAAS_ALERTS_PERFORMANCE_ALERT_HPP

#include "alerts/IAlert.hpp"

namespace ssaas {

class PerformanceAlert : public IAlert {
public:
    PerformanceAlert(AlertSeverity s,
                     std::string subjectCode,
                     double dropDelta,
                     double historicalAvg,
                     double latestPercent);

    std::string getCategory() const override { return "Performance"; }
    std::string getMessage()  const override;
    std::string getActionHint() const override;
    int         getPriorityScore() const override;     // severity + drop magnitude
    nlohmann::json toJson() const override;

private:
    std::string subjectCode;
    double      dropDelta;        // negative => drop
    double      historicalAvg;
    double      latestPercent;
};

}

#endif
