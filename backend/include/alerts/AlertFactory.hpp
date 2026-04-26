#ifndef SSAAS_ALERTS_ALERT_FACTORY_HPP
#define SSAAS_ALERTS_ALERT_FACTORY_HPP

#include "alerts/IAlert.hpp"
#include <vector>

namespace ssaas {

class Student;

// Factory pattern: derives a fresh batch of alerts from a Student snapshot.
// Decouples alert creation from the analyzers and from the AlertSystem.
class AlertFactory {
public:
    static std::vector<AlertPtr> deriveAlerts(const Student& student);
};

}

#endif
