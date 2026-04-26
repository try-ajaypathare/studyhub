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
    nlohmann::json out;
    out["alerts"] = arr;
    out["counts"] = {
        {"critical", countOf(AlertSeverity::CRITICAL)},
        {"warning",  countOf(AlertSeverity::WARNING)},
        {"info",     countOf(AlertSeverity::INFO)}
    };
    return out;
}

}
