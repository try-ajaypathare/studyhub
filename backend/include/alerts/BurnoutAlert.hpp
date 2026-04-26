#ifndef SSAAS_ALERTS_BURNOUT_ALERT_HPP
#define SSAAS_ALERTS_BURNOUT_ALERT_HPP

#include "alerts/IAlert.hpp"

namespace ssaas {

class BurnoutAlert : public IAlert {
public:
    BurnoutAlert(AlertSeverity s, double score, std::string level);

    std::string getCategory()   const override { return "Wellbeing"; }
    std::string getMessage()    const override;
    std::string getActionHint() const override;
    nlohmann::json toJson()     const override;

private:
    double      score;
    std::string level;
};

}

#endif
