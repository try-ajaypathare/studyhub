#ifndef SSAAS_CORE_EXAM_HPP
#define SSAAS_CORE_EXAM_HPP

#include "core/Date.hpp"
#include "core/MarksRecord.hpp"   // reuse AssessmentType

#include <string>

namespace ssaas {

class Exam {
public:
    Exam(std::string subjectCode,
         AssessmentType type,
         Date date,
         std::string time = "",
         std::string venue = "",
         double maxMarks = 100.0,
         int    preparationProgress = 0); // 0..100

    const std::string&  getSubjectCode() const { return subjectCode; }
    AssessmentType      getType()        const { return type; }
    const Date&         getDate()        const { return date; }
    const std::string&  getTime()        const { return time; }
    const std::string&  getVenue()       const { return venue; }
    double              getMaxMarks()    const { return maxMarks; }
    int                 getPreparationProgress() const { return preparationProgress; }

    void setPreparationProgress(int p);

    int daysUntil() const;       // signed; negative if past
    bool isUpcoming() const;     // strictly in future

    bool operator<(const Exam& other) const { return date < other.date; }

private:
    std::string    subjectCode;
    AssessmentType type;
    Date           date;
    std::string    time;
    std::string    venue;
    double         maxMarks;
    int            preparationProgress;
};

}

#endif
