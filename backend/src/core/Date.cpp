#include "core/Date.hpp"
#include "utils/Exceptions.hpp"

#include <ostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace ssaas {

namespace {
// Convert to Julian-day-like integer for arithmetic (Fliegel & Van Flandern algorithm).
int toJulianImpl(int y, int m, int d) {
    int a = (14 - m) / 12;
    int y2 = y + 4800 - a;
    int m2 = m + 12 * a - 3;
    return d + (153 * m2 + 2) / 5 + 365 * y2
           + y2 / 4 - y2 / 100 + y2 / 400 - 32045;
}

bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int year, int month) {
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 0;
    if (month == 2 && isLeap(year)) return 29;
    return dim[month - 1];
}

const char* monthName(int m) {
    static const char* names[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    if (m < 1 || m > 12) return "???";
    return names[m - 1];
}
}

Date::Date() {
    Date t = today();
    year  = t.year;
    month = t.month;
    day   = t.day;
}

Date::Date(int y, int m, int d) : year(y), month(m), day(d) {
    validate();
}

Date::Date(const std::string& iso) {
    if (iso.size() < 10 || iso[4] != '-' || iso[7] != '-')
        throw ValidationException("Invalid date format: " + iso);
    try {
        year  = std::stoi(iso.substr(0, 4));
        month = std::stoi(iso.substr(5, 2));
        day   = std::stoi(iso.substr(8, 2));
    } catch (const std::exception&) {
        throw ValidationException("Invalid date numerals: " + iso);
    }
    validate();
}

void Date::validate() const {
    if (month < 1 || month > 12)
        throw ValidationException("Month out of range");
    if (day < 1 || day > daysInMonth(year, month))
        throw ValidationException("Day out of range for given month");
}

Date Date::today() {
    std::time_t t = std::time(nullptr);
    return fromTimeT(t);
}

Date Date::fromTimeT(std::time_t t) {
    std::tm* tm = std::localtime(&t);
    if (!tm) return Date(2026, 1, 1);
    return Date(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
}

std::string Date::toIso() const {
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << year << '-'
        << std::setw(2) << std::setfill('0') << month << '-'
        << std::setw(2) << std::setfill('0') << day;
    return oss.str();
}

std::string Date::toReadable() const {
    std::ostringstream oss;
    oss << day << ' ' << monthName(month) << ' ' << year;
    return oss.str();
}

std::time_t Date::toTimeT() const {
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon  = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 12;
    return std::mktime(&tm);
}

int Date::toJulian() const {
    return toJulianImpl(year, month, day);
}

int Date::daysUntil(const Date& other) const {
    return other.toJulian() - toJulian();
}

Date Date::addDays(int n) const {
    int y = year, m = month, d = day + n;
    while (d > daysInMonth(y, m)) {
        d -= daysInMonth(y, m);
        m++;
        if (m > 12) { m = 1; y++; }
    }
    while (d < 1) {
        m--;
        if (m < 1) { m = 12; y--; }
        d += daysInMonth(y, m);
    }
    return Date(y, m, d);
}

bool Date::operator==(const Date& rhs) const {
    return year == rhs.year && month == rhs.month && day == rhs.day;
}

bool Date::operator<(const Date& rhs) const {
    if (year  != rhs.year)  return year  < rhs.year;
    if (month != rhs.month) return month < rhs.month;
    return day < rhs.day;
}

std::ostream& operator<<(std::ostream& os, const Date& d) {
    return os << d.toIso();
}

}
