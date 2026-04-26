#ifndef SSAAS_ALERTS_ALERT_SYSTEM_HPP
#define SSAAS_ALERTS_ALERT_SYSTEM_HPP

#include "alerts/IAlert.hpp"

#include <vector>
#include <functional>

namespace ssaas {

class Student;

// Observer + Singleton.
// Subscribers register a callback; whenever refreshFrom() recomputes alerts,
// each subscriber is notified with a JSON snapshot of the current alert list.
class AlertSystem {
public:
    using Listener = std::function<void(const nlohmann::json&)>;

    static AlertSystem& getInstance();
    AlertSystem(const AlertSystem&) = delete;
    AlertSystem& operator=(const AlertSystem&) = delete;

    // Subscriber management
    int  subscribe(Listener cb);
    void unsubscribe(int handle);

    // Recompute alerts for the given student & notify all subscribers.
    void refreshFrom(const Student& student);

    // Inspectors — returns the most recent alert set as JSON.
    const std::vector<AlertPtr>& current() const { return currentAlerts; }
    nlohmann::json toJson() const;

    // Severity counts for badge displays.
    int countOf(AlertSeverity s) const;

private:
    AlertSystem() : nextHandle(1) {}

    std::vector<AlertPtr>                 currentAlerts;
    std::vector<std::pair<int, Listener>> listeners;
    int                                   nextHandle;
};

}

#endif
