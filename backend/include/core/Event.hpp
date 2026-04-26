#ifndef SSAAS_CORE_EVENT_HPP
#define SSAAS_CORE_EVENT_HPP

#include "core/Date.hpp"
#include <string>

namespace ssaas {

enum class EventType { ACADEMIC, CULTURAL, SPORTS, CAREER, WORKSHOP, OTHER };

std::string eventTypeName(EventType t);
EventType   parseEventType(const std::string& s);

class Event {
public:
    Event(int id,
          std::string title,
          EventType   type,
          Date        date,
          std::string time        = "",
          std::string location    = "",
          std::string description = "",
          bool        registered  = false);

    int                getId()          const { return id; }
    const std::string& getTitle()       const { return title; }
    EventType          getType()        const { return type; }
    const Date&        getDate()        const { return date; }
    const std::string& getTime()        const { return time; }
    const std::string& getLocation()    const { return location; }
    const std::string& getDescription() const { return description; }
    bool               isRegistered()   const { return registered; }

    void setTitle(const std::string& t);
    void setType(EventType t)                  { type = t; }
    void setDate(Date d)                       { date = d; }
    void setTime(const std::string& t)         { time = t; }
    void setLocation(const std::string& l)     { location = l; }
    void setDescription(const std::string& d)  { description = d; }
    void setRegistered(bool r)                 { registered = r; }

    int  daysUntil()  const;
    bool isUpcoming() const;

    bool operator<(const Event& o) const { return date < o.date; }

private:
    int         id;
    std::string title;
    EventType   type;
    Date        date;
    std::string time;
    std::string location;
    std::string description;
    bool        registered;
};

}

#endif
