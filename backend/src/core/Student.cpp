#include "core/Student.hpp"
#include "utils/Exceptions.hpp"

#include <algorithm>
#include <numeric>

namespace ssaas {

Student::Student(std::string n,
                 std::string e,
                 std::string roll,
                 std::string prog,
                 int sem)
    : Person(std::move(n), std::move(e)),
      rollNumber(std::move(roll)),
      program(std::move(prog)),
      semester(sem) {
    if (rollNumber.empty()) throw ValidationException("rollNumber empty");
    if (program.empty())    throw ValidationException("program empty");
    if (sem < 1 || sem > 12) throw ValidationException("semester out of range");
}

void Student::setSemester(int s) {
    if (s < 1 || s > 12) throw ValidationException("semester out of range");
    semester = s;
}

std::string Student::introduce() const {
    return "Hi, I'm " + name + " (" + rollNumber + ") — " + program +
           ", Semester " + std::to_string(semester);
}

// ---------- Subjects ----------
void Student::addSubject(const Subject& s) {
    if (hasSubject(s.getCode()))
        throw ValidationException("Subject already exists: " + s.getCode());
    subjects.push_back(s);
}

void Student::updateSubject(const std::string& code, const Subject& patch) {
    auto& s = getSubject(code);
    // Replace the subject — preserve attendance counters since those are
    // not patched directly through this entry point.
    int total = s.getTotalLectures();
    int attended = s.getAttendedLectures();
    s = patch;
    s.setAttendance(total, attended);
}

void Student::removeSubject(const std::string& code) {
    auto it = std::find_if(subjects.begin(), subjects.end(),
        [&](const Subject& s) { return s.getCode() == code; });
    if (it == subjects.end())
        throw NotFoundException("Subject not found: " + code);
    subjects.erase(it);

    // Cascade: drop dependent records that reference this subject code.
    attendance.erase(std::remove_if(attendance.begin(), attendance.end(),
        [&](const AttendanceRecord& r) { return r.getSubjectCode() == code; }),
        attendance.end());
    marks.erase(std::remove_if(marks.begin(), marks.end(),
        [&](const MarksRecord& r) { return r.getSubjectCode() == code; }),
        marks.end());
    exams.erase(std::remove_if(exams.begin(), exams.end(),
        [&](const Exam& r) { return r.getSubjectCode() == code; }),
        exams.end());
}

bool Student::hasSubject(const std::string& code) const {
    return std::any_of(subjects.begin(), subjects.end(),
        [&](const Subject& s) { return s.getCode() == code; });
}

Subject& Student::getSubject(const std::string& code) {
    auto it = std::find_if(subjects.begin(), subjects.end(),
        [&](const Subject& s) { return s.getCode() == code; });
    if (it == subjects.end())
        throw NotFoundException("Subject not found: " + code);
    return *it;
}

const Subject& Student::getSubject(const std::string& code) const {
    auto it = std::find_if(subjects.begin(), subjects.end(),
        [&](const Subject& s) { return s.getCode() == code; });
    if (it == subjects.end())
        throw NotFoundException("Subject not found: " + code);
    return *it;
}

// ---------- Attendance ----------
void Student::addAttendance(const AttendanceRecord& a) {
    if (!hasSubject(a.getSubjectCode()))
        throw NotFoundException("Cannot record attendance — subject missing: " + a.getSubjectCode());
    getSubject(a.getSubjectCode()).markAttendance(a.isPresent());
    attendance.push_back(a);
}

std::vector<AttendanceRecord> Student::getAttendanceFor(const std::string& code) const {
    std::vector<AttendanceRecord> out;
    std::copy_if(attendance.begin(), attendance.end(), std::back_inserter(out),
        [&](const AttendanceRecord& r) { return r.getSubjectCode() == code; });
    return out;
}

// ---------- Marks ----------
void Student::addMarks(const MarksRecord& m) {
    if (!hasSubject(m.getSubjectCode()))
        throw NotFoundException("Cannot record marks — subject missing: " + m.getSubjectCode());
    marks.push_back(m);
}

void Student::updateMarks(size_t index, const MarksRecord& patch) {
    if (index >= marks.size())
        throw NotFoundException("Marks index out of range");
    if (!hasSubject(patch.getSubjectCode()))
        throw NotFoundException("Subject missing: " + patch.getSubjectCode());
    marks[index] = patch;
}

void Student::removeMarks(size_t index) {
    if (index >= marks.size())
        throw NotFoundException("Marks index out of range");
    marks.erase(marks.begin() + index);
}

std::vector<MarksRecord> Student::getMarksFor(const std::string& code) const {
    std::vector<MarksRecord> out;
    std::copy_if(marks.begin(), marks.end(), std::back_inserter(out),
        [&](const MarksRecord& r) { return r.getSubjectCode() == code; });
    std::sort(out.begin(), out.end());
    return out;
}

// ---------- Exams ----------
void Student::addExam(const Exam& e) {
    if (!hasSubject(e.getSubjectCode()))
        throw NotFoundException("Cannot add exam — subject missing: " + e.getSubjectCode());
    exams.push_back(e);
}

void Student::updateExam(size_t index, const Exam& patch) {
    if (index >= exams.size())
        throw NotFoundException("Exam index out of range");
    if (!hasSubject(patch.getSubjectCode()))
        throw NotFoundException("Subject missing: " + patch.getSubjectCode());
    exams[index] = patch;
}

void Student::removeExam(size_t index) {
    if (index >= exams.size())
        throw NotFoundException("Exam index out of range");
    exams.erase(exams.begin() + index);
}

std::vector<Exam> Student::getUpcomingExams() const {
    std::vector<Exam> upcoming;
    std::copy_if(exams.begin(), exams.end(), std::back_inserter(upcoming),
        [](const Exam& ex) { return ex.isUpcoming(); });
    std::sort(upcoming.begin(), upcoming.end());
    return upcoming;
}

std::vector<Exam> Student::getExamsFor(const std::string& code) const {
    std::vector<Exam> out;
    std::copy_if(exams.begin(), exams.end(), std::back_inserter(out),
        [&](const Exam& ex) { return ex.getSubjectCode() == code; });
    return out;
}

// ---------- Events ----------
int Student::addEvent(const Event& e) {
    events.push_back(e);
    return e.getId();
}

void Student::updateEvent(int id, const Event& patch) {
    auto it = std::find_if(events.begin(), events.end(),
        [&](const Event& e) { return e.getId() == id; });
    if (it == events.end()) throw NotFoundException("Event not found");
    *it = patch;
}

void Student::removeEvent(int id) {
    auto it = std::find_if(events.begin(), events.end(),
        [&](const Event& e) { return e.getId() == id; });
    if (it == events.end()) throw NotFoundException("Event not found");
    events.erase(it);
}

Event& Student::getEvent(int id) {
    auto it = std::find_if(events.begin(), events.end(),
        [&](Event& e) { return e.getId() == id; });
    if (it == events.end()) throw NotFoundException("Event not found");
    return *it;
}

std::vector<Event> Student::getUpcomingEvents() const {
    std::vector<Event> up;
    std::copy_if(events.begin(), events.end(), std::back_inserter(up),
        [](const Event& e) { return e.isUpcoming(); });
    std::sort(up.begin(), up.end());
    return up;
}

// ---------- Competitions ----------
int Student::addCompetition(const Competition& c) {
    competitions.push_back(c);
    return c.getId();
}

void Student::updateCompetition(int id, const Competition& patch) {
    auto it = std::find_if(competitions.begin(), competitions.end(),
        [&](const Competition& c) { return c.getId() == id; });
    if (it == competitions.end()) throw NotFoundException("Competition not found");
    *it = patch;
}

void Student::removeCompetition(int id) {
    auto it = std::find_if(competitions.begin(), competitions.end(),
        [&](const Competition& c) { return c.getId() == id; });
    if (it == competitions.end()) throw NotFoundException("Competition not found");
    competitions.erase(it);
}

Competition& Student::getCompetition(int id) {
    auto it = std::find_if(competitions.begin(), competitions.end(),
        [&](Competition& c) { return c.getId() == id; });
    if (it == competitions.end()) throw NotFoundException("Competition not found");
    return *it;
}

// ---------- Goals ----------
int Student::addGoal(const Goal& g) {
    goals.push_back(g);
    return g.getId();
}

void Student::updateGoal(int id, const Goal& patch) {
    auto it = std::find_if(goals.begin(), goals.end(),
        [&](const Goal& g) { return g.getId() == id; });
    if (it == goals.end()) throw NotFoundException("Goal not found");
    *it = patch;
}

void Student::removeGoal(int id) {
    auto it = std::find_if(goals.begin(), goals.end(),
        [&](const Goal& g) { return g.getId() == id; });
    if (it == goals.end()) throw NotFoundException("Goal not found");
    goals.erase(it);
}

Goal& Student::getGoal(int id) {
    auto it = std::find_if(goals.begin(), goals.end(),
        [&](Goal& g) { return g.getId() == id; });
    if (it == goals.end()) throw NotFoundException("Goal not found");
    return *it;
}

// ---------- Aggregates ----------
double Student::getOverallAttendancePercentage() const {
    int totalLec = 0, attendedLec = 0;
    for (const auto& s : subjects) {
        totalLec    += s.getTotalLectures();
        attendedLec += s.getAttendedLectures();
    }
    if (totalLec == 0) return 0.0;
    return (100.0 * attendedLec) / totalLec;
}

double Student::getOverallAveragePercentage() const {
    if (marks.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& m : marks) sum += m.getPercentage();
    return sum / marks.size();
}

double Student::getCreditWeightedAveragePercentage() const {
    double weightedSum = 0.0;
    int    totalCredits = 0;
    for (const auto& s : subjects) {
        auto subjMarks = getMarksFor(s.getCode());
        if (subjMarks.empty()) continue;
        double avg = 0.0;
        for (const auto& m : subjMarks) avg += m.getPercentage();
        avg /= subjMarks.size();
        weightedSum += avg * s.getCredits();
        totalCredits += s.getCredits();
    }
    if (totalCredits == 0) return getOverallAveragePercentage();
    return weightedSum / totalCredits;
}

}
