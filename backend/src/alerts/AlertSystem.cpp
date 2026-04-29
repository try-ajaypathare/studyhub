#include "alerts/AlertSystem.hpp"
#include "alerts/AlertFactory.hpp"
#include "utils/Logger.hpp"
#include "core/Student.hpp"

#include <algorithm>

namespace ssaas {

AlertSystem& AlertSystem::getInstance() {
    static AlertSystem instance;
    return instance;
}

int AlertSystem::subscribe(Listener cb) {
    int h = nextHandle++;
    listeners.emplace_back(h, std::move(cb));
    return h;
}

void AlertSystem::unsubscribe(int handle) {
    listeners.erase(
        std::remove_if(listeners.begin(), listeners.end(),
            [&](const std::pair<int, Listener>& p) { return p.first == handle; }),
        listeners.end());
}

void AlertSystem::refreshFrom(const Student& student) {
    currentAlerts = AlertFactory::deriveAlerts(student);

    // Sort by polymorphic priority score (descending) so highest-priority
    // alerts come first. Each alert's getPriorityScore() is virtual and
    // computed by the concrete subclass — Strategy + polymorphism pattern.
    std::sort(currentAlerts.begin(), currentAlerts.end(),
        [](const AlertPtr& a, const AlertPtr& b) {
            return a->getPriorityScore() > b->getPriorityScore();
        });

    Logger::getInstance().info(
        "AlertSystem refreshed: " + std::to_string(currentAlerts.size()) + " alert(s)");

    auto snapshot = toJson();
    for (auto& kv : listeners) {
        try { kv.second(snapshot); }
        catch (const std::exception& e) {
            Logger::getInstance().warn(std::string("Listener threw: ") + e.what());
        }
    }
}

int AlertSystem::countOf(AlertSeverity s) const {
    int n = 0;
    for (const auto& a : currentAlerts) if (a->getSeverity() == s) n++;
    return n;
}

nlohmann::json AlertSystem::toJson() const {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& a : currentAlerts) arr.push_back(a->toJson());

    int p0 = 0, p1 = 0, p2 = 0, p3 = 0;
    for (const auto& a : currentAlerts) {
        switch (a->getTier()) {
            case PriorityTier::P0: p0++; break;
            case PriorityTier::P1: p1++; break;
            case PriorityTier::P2: p2++; break;
            case PriorityTier::P3: p3++; break;
        }
    }

    nlohmann::json out;
    out["alerts"] = arr;
    out["counts"] = {
        {"critical", countOf(AlertSeverity::CRITICAL)},
        {"warning",  countOf(AlertSeverity::WARNING)},
        {"info",     countOf(AlertSeverity::INFO)}
    };
    out["tierCounts"] = {
        {"P0", p0}, {"P1", p1}, {"P2", p2}, {"P3", p3}
    };
    return out;
}

}
