#include "analytics/BunkPredictor.hpp"
#include "core/Student.hpp"

#include <cmath>

namespace ssaas {

// Solve: attended / (total + b) >= target  ->  b <= attended/target - total
int BunkPredictor::safeBunks(int attended, int total, double targetPct) {
    double t = targetPct / 100.0;
    if (t <= 0.0) return 0;
    double maxTotal = static_cast<double>(attended) / t;
    int b = static_cast<int>(std::floor(maxTotal - total));
    return b > 0 ? b : 0;
}

// Solve: (attended + n) / (total + n) >= target  ->  n >= (target*total - attended) / (1 - target)
int BunkPredictor::recoveryPresents(int attended, int total, double targetPct) {
    double t = targetPct / 100.0;
    if (t >= 1.0) return -1; // impossible target
    double need = (t * total - attended) / (1.0 - t);
    if (need <= 0) return 0;
    return static_cast<int>(std::ceil(need));
}

nlohmann::json BunkPredictor::analyze(const Student& student) const {
    nlohmann::json out;
    out["target"]   = target;
    out["subjects"] = nlohmann::json::array();

    int totalSafeBunks = 0;
    int subjectsAtRisk = 0;

    for (const auto& s : student.getSubjects()) {
        int    a   = s.getAttendedLectures();
        int    t   = s.getTotalLectures();
        double pct = s.getAttendancePercentage();
        bool   ok  = pct >= target;

        nlohmann::json sj;
        sj["code"]       = s.getCode();
        sj["name"]       = s.getName();
        sj["attended"]   = a;
        sj["total"]      = t;
        sj["percentage"] = pct;
        sj["aboveTarget"] = ok;

        if (ok) {
            int b = safeBunks(a, t, target);
            sj["safeBunks"] = b;
            sj["recoveryPresents"] = 0;
            sj["message"] = "You can miss " + std::to_string(b) + " more lecture(s).";
            totalSafeBunks += b;
        } else {
            int r = recoveryPresents(a, t, target);
            sj["safeBunks"] = 0;
            sj["recoveryPresents"] = r;
            sj["message"] = "Attend next " + std::to_string(r) + " lecture(s) to recover.";
            subjectsAtRisk++;
        }
        out["subjects"].push_back(sj);
    }

    // Overall (sum across subjects)
    int totA = 0, totT = 0;
    for (const auto& s : student.getSubjects()) {
        totA += s.getAttendedLectures();
        totT += s.getTotalLectures();
    }
    double overallPct = totT == 0 ? 0.0 : (100.0 * totA) / totT;
    out["overall"] = {
        {"attended", totA},
        {"total", totT},
        {"percentage", overallPct},
        {"aboveTarget", overallPct >= target},
        {"safeBunks", overallPct >= target ? safeBunks(totA, totT, target) : 0},
        {"recoveryPresents", overallPct < target ? recoveryPresents(totA, totT, target) : 0}
    };
    out["totalSafeBunksAcrossSubjects"] = totalSafeBunks;
    out["subjectsAtRisk"] = subjectsAtRisk;
    return out;
}

}
