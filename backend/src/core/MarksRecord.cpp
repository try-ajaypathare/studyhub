#include "core/MarksRecord.hpp"
#include "utils/Exceptions.hpp"

#include <unordered_map>

namespace ssaas {

std::string assessmentTypeName(AssessmentType type) {
    switch (type) {
        case AssessmentType::QUIZ:        return "Quiz";
        case AssessmentType::ASSIGNMENT:  return "Assignment";
        case AssessmentType::MID_TERM:    return "Mid Term";
        case AssessmentType::END_TERM:    return "End Term";
        case AssessmentType::PROJECT:     return "Project";
        case AssessmentType::PRACTICAL:   return "Practical";
    }
    return "Unknown";
}

AssessmentType parseAssessmentType(const std::string& s) {
    static const std::unordered_map<std::string, AssessmentType> map = {
        {"quiz",       AssessmentType::QUIZ},
        {"assignment", AssessmentType::ASSIGNMENT},
        {"mid",        AssessmentType::MID_TERM},
        {"midterm",    AssessmentType::MID_TERM},
        {"mid_term",   AssessmentType::MID_TERM},
        {"end",        AssessmentType::END_TERM},
        {"endterm",    AssessmentType::END_TERM},
        {"end_term",   AssessmentType::END_TERM},
        {"project",    AssessmentType::PROJECT},
        {"practical",  AssessmentType::PRACTICAL}
    };
    std::string lower;
    for (char c : s) lower.push_back(std::tolower(static_cast<unsigned char>(c)));
    auto it = map.find(lower);
    if (it == map.end())
        throw ValidationException("Unknown assessment type: " + s);
    return it->second;
}

MarksRecord::MarksRecord(std::string code,
                         AssessmentType t,
                         std::string ti,
                         double obt,
                         double mx,
                         Date d)
    : subjectCode(std::move(code)),
      type(t),
      title(std::move(ti)),
      obtained(obt),
      maxMarks(mx),
      date(d) {
    if (subjectCode.empty())
        throw ValidationException("MarksRecord needs subject code");
    if (mx <= 0.0)
        throw ValidationException("maxMarks must be > 0");
    if (obt < 0.0 || obt > mx)
        throw ValidationException("obtained out of range [0, max]");
}

double MarksRecord::getPercentage() const {
    return (obtained / maxMarks) * 100.0;
}

}
