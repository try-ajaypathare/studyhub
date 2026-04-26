#ifndef SSAAS_ALERTS_EXAM_ALERT_HPP
#define SSAAS_ALERTS_EXAM_ALERT_HPP

#include "alerts/IAlert.hpp"

namespace ssaas {

class ExamAlert : public IAlert {
public:
    ExamAlert(AlertSeverity s,
              std::string subjectCode,
              std::string examType,
              int    daysUntil,
              int    preparationProgress);

    std::string getCategory()   const override { return "Exam"; }
    std::string getMessage()    const override;
    std::string getActionHint() const override;
    nlohmann::json toJson()     const override;

private:
    std::string subjectCode;
    std::string examType;
    int         daysUntil;
    int         preparationProgress;
};

}

#endif
