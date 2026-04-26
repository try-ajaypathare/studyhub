#ifndef SSAAS_ANALYTICS_IANALYZER_HPP
#define SSAAS_ANALYTICS_IANALYZER_HPP

#include "third_party/json.hpp"
#include <string>

namespace ssaas {

class Student;

// Strategy pattern: each analyzer encapsulates one piece of academic intelligence.
// All analyzers expose a uniform interface so they can be composed/iterated.
class IAnalyzer {
public:
    virtual ~IAnalyzer() = default;

    // Human-readable identifier (e.g. "HealthScore", "BunkPredictor").
    virtual std::string getName() const = 0;

    // Run the analyzer on a student snapshot, return JSON-serializable result.
    virtual nlohmann::json analyze(const Student& student) const = 0;
};

}

#endif
