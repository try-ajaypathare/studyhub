#ifndef SSAAS_ALERTS_ATTENDANCE_ALERT_HPP
#define SSAAS_ALERTS_ATTENDANCE_ALERT_HPP

#include "alerts/IAlert.hpp"

namespace ssaas {

class AttendanceAlert : public IAlert {
public:
    AttendanceAlert(AlertSeverity s,
                    std::string subjectCode,
                    double currentPercent,
                    double targetPercent = 75.0);

    std::string getCategory() const override { return "Attendance"; }
    std::string getMessage()  const override;
    std::string getActionHint() const override;

    nlohmann::json toJson() const override;

private:
    std::string subjectCode;
    double      currentPercent;
    double      targetPercent;
};

}

#endif
