#include "core/Subject.hpp"
#include "utils/Exceptions.hpp"

#include <ostream>

namespace ssaas {

Subject::Subject(std::string c,
                 std::string n,
                 int cr,
                 std::string prof,
                 std::string sch)
    : code(std::move(c)),
      name(std::move(n)),
      credits(cr),
      professor(std::move(prof)),
      schedule(std::move(sch)),
      totalLectures(0),
      attendedLectures(0) {
    if (code.empty())  throw ValidationException("Subject code empty");
    if (name.empty())  throw ValidationException("Subject name empty");
    if (credits <= 0)  throw ValidationException("Credits must be positive");
}

void Subject::markAttendance(bool present) {
    totalLectures++;
    if (present) attendedLectures++;
}

void Subject::resetAttendance() {
    totalLectures = 0;
    attendedLectures = 0;
}

void Subject::setAttendance(int total, int attended) {
    if (total < 0 || attended < 0 || attended > total)
        throw ValidationException("Invalid attendance counters");
    totalLectures    = total;
    attendedLectures = attended;
}

double Subject::getAttendancePercentage() const {
    if (totalLectures == 0) return 0.0;
    return (100.0 * attendedLectures) / totalLectures;
}

std::ostream& operator<<(std::ostream& os, const Subject& s) {
    os << s.code << " - " << s.name << " (" << s.credits << " cr)";
    return os;
}

}
