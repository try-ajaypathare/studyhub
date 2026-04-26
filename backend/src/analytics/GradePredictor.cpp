#include "analytics/GradePredictor.hpp"
#include "core/Student.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ssaas {

GradePredictor::Fit
GradePredictor::fitLinear(const std::vector<double>& xs,
                          const std::vector<double>& ys) {
    Fit f{0.0, 0.0, 0.0, 0.0, false};
    if (xs.size() != ys.size() || xs.size() < 2) return f;

    const size_t n = xs.size();
    double sumX = std::accumulate(xs.begin(), xs.end(), 0.0);
    double sumY = std::accumulate(ys.begin(), ys.end(), 0.0);
    double meanX = sumX / n;
    double meanY = sumY / n;

    double sxx = 0.0, sxy = 0.0, syy = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double dx = xs[i] - meanX;
        double dy = ys[i] - meanY;
        sxx += dx * dx;
        sxy += dx * dy;
        syy += dy * dy;
    }
    if (sxx < 1e-9) {
        f.slope = 0.0;
        f.intercept = meanY;
        f.rSquared = 0.0;
        f.valid = true;
        return f;
    }

    f.slope     = sxy / sxx;
    f.intercept = meanY - f.slope * meanX;
    f.rSquared  = (syy < 1e-9) ? 1.0 : (sxy * sxy) / (sxx * syy);

    // Residual std deviation (sample-based).
    double ss_res = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double pred = f.slope * xs[i] + f.intercept;
        double r = ys[i] - pred;
        ss_res += r * r;
    }
    f.residualStdDev = std::sqrt(ss_res / std::max<size_t>(1, n - 1));
    f.valid = true;
    return f;
}

double GradePredictor::requiredForTarget(const std::vector<double>& history,
                                         double targetAvg,
                                         int futureCount) {
    if (futureCount <= 0) return targetAvg;
    double sum = std::accumulate(history.begin(), history.end(), 0.0);
    int totalCount = static_cast<int>(history.size()) + futureCount;
    return ((targetAvg * totalCount) - sum) / futureCount;
}

nlohmann::json GradePredictor::analyze(const Student& student) const {
    nlohmann::json out;
    out["target"]   = target;
    out["subjects"] = nlohmann::json::array();

    double overallPredictedAvg = 0.0;
    int    predictedSubjectCount = 0;

    for (const auto& s : student.getSubjects()) {
        auto subjMarks = student.getMarksFor(s.getCode());
        if (subjMarks.empty()) continue;

        std::vector<double> xs, ys;
        xs.reserve(subjMarks.size());
        ys.reserve(subjMarks.size());
        for (size_t i = 0; i < subjMarks.size(); ++i) {
            xs.push_back(static_cast<double>(i + 1));
            ys.push_back(subjMarks[i].getPercentage());
        }

        Fit f = fitLinear(xs, ys);
        double currentAvg = std::accumulate(ys.begin(), ys.end(), 0.0) / ys.size();
        double nextX      = static_cast<double>(ys.size() + 1);
        double prediction = f.valid ? (f.slope * nextX + f.intercept)
                                    : currentAvg;
        prediction = std::max(0.0, std::min(100.0, prediction));

        double ci = f.valid ? std::min(20.0, 1.96 * f.residualStdDev) : 10.0;
        double lo = std::max(0.0,   prediction - ci);
        double hi = std::min(100.0, prediction + ci);

        // Required for target — assume one final exam remaining.
        double need = requiredForTarget(ys, target, 1);
        bool achievable = need <= 100.0 && need >= 0.0;

        nlohmann::json sj;
        sj["code"]            = s.getCode();
        sj["name"]            = s.getName();
        sj["history"]         = ys;
        sj["currentAverage"]  = currentAvg;
        sj["slope"]           = f.slope;
        sj["intercept"]       = f.intercept;
        sj["rSquared"]        = f.rSquared;
        sj["prediction"]      = prediction;
        sj["confidenceLo"]    = lo;
        sj["confidenceHi"]    = hi;
        sj["trend"]           = f.slope >  0.5 ? "improving"
                              : f.slope < -0.5 ? "declining"
                              : "stable";
        sj["targetReachable"] = achievable;
        sj["requiredInFinals"]= need;
        sj["explanation"]     = "Linear OLS fit on " + std::to_string(ys.size())
                                + " assessments; predicting next attempt.";
        out["subjects"].push_back(sj);

        overallPredictedAvg += prediction;
        predictedSubjectCount++;
    }
    if (predictedSubjectCount > 0)
        overallPredictedAvg /= predictedSubjectCount;
    out["overallPredictedAverage"] = overallPredictedAvg;
    out["model"] = "Ordinary Least Squares (OLS) linear regression";
    return out;
}

}
