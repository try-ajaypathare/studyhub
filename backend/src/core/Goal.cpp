#include "core/Goal.hpp"
#include "utils/Exceptions.hpp"

#include <unordered_map>
#include <cctype>
#include <algorithm>

namespace ssaas {

std::string goalTypeName(GoalType t) {
    switch (t) {
        case GoalType::CGPA:        return "CGPA";
        case GoalType::ATTENDANCE:  return "Attendance";
        case GoalType::STUDY_HOURS: return "Study Hours";
        case GoalType::MARKS:       return "Marks";
        case GoalType::CUSTOM:      return "Custom";
    }
    return "Custom";
}

GoalType parseGoalType(const std::string& s) {
    static const std::unordered_map<std::string, GoalType> map = {
        {"cgpa",        GoalType::CGPA},
        {"attendance",  GoalType::ATTENDANCE},
        {"study_hours", GoalType::STUDY_HOURS},
        {"hours",       GoalType::STUDY_HOURS},
        {"marks",       GoalType::MARKS},
        {"custom",      GoalType::CUSTOM},
    };
    std::string lower;
    for (char c : s) lower.push_back(std::tolower(static_cast<unsigned char>(c)));
    auto it = map.find(lower);
    return it == map.end() ? GoalType::CUSTOM : it->second;
}

Goal::Goal(int i, GoalType ty, std::string t, double tgt, double cur,
           std::string dl, std::string n)
    : id(i), type(ty), title(std::move(t)), target(tgt), current(cur),
      deadline(std::move(dl)), note(std::move(n)) {
    if (title.empty()) throw ValidationException("Goal title empty");
    if (target <= 0)   throw ValidationException("Goal target must be > 0");
}

void Goal::setTitle(const std::string& t) {
    if (t.empty()) throw ValidationException("Goal title empty");
    title = t;
}

double Goal::progressPercent() const {
    if (target <= 0) return 0.0;
    double pct = (current / target) * 100.0;
    return std::max(0.0, std::min(100.0, pct));
}

}
