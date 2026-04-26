#ifndef SSAAS_CORE_ATTENDANCE_RECORD_HPP
#define SSAAS_CORE_ATTENDANCE_RECORD_HPP

#include "core/Date.hpp"
#include <string>

namespace ssaas {

// Single per-day, per-subject attendance entry.
class AttendanceRecord {
public:
    AttendanceRecord(std::string subjectCode, Date date, bool present);

    const std::string& getSubjectCode() const { return subjectCode; }
    const Date&        getDate()        const { return date; }
    bool               isPresent()      const { return present; }

    void setPresent(bool p) { present = p; }

    // sort by date for trend views
    bool operator<(const AttendanceRecord& other) const { return date < other.date; }

private:
    std::string subjectCode;
    Date        date;
    bool        present;
};

}

#endif
