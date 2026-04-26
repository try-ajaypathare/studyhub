#ifndef SSAAS_CORE_DATE_HPP
#define SSAAS_CORE_DATE_HPP

#include <string>
#include <iosfwd>
#include <ctime>

namespace ssaas {

// Demonstrates: encapsulation, operator overloading, value semantics.
class Date {
public:
    Date();
    Date(int year, int month, int day);
    explicit Date(const std::string& iso);  // "YYYY-MM-DD"

    static Date today();
    static Date fromTimeT(std::time_t t);

    int getYear()  const { return year; }
    int getMonth() const { return month; }
    int getDay()   const { return day; }

    std::string toIso() const;        // "YYYY-MM-DD"
    std::string toReadable() const;   // "27 Apr 2026"
    std::time_t toTimeT() const;
    int toJulian() const;             // continuous day count (for diff)

    int daysUntil(const Date& other) const;   // signed
    int daysSince(const Date& other) const { return -daysUntil(other); }

    Date addDays(int n) const;

    // Operator overloads
    bool operator==(const Date& rhs) const;
    bool operator!=(const Date& rhs) const { return !(*this == rhs); }
    bool operator<(const Date& rhs)  const;
    bool operator<=(const Date& rhs) const { return *this < rhs || *this == rhs; }
    bool operator>(const Date& rhs)  const { return rhs < *this; }
    bool operator>=(const Date& rhs) const { return !(*this < rhs); }
    int  operator-(const Date& rhs)  const { return rhs.daysUntil(*this); }

    friend std::ostream& operator<<(std::ostream& os, const Date& d);

private:
    int year;
    int month;
    int day;

    void validate() const;
};

}

#endif
