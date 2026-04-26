#include "core/Person.hpp"
#include "utils/Exceptions.hpp"

namespace ssaas {

Person::Person(std::string n, std::string e)
    : name(std::move(n)), email(std::move(e)) {
    if (name.empty()) throw ValidationException("Person name cannot be empty");
    if (email.find('@') == std::string::npos)
        throw ValidationException("Person email must contain '@'");
}

void Person::setName(const std::string& n) {
    if (n.empty()) throw ValidationException("Name cannot be empty");
    name = n;
}

void Person::setEmail(const std::string& e) {
    if (e.find('@') == std::string::npos)
        throw ValidationException("Email must contain '@'");
    email = e;
}

std::string Person::introduce() const {
    return "Hi, I'm " + name + " (" + getRole() + ")";
}

}
