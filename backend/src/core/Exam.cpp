#include "core/Exam.hpp"
#include "utils/Exceptions.hpp"

namespace ssaas {

Exam::Exam(std::string code,
           AssessmentType t,
           Date d,
           std::string tm,
           std::string v,
           double mx,
           int prep)
    : subjectCode(std::move(code)),
      type(t),
      date(d),
      time(std::move(tm)),
      venue(std::move(v)),
      maxMarks(mx),
      preparationProgress(prep) {
    if (subjectCode.empty()) throw ValidationException("Exam needs subject code");
    if (maxMarks <= 0.0) throw ValidationException("Exam maxMarks must be > 0");
    if (prep < 0 || prep > 100)
        throw ValidationException("preparationProgress must be 0..100");
}

void Exam::setPreparationProgress(int p) {
    if (p < 0 || p > 100)
        throw ValidationException("preparationProgress must be 0..100");
    preparationProgress = p;
}

int Exam::daysUntil() const {
    return Date::today().daysUntil(date);
}

bool Exam::isUpcoming() const {
    return daysUntil() >= 0;
}

}
