#include "core/Competition.hpp"
#include "utils/Exceptions.hpp"

#include <unordered_map>
#include <cctype>

namespace ssaas {

std::string competitionStatusName(CompetitionStatus s) {
    switch (s) {
        case CompetitionStatus::OPEN:       return "Open";
        case CompetitionStatus::REGISTERED: return "Registered";
        case CompetitionStatus::WON:        return "Won";
        case CompetitionStatus::MISSED:     return "Missed";
        case CompetitionStatus::CLOSED:     return "Closed";
    }
    return "Open";
}

CompetitionStatus parseCompetitionStatus(const std::string& s) {
    static const std::unordered_map<std::string, CompetitionStatus> map = {
        {"open",       CompetitionStatus::OPEN},
        {"registered", CompetitionStatus::REGISTERED},
        {"won",        CompetitionStatus::WON},
        {"missed",     CompetitionStatus::MISSED},
        {"closed",     CompetitionStatus::CLOSED},
    };
    std::string lower;
    for (char c : s) lower.push_back(std::tolower(static_cast<unsigned char>(c)));
    auto it = map.find(lower);
    return it == map.end() ? CompetitionStatus::OPEN : it->second;
}

Competition::Competition(int i, std::string t, std::string org, std::string cat,
                         Date d, std::string pr, std::string desc, CompetitionStatus st)
    : id(i), title(std::move(t)), organizer(std::move(org)),
      category(std::move(cat)), deadline(d),
      prize(std::move(pr)), description(std::move(desc)), status(st) {
    if (title.empty()) throw ValidationException("Competition title empty");
}

void Competition::setTitle(const std::string& t) {
    if (t.empty()) throw ValidationException("Competition title empty");
    title = t;
}

int Competition::daysUntilDeadline() const {
    return Date::today().daysUntil(deadline);
}

bool Competition::isOpen() const {
    return status == CompetitionStatus::OPEN
        || status == CompetitionStatus::REGISTERED;
}

}
