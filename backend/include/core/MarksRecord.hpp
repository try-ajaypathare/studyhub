#ifndef SSAAS_CORE_MARKS_RECORD_HPP
#define SSAAS_CORE_MARKS_RECORD_HPP

#include "core/Date.hpp"
#include <string>

namespace ssaas {

enum class AssessmentType {
    QUIZ,
    ASSIGNMENT,
    MID_TERM,
    END_TERM,
    PROJECT,
    PRACTICAL
};

std::string assessmentTypeName(AssessmentType type);
AssessmentType parseAssessmentType(const std::string& s);

class MarksRecord {
public:
    MarksRecord(std::string subjectCode,
                AssessmentType type,
                std::string title,
                double obtained,
                double maxMarks,
                Date date);

    const std::string&  getSubjectCode() const { return subjectCode; }
    AssessmentType      getType()        const { return type; }
    const std::string&  getTitle()       const { return title; }
    double              getObtained()    const { return obtained; }
    double              getMaxMarks()    const { return maxMarks; }
    const Date&         getDate()        const { return date; }

    double getPercentage() const;

    bool operator<(const MarksRecord& other) const { return date < other.date; }

private:
    std::string    subjectCode;
    AssessmentType type;
    std::string    title;
    double         obtained;
    double         maxMarks;
    Date           date;
};

}

#endif
