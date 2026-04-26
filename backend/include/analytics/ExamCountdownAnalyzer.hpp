#ifndef SSAAS_ANALYTICS_EXAM_COUNTDOWN_HPP
#define SSAAS_ANALYTICS_EXAM_COUNTDOWN_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

class ExamCountdownAnalyzer : public IAnalyzer {
public:
    std::string getName() const override { return "ExamCountdown"; }
    nlohmann::json analyze(const Student& student) const override;
};

}

#endif
