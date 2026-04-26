#include "alerts/AlertFactory.hpp"
#include "alerts/AttendanceAlert.hpp"
#include "alerts/PerformanceAlert.hpp"
#include "alerts/ExamAlert.hpp"
#include "alerts/BurnoutAlert.hpp"

#include "analytics/BurnoutDetector.hpp"
#include "analytics/PerformanceAnalyzer.hpp"
#include "core/Student.hpp"

#include <algorithm>

namespace ssaas {

std::vector<AlertPtr> AlertFactory::deriveAlerts(const Student& student) {
    std::vector<AlertPtr> alerts;

    // Attendance alerts — per subject
    for (const auto& s : student.getSubjects()) {
        double pct = s.getAttendancePercentage();
        if (s.getTotalLectures() == 0) continue;
        AlertSeverity sev;
        if (pct < 65.0) sev = AlertSeverity::CRITICAL;
        else if (pct < 75.0) sev = AlertSeverity::WARNING;
        else continue;
        alerts.emplace_back(new AttendanceAlert(sev, s.getCode(), pct));
    }

    // Performance alerts — per subject (drop > threshold)
    for (const auto& s : student.getSubjects()) {
        auto m = student.getMarksFor(s.getCode());
        if (m.size() < 2) continue;
        double histAvg = 0.0;
        for (size_t i = 0; i < m.size() - 1; ++i) histAvg += m[i].getPercentage();
        histAvg /= (m.size() - 1);
        double latest = m.back().getPercentage();
        double delta  = latest - histAvg;
        if (delta < -PerformanceAnalyzer::DROP_THRESHOLD) {
            AlertSeverity sev = (delta < -15.0) ? AlertSeverity::CRITICAL
                                                : AlertSeverity::WARNING;
            alerts.emplace_back(
                new PerformanceAlert(sev, s.getCode(), delta, histAvg, latest));
        }
    }

    // Exam alerts — within 7 days OR low prep within 14 days
    for (const auto& ex : student.getUpcomingExams()) {
        int days = ex.daysUntil();
        int prep = ex.getPreparationProgress();
        if (days <= 3 || (days <= 7 && prep < 60) || (days <= 14 && prep < 30)) {
            AlertSeverity sev = (days <= 3 && prep < 50) ? AlertSeverity::CRITICAL
                              : AlertSeverity::WARNING;
            alerts.emplace_back(new ExamAlert(
                sev, ex.getSubjectCode(), assessmentTypeName(ex.getType()),
                days, prep));
        }
    }

    // Burnout alert — if BurnoutDetector says so
    BurnoutDetector bd;
    auto burnout = bd.analyze(student);
    double score = burnout["score"].get<double>();
    std::string level = burnout["level"].get<std::string>();
    if (score >= 60.0) {
        alerts.emplace_back(new BurnoutAlert(AlertSeverity::CRITICAL, score, level));
    } else if (score >= 30.0) {
        alerts.emplace_back(new BurnoutAlert(AlertSeverity::WARNING, score, level));
    }

    // Sort by severity (critical first), then by category for stability.
    std::sort(alerts.begin(), alerts.end(),
        [](const AlertPtr& a, const AlertPtr& b) {
            if (a->getSeverity() != b->getSeverity())
                return static_cast<int>(a->getSeverity()) > static_cast<int>(b->getSeverity());
            return a->getCategory() < b->getCategory();
        });

    return alerts;
}

}
