#include "core/Event.hpp"
#include "utils/Exceptions.hpp"

#include <unordered_map>
#include <cctype>

namespace ssaas {

std::string eventTypeName(EventType t) {
    switch (t) {
        case EventType::ACADEMIC: return "Academic";
        case EventType::CULTURAL: return "Cultural";
        case EventType::SPORTS:   return "Sports";
        case EventType::CAREER:   return "Career";
        case EventType::WORKSHOP: return "Workshop";
        case EventType::OTHER:    return "Other";
    }
    return "Other";
}

EventType parseEventType(const std::string& s) {
    static const std::unordered_map<std::string, EventType> map = {
        {"academic", EventType::ACADEMIC},
        {"cultural", EventType::CULTURAL},
        {"sports",   EventType::SPORTS},
        {"career",   EventType::CAREER},
        {"workshop", EventType::WORKSHOP},
        {"other",    EventType::OTHER},
    };
    std::string lower;
    for (char c : s) lower.push_back(std::tolower(static_cast<unsigned char>(c)));
    auto it = map.find(lower);
    return it == map.end() ? EventType::OTHER : it->second;
}

Event::Event(int i, std::string t, EventType ty, Date d,
             std::string tm, std::string loc, std::string desc, bool reg)
    : id(i), title(std::move(t)), type(ty), date(d),
      time(std::move(tm)), location(std::move(loc)),
      description(std::move(desc)), registered(reg) {
    if (title.empty()) throw ValidationException("Event title empty");
}

void Event::setTitle(const std::string& t) {
    if (t.empty()) throw ValidationException("Event title empty");
    title = t;
}

int  Event::daysUntil()  const { return Date::today().daysUntil(date); }
bool Event::isUpcoming() const { return daysUntil() >= 0; }

}
