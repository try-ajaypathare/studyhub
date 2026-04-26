#ifndef SSAAS_CORE_PERSON_HPP
#define SSAAS_CORE_PERSON_HPP

#include <string>

namespace ssaas {

// Abstract base class — demonstrates abstraction & encapsulation.
// Pure virtual getRole() forces derived classes to identify themselves.
class Person {
public:
    Person(std::string name, std::string email);
    virtual ~Person() = default;

    const std::string& getName()  const { return name; }
    const std::string& getEmail() const { return email; }

    void setName(const std::string& n);
    void setEmail(const std::string& e);

    virtual std::string getRole() const = 0;     // pure virtual
    virtual std::string introduce() const;       // virtual, overridable

protected:
    std::string name;
    std::string email;
};

}

#endif
