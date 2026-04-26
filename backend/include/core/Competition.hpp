#ifndef SSAAS_CORE_COMPETITION_HPP
#define SSAAS_CORE_COMPETITION_HPP

#include "core/Date.hpp"
#include <string>

namespace ssaas {

enum class CompetitionStatus { OPEN, REGISTERED, WON, MISSED, CLOSED };

std::string competitionStatusName(CompetitionStatus s);
CompetitionStatus parseCompetitionStatus(const std::string& s);

class Competition {
public:
    Competition(int id,
                std::string title,
                std::string organizer,
                std::string category,
                Date        deadline,
                std::string prize       = "",
                std::string description = "",
                CompetitionStatus status = CompetitionStatus::OPEN);

    int                getId()          const { return id; }
    const std::string& getTitle()       const { return title; }
    const std::string& getOrganizer()   const { return organizer; }
    const std::string& getCategory()    const { return category; }
    const Date&        getDeadline()    const { return deadline; }
    const std::string& getPrize()       const { return prize; }
    const std::string& getDescription() const { return description; }
    CompetitionStatus  getStatus()      const { return status; }

    void setTitle(const std::string& t);
    void setOrganizer(const std::string& o)    { organizer = o; }
    void setCategory(const std::string& c)     { category = c; }
    void setDeadline(Date d)                    { deadline = d; }
    void setPrize(const std::string& p)        { prize = p; }
    void setDescription(const std::string& d)  { description = d; }
    void setStatus(CompetitionStatus s)        { status = s; }

    int  daysUntilDeadline() const;
    bool isOpen() const;

private:
    int               id;
    std::string       title;
    std::string       organizer;
    std::string       category;
    Date              deadline;
    std::string       prize;
    std::string       description;
    CompetitionStatus status;
};

}

#endif
