#ifndef SSAAS_CORE_STUDENT_HPP
#define SSAAS_CORE_STUDENT_HPP

#include "core/Person.hpp"
#include "core/Subject.hpp"
#include "core/AttendanceRecord.hpp"
#include "core/MarksRecord.hpp"
#include "core/Exam.hpp"
#include "core/Event.hpp"
#include "core/Competition.hpp"
#include "core/Goal.hpp"

#include <string>
#include <vector>

namespace ssaas {

// Aggregate root: a Student owns subjects/attendance/marks/exams/events/comps/goals.
// Demonstrates inheritance (extends Person), composition, encapsulation, full CRUD.
class Student : public Person {
public:
    Student(std::string name,
            std::string email,
            std::string rollNumber,
            std::string program,
            int         semester);

    const std::string& getRollNumber() const { return rollNumber; }
    const std::string& getProgram()    const { return program; }
    int                getSemester()   const { return semester; }

    void setSemester(int s);
    void setName(const std::string& n)    { Person::setName(n); }
    void setEmail(const std::string& e)   { Person::setEmail(e); }
    void setProgram(const std::string& p) { program = p; }

    // Person overrides
    std::string getRole() const override { return "Student"; }
    std::string introduce() const override;

    // ---------- Subjects ----------
    void                        addSubject(const Subject& s);
    void                        updateSubject(const std::string& code, const Subject& patch);
    void                        removeSubject(const std::string& code);
    bool                        hasSubject(const std::string& code) const;
    Subject&                    getSubject(const std::string& code);
    const Subject&              getSubject(const std::string& code) const;
    std::vector<Subject>&       getSubjects()       { return subjects; }
    const std::vector<Subject>& getSubjects() const { return subjects; }

    // ---------- Attendance ----------
    void addAttendance(const AttendanceRecord& a);
    const std::vector<AttendanceRecord>& getAttendanceRecords() const { return attendance; }
    std::vector<AttendanceRecord> getAttendanceFor(const std::string& subjectCode) const;

    // ---------- Marks ----------
    void addMarks(const MarksRecord& m);
    void updateMarks(size_t index, const MarksRecord& patch);
    void removeMarks(size_t index);
    const std::vector<MarksRecord>& getMarksRecords() const { return marks; }
    std::vector<MarksRecord> getMarksFor(const std::string& subjectCode) const;

    // ---------- Exams ----------
    void addExam(const Exam& e);
    void updateExam(size_t index, const Exam& patch);
    void removeExam(size_t index);
    const std::vector<Exam>& getExams() const { return exams; }
    std::vector<Exam>&       getExams()       { return exams; }
    std::vector<Exam> getUpcomingExams() const;
    std::vector<Exam> getExamsFor(const std::string& subjectCode) const;

    // ---------- Events ----------
    int  addEvent(const Event& e);             // returns assigned id
    void updateEvent(int id, const Event& patch);
    void removeEvent(int id);
    const std::vector<Event>& getEvents() const { return events; }
    Event&                    getEvent(int id);
    std::vector<Event>        getUpcomingEvents() const;

    // ---------- Competitions ----------
    int  addCompetition(const Competition& c);
    void updateCompetition(int id, const Competition& patch);
    void removeCompetition(int id);
    const std::vector<Competition>& getCompetitions() const { return competitions; }
    Competition&                    getCompetition(int id);

    // ---------- Goals ----------
    int  addGoal(const Goal& g);
    void updateGoal(int id, const Goal& patch);
    void removeGoal(int id);
    const std::vector<Goal>& getGoals() const { return goals; }
    Goal&                    getGoal(int id);

    // ---------- Aggregates ----------
    double getOverallAttendancePercentage() const;
    double getOverallAveragePercentage()    const;
    double getCreditWeightedAveragePercentage() const;
    int    nextEventId()       { return ++lastEventId; }
    int    nextCompetitionId() { return ++lastCompetitionId; }
    int    nextGoalId()        { return ++lastGoalId; }

private:
    std::string rollNumber;
    std::string program;
    int         semester;

    std::vector<Subject>          subjects;
    std::vector<AttendanceRecord> attendance;
    std::vector<MarksRecord>      marks;
    std::vector<Exam>             exams;
    std::vector<Event>            events;
    std::vector<Competition>      competitions;
    std::vector<Goal>             goals;

    int lastEventId       = 0;
    int lastCompetitionId = 0;
    int lastGoalId        = 0;
};

}

#endif
