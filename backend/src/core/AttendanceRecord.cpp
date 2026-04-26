#include "core/AttendanceRecord.hpp"
#include "utils/Exceptions.hpp"

namespace ssaas {

AttendanceRecord::AttendanceRecord(std::string code, Date d, bool p)
    : subjectCode(std::move(code)), date(d), present(p) {
    if (subjectCode.empty())
        throw ValidationException("AttendanceRecord requires subject code");
}

}
