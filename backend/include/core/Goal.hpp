#ifndef SSAAS_CORE_GOAL_HPP
#define SSAAS_CORE_GOAL_HPP

#include "core/Date.hpp"
#include <string>

namespace ssaas {

enum class GoalType { CGPA, ATTENDANCE, STUDY_HOURS, MARKS, CUSTOM };

std::string goalTypeName(GoalType t);
GoalType    parseGoalType(const std::string& s);

class Goal {
public:
    Goal(int id,
         GoalType type,
         std::string title,
         double  target,
         double  current = 0.0,
         std::string deadline = "",
         std::string note    = "");

    int                getId()       const { return id; }
    GoalType           getType()     const { return type; }
    const std::string& getTitle()    const { return title; }
    double             getTarget()   const { return target; }
    double             getCurrent()  const { return current; }
    const std::string& getDeadline() const { return deadline; }
    const std::string& getNote()     const { return note; }

    void setTitle(const std::string& t);
    void setType(GoalType t)                    { type = t; }
    void setTarget(double t)                    { target = t; }
    void setCurrent(double c)                   { current = c; }
    void setDeadline(const std::string& d)      { deadline = d; }
    void setNote(const std::string& n)          { note = n; }

    double progressPercent() const;

private:
    int         id;
    GoalType    type;
    std::string title;
    double      target;
    double      current;
    std::string deadline;
    std::string note;
};

}

#endif
