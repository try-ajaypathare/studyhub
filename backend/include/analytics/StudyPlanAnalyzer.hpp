#ifndef SSAAS_ANALYTICS_STUDY_PLAN_HPP
#define SSAAS_ANALYTICS_STUDY_PLAN_HPP

#include "analytics/IAnalyzer.hpp"

namespace ssaas {

// Generates today's session-based study plan. Subjects with the lowest current
// average and the closest exams get prioritized into Morning/Afternoon/Evening.
class StudyPlanAnalyzer : public IAnalyzer {
public:
    std::string getName() const override { return "StudyPlan"; }
    nlohmann::json analyze(const Student& student) const override;
};

}

#endif
