// Smoke test: core + analytics + alerts + storage end-to-end.
#include "storage/DataStore.hpp"
#include "alerts/AlertSystem.hpp"

#include "analytics/HealthScoreAnalyzer.hpp"
#include "analytics/BunkPredictor.hpp"
#include "analytics/AttendanceRiskAnalyzer.hpp"
#include "analytics/PerformanceAnalyzer.hpp"
#include "analytics/PressureAnalyzer.hpp"
#include "analytics/GradePredictor.hpp"
#include "analytics/BayesianRiskScorer.hpp"
#include "analytics/BurnoutDetector.hpp"
#include "analytics/ExamCountdownAnalyzer.hpp"
#include "analytics/StudyPlanAnalyzer.hpp"

#include "utils/Logger.hpp"

#include <iostream>
#include <memory>
#include <vector>

using namespace ssaas;

int main() {
    Logger::getInstance().setMinLevel(LogLevel::DEBUG);
    Logger::getInstance().info("Boot smoke test");

    DataStore::getInstance().bootstrap("data/sample_data.json");
    const auto& s = DataStore::getInstance().activeStudent();

    AlertSystem::getInstance().subscribe([](const nlohmann::json& snap){
        Logger::getInstance().info("Listener got " +
            std::to_string(snap["counts"]["critical"].get<int>()) + " critical alerts");
    });
    AlertSystem::getInstance().refreshFrom(s);

    std::cout << "\n=== " << s.introduce() << " ===\n";
    std::cout << "Overall attendance: " << s.getOverallAttendancePercentage() << "%\n";
    std::cout << "Credit-weighted avg: " << s.getCreditWeightedAveragePercentage() << "%\n\n";

    std::vector<std::unique_ptr<IAnalyzer>> analyzers;
    analyzers.emplace_back(new HealthScoreAnalyzer());
    analyzers.emplace_back(new BunkPredictor());
    analyzers.emplace_back(new AttendanceRiskAnalyzer());
    analyzers.emplace_back(new PerformanceAnalyzer());
    analyzers.emplace_back(new PressureAnalyzer());
    analyzers.emplace_back(new GradePredictor());
    analyzers.emplace_back(new BayesianRiskScorer());
    analyzers.emplace_back(new BurnoutDetector());
    analyzers.emplace_back(new ExamCountdownAnalyzer());
    analyzers.emplace_back(new StudyPlanAnalyzer());

    for (const auto& a : analyzers) {
        std::cout << "--- " << a->getName() << " ---\n";
        std::cout << a->analyze(s).dump(2) << "\n\n";
    }

    std::cout << "--- Alerts ---\n";
    std::cout << AlertSystem::getInstance().toJson().dump(2) << "\n";
    return 0;
}
