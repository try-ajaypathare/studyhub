#ifndef SSAAS_CORE_SUBJECT_HPP
#define SSAAS_CORE_SUBJECT_HPP

#include <string>
#include <iosfwd>

namespace ssaas {

// Encapsulates subject metadata + attendance counters.
// Demonstrates encapsulation, operator overloading, derived metrics.
class Subject {
public:
    Subject(std::string code,
            std::string name,
            int credits,
            std::string professor = "",
            std::string schedule  = "");

    // accessors
    const std::string& getCode()      const { return code; }
    const std::string& getName()      const { return name; }
    int                getCredits()   const { return credits; }
    const std::string& getProfessor() const { return professor; }
    const std::string& getSchedule()  const { return schedule; }

    int getTotalLectures()    const { return totalLectures; }
    int getAttendedLectures() const { return attendedLectures; }

    void setProfessor(const std::string& p) { professor = p; }
    void setSchedule(const std::string& s)  { schedule = s; }

    // Marking attendance
    void markAttendance(bool present);
    void resetAttendance();
    void setAttendance(int total, int attended);

    // Derived
    double getAttendancePercentage() const;
    int    getMissedLectures()       const { return totalLectures - attendedLectures; }

    // ordering by code (useful for sorted views)
    bool operator<(const Subject& other) const { return code < other.code; }
    bool operator==(const Subject& other) const { return code == other.code; }

    friend std::ostream& operator<<(std::ostream& os, const Subject& s);

private:
    std::string code;
    std::string name;
    int         credits;
    std::string professor;
    std::string schedule;

    int totalLectures;
    int attendedLectures;
};

}

#endif
