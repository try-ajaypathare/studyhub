#include "server/Controllers.hpp"
#include "storage/DataStore.hpp"
#include "alerts/AlertSystem.hpp"
#include "core/Student.hpp"

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

#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"

#include <cstdlib>
#include <algorithm>

namespace ssaas {

namespace {

const Student& currentStudent() { return DataStore::getInstance().activeStudent(); }
Student&       currentStudentMut() { return DataStore::getInstance().activeStudent(); }

nlohmann::json studentSummaryJson(const Student& s) {
    nlohmann::json j;
    j["name"]       = s.getName();
    j["email"]      = s.getEmail();
    j["rollNumber"] = s.getRollNumber();
    j["program"]    = s.getProgram();
    j["semester"]   = s.getSemester();
    j["overallAttendance"]      = s.getOverallAttendancePercentage();
    j["overallAverage"]         = s.getOverallAveragePercentage();
    j["creditWeightedAverage"]  = s.getCreditWeightedAveragePercentage();
    j["subjectCount"]           = static_cast<int>(s.getSubjects().size());
    j["upcomingExamCount"]      = static_cast<int>(s.getUpcomingExams().size());
    j["upcomingEventCount"]     = static_cast<int>(s.getUpcomingEvents().size());
    j["competitionCount"]       = static_cast<int>(s.getCompetitions().size());
    j["goalCount"]              = static_cast<int>(s.getGoals().size());
    return j;
}

nlohmann::json subjectsJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& sub : s.getSubjects()) {
        nlohmann::json item;
        item["code"]       = sub.getCode();
        item["name"]       = sub.getName();
        item["credits"]    = sub.getCredits();
        item["professor"]  = sub.getProfessor();
        item["schedule"]   = sub.getSchedule();
        item["attendance"] = {
            {"total",    sub.getTotalLectures()},
            {"attended", sub.getAttendedLectures()},
            {"percentage", sub.getAttendancePercentage()}
        };
        auto m = s.getMarksFor(sub.getCode());
        double avg = 0.0;
        if (!m.empty()) {
            for (const auto& r : m) avg += r.getPercentage();
            avg /= m.size();
        }
        item["averagePercent"]  = avg;
        item["assessmentCount"] = static_cast<int>(m.size());
        arr.push_back(item);
    }
    return arr;
}

nlohmann::json marksJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    int idx = 0;
    for (const auto& m : s.getMarksRecords()) {
        arr.push_back({
            {"id",       idx++},
            {"subject",  m.getSubjectCode()},
            {"type",     assessmentTypeName(m.getType())},
            {"title",    m.getTitle()},
            {"obtained", m.getObtained()},
            {"max",      m.getMaxMarks()},
            {"percent",  m.getPercentage()},
            {"date",     m.getDate().toIso()}
        });
    }
    return arr;
}

nlohmann::json examsJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    int idx = 0;
    for (const auto& e : s.getExams()) {
        arr.push_back({
            {"id",          idx++},
            {"subject",     e.getSubjectCode()},
            {"type",        assessmentTypeName(e.getType())},
            {"date",        e.getDate().toIso()},
            {"dateReadable",e.getDate().toReadable()},
            {"time",        e.getTime()},
            {"venue",       e.getVenue()},
            {"max",         e.getMaxMarks()},
            {"preparation", e.getPreparationProgress()},
            {"daysUntil",   e.daysUntil()}
        });
    }
    return arr;
}

nlohmann::json eventToJson(const Event& e) {
    return {
        {"id",          e.getId()},
        {"title",       e.getTitle()},
        {"type",        eventTypeName(e.getType())},
        {"date",        e.getDate().toIso()},
        {"dateReadable",e.getDate().toReadable()},
        {"time",        e.getTime()},
        {"location",    e.getLocation()},
        {"description", e.getDescription()},
        {"registered",  e.isRegistered()},
        {"daysUntil",   e.daysUntil()}
    };
}

nlohmann::json competitionToJson(const Competition& c) {
    return {
        {"id",          c.getId()},
        {"title",       c.getTitle()},
        {"organizer",   c.getOrganizer()},
        {"category",    c.getCategory()},
        {"deadline",    c.getDeadline().toIso()},
        {"deadlineReadable", c.getDeadline().toReadable()},
        {"prize",       c.getPrize()},
        {"description", c.getDescription()},
        {"status",      competitionStatusName(c.getStatus())},
        {"daysUntilDeadline", c.daysUntilDeadline()}
    };
}

nlohmann::json goalToJson(const Goal& g) {
    return {
        {"id",       g.getId()},
        {"type",     goalTypeName(g.getType())},
        {"title",    g.getTitle()},
        {"target",   g.getTarget()},
        {"current",  g.getCurrent()},
        {"progress", g.progressPercent()},
        {"deadline", g.getDeadline()},
        {"note",     g.getNote()}
    };
}

nlohmann::json eventsJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& e : s.getEvents()) arr.push_back(eventToJson(e));
    return arr;
}

nlohmann::json competitionsJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& c : s.getCompetitions()) arr.push_back(competitionToJson(c));
    return arr;
}

nlohmann::json goalsJson(const Student& s) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& g : s.getGoals()) arr.push_back(goalToJson(g));
    return arr;
}

// Extract the trailing path segment (numeric id) — e.g. "/api/events/3" -> 3.
int trailingId(const std::string& path) {
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos || slash + 1 >= path.size())
        throw HttpException(400, "Missing id in path");
    try { return std::stoi(path.substr(slash + 1)); }
    catch (...) { throw HttpException(400, "Invalid id"); }
}

std::string trailingString(const std::string& path) {
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos || slash + 1 >= path.size())
        throw HttpException(400, "Missing key in path");
    return path.substr(slash + 1);
}

nlohmann::json parseBody(const HttpRequest& req) {
    try { return nlohmann::json::parse(req.getBody()); }
    catch (...) { throw HttpException(400, "Invalid JSON body"); }
}

// Helpers to construct domain objects from JSON
Subject subjectFromJson(const nlohmann::json& body) {
    Subject s(
        body.at("code").get<std::string>(),
        body.at("name").get<std::string>(),
        body.value("credits", 3),
        body.value("professor", ""),
        body.value("schedule", "")
    );
    if (body.contains("attendance")) {
        const auto& att = body["attendance"];
        s.setAttendance(att.value("total", 0), att.value("attended", 0));
    }
    return s;
}

MarksRecord marksFromJson(const nlohmann::json& body) {
    return MarksRecord(
        body.at("subject").get<std::string>(),
        parseAssessmentType(body.at("type").get<std::string>()),
        body.value("title", ""),
        body.at("obtained").get<double>(),
        body.at("max").get<double>(),
        body.contains("date") ? Date(body["date"].get<std::string>()) : Date::today()
    );
}

Exam examFromJson(const nlohmann::json& body) {
    return Exam(
        body.at("subject").get<std::string>(),
        parseAssessmentType(body.at("type").get<std::string>()),
        Date(body.at("date").get<std::string>()),
        body.value("time", ""),
        body.value("venue", ""),
        body.value("max", 100.0),
        body.value("preparation", 0)
    );
}

Event eventFromJson(int id, const nlohmann::json& body) {
    return Event(
        id,
        body.at("title").get<std::string>(),
        parseEventType(body.value("type", "other")),
        Date(body.at("date").get<std::string>()),
        body.value("time", ""),
        body.value("location", ""),
        body.value("description", ""),
        body.value("registered", false)
    );
}

Competition competitionFromJson(int id, const nlohmann::json& body) {
    return Competition(
        id,
        body.at("title").get<std::string>(),
        body.value("organizer", ""),
        body.value("category", "Other"),
        Date(body.at("deadline").get<std::string>()),
        body.value("prize", ""),
        body.value("description", ""),
        parseCompetitionStatus(body.value("status", "open"))
    );
}

Goal goalFromJson(int id, const nlohmann::json& body) {
    return Goal(
        id,
        parseGoalType(body.value("type", "custom")),
        body.at("title").get<std::string>(),
        body.at("target").get<double>(),
        body.value("current", 0.0),
        body.value("deadline", ""),
        body.value("note", "")
    );
}

}  // anonymous

// ============================================================
// Route registration
// ============================================================
void Controllers::registerAll(Router& r) {

    // ---------- Health & meta ----------
    r.get("/api/health", [](const HttpRequest&) {
        return HttpResponse::ok({
            {"status", "ok"}, {"name", "SSAAS Backend"},
            {"version", "1.1.0"}, {"language", "C++14"}
        });
    });

    // ---------- Student ----------
    r.get("/api/student", [](const HttpRequest&) {
        return HttpResponse::ok(studentSummaryJson(currentStudent()));
    });

    r.put("/api/student", [](const HttpRequest& req) {
        auto body = parseBody(req);
        Student& s = currentStudentMut();
        try {
            if (body.contains("name"))     s.setName(body["name"].get<std::string>());
            if (body.contains("email"))    s.setEmail(body["email"].get<std::string>());
            if (body.contains("program"))  s.setProgram(body["program"].get<std::string>());
            if (body.contains("semester")) s.setSemester(body["semester"].get<int>());
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
        return HttpResponse::ok(studentSummaryJson(s));
    });

    // ---------- Subjects (full CRUD) ----------
    r.get("/api/subjects", [](const HttpRequest&) {
        return HttpResponse::ok(subjectsJson(currentStudent()));
    });

    r.post("/api/subjects", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            currentStudentMut().addSubject(subjectFromJson(body));
            return HttpResponse::ok(subjectsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/subjects/*", [](const HttpRequest& req) {
        std::string code = trailingString(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateSubject(code, subjectFromJson(body));
            return HttpResponse::ok(subjectsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/subjects/*", [](const HttpRequest& req) {
        std::string code = trailingString(req.getPath());
        try {
            currentStudentMut().removeSubject(code);
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(subjectsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Marks (full CRUD) ----------
    r.get("/api/marks", [](const HttpRequest&) {
        return HttpResponse::ok(marksJson(currentStudent()));
    });

    r.post("/api/marks", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            currentStudentMut().addMarks(marksFromJson(body));
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(marksJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        } catch (const std::exception& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/marks/*", [](const HttpRequest& req) {
        int idx = trailingId(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateMarks(idx, marksFromJson(body));
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(marksJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/marks/*", [](const HttpRequest& req) {
        int idx = trailingId(req.getPath());
        try {
            currentStudentMut().removeMarks(idx);
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(marksJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Exams (full CRUD) ----------
    r.get("/api/exams", [](const HttpRequest&) {
        return HttpResponse::ok(examsJson(currentStudent()));
    });

    r.post("/api/exams", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            currentStudentMut().addExam(examFromJson(body));
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(examsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        } catch (const std::exception& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/exams/*", [](const HttpRequest& req) {
        int idx = trailingId(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateExam(idx, examFromJson(body));
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(examsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/exams/*", [](const HttpRequest& req) {
        int idx = trailingId(req.getPath());
        try {
            currentStudentMut().removeExam(idx);
            AlertSystem::getInstance().refreshFrom(currentStudent());
            return HttpResponse::ok(examsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Attendance ----------
    r.post("/api/attendance", [](const HttpRequest& req) {
        auto body = parseBody(req);
        std::string code = body.value("subject", "");
        bool present     = body.value("present", true);
        if (code.empty()) return HttpResponse::badRequest("subject required");
        Student& s = currentStudentMut();
        try {
            s.addAttendance(AttendanceRecord(code, Date::today(), present));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
        AlertSystem::getInstance().refreshFrom(s);
        return HttpResponse::ok({
            {"ok", true},
            {"newPercentage", s.getSubject(code).getAttendancePercentage()}
        });
    });

    r.put("/api/attendance/*", [](const HttpRequest& req) {
        // Bulk patch — update total + attended counters for a subject.
        std::string code = trailingString(req.getPath());
        auto body = parseBody(req);
        try {
            Subject& s = currentStudentMut().getSubject(code);
            int total    = body.value("total",    s.getTotalLectures());
            int attended = body.value("attended", s.getAttendedLectures());
            s.setAttendance(total, attended);
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
        AlertSystem::getInstance().refreshFrom(currentStudent());
        return HttpResponse::ok(subjectsJson(currentStudent()));
    });

    r.post("/api/exam-prep", [](const HttpRequest& req) {
        auto body = parseBody(req);
        std::string code = body.value("subject", "");
        int prep         = body.value("preparation", 0);
        if (code.empty()) return HttpResponse::badRequest("subject required");
        Student& s = currentStudentMut();
        bool found = false;
        for (auto& ex : s.getExams()) {
            if (ex.getSubjectCode() == code && ex.isUpcoming()) {
                try { ex.setPreparationProgress(prep); }
                catch (const SSAASException& e) {
                    return HttpResponse::badRequest(e.what());
                }
                found = true;
            }
        }
        if (!found) return HttpResponse::notFound("Upcoming exam not found for " + code);
        AlertSystem::getInstance().refreshFrom(s);
        return HttpResponse::ok({{"ok", true}});
    });

    // ---------- Events (full CRUD) ----------
    r.get("/api/events", [](const HttpRequest&) {
        return HttpResponse::ok(eventsJson(currentStudent()));
    });

    r.post("/api/events", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            int id = currentStudentMut().nextEventId();
            currentStudentMut().addEvent(eventFromJson(id, body));
            return HttpResponse::ok(eventsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        } catch (const std::exception& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/events/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateEvent(id, eventFromJson(id, body));
            return HttpResponse::ok(eventsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/events/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        try {
            currentStudentMut().removeEvent(id);
            return HttpResponse::ok(eventsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Competitions (full CRUD) ----------
    r.get("/api/competitions", [](const HttpRequest&) {
        return HttpResponse::ok(competitionsJson(currentStudent()));
    });

    r.post("/api/competitions", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            int id = currentStudentMut().nextCompetitionId();
            currentStudentMut().addCompetition(competitionFromJson(id, body));
            return HttpResponse::ok(competitionsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        } catch (const std::exception& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/competitions/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateCompetition(id, competitionFromJson(id, body));
            return HttpResponse::ok(competitionsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/competitions/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        try {
            currentStudentMut().removeCompetition(id);
            return HttpResponse::ok(competitionsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Goals (full CRUD) ----------
    r.get("/api/goals", [](const HttpRequest&) {
        return HttpResponse::ok(goalsJson(currentStudent()));
    });

    r.post("/api/goals", [](const HttpRequest& req) {
        auto body = parseBody(req);
        try {
            int id = currentStudentMut().nextGoalId();
            currentStudentMut().addGoal(goalFromJson(id, body));
            return HttpResponse::ok(goalsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        } catch (const std::exception& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.put("/api/goals/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        auto body = parseBody(req);
        try {
            currentStudentMut().updateGoal(id, goalFromJson(id, body));
            return HttpResponse::ok(goalsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    r.del("/api/goals/*", [](const HttpRequest& req) {
        int id = trailingId(req.getPath());
        try {
            currentStudentMut().removeGoal(id);
            return HttpResponse::ok(goalsJson(currentStudent()));
        } catch (const SSAASException& e) {
            return HttpResponse::badRequest(e.what());
        }
    });

    // ---------- Alerts ----------
    r.get("/api/alerts", [](const HttpRequest&) {
        AlertSystem::getInstance().refreshFrom(currentStudent());
        return HttpResponse::ok(AlertSystem::getInstance().toJson());
    });

    // ---------- Analyzers (Strategy pattern) ----------
    auto wireAnalyzer = [&r](const std::string& path,
                             std::shared_ptr<IAnalyzer> analyzer) {
        r.get(path, [analyzer](const HttpRequest&) {
            return HttpResponse::ok(analyzer->analyze(currentStudent()));
        });
    };

    wireAnalyzer("/api/analytics/health",       std::make_shared<HealthScoreAnalyzer>());
    wireAnalyzer("/api/analytics/bunk",         std::make_shared<BunkPredictor>());
    wireAnalyzer("/api/analytics/risk",         std::make_shared<AttendanceRiskAnalyzer>());
    wireAnalyzer("/api/analytics/performance",  std::make_shared<PerformanceAnalyzer>());
    wireAnalyzer("/api/analytics/pressure",     std::make_shared<PressureAnalyzer>());
    wireAnalyzer("/api/analytics/predict",      std::make_shared<GradePredictor>());
    wireAnalyzer("/api/analytics/bayes",        std::make_shared<BayesianRiskScorer>());
    wireAnalyzer("/api/analytics/burnout",      std::make_shared<BurnoutDetector>());
    wireAnalyzer("/api/analytics/exams",        std::make_shared<ExamCountdownAnalyzer>());
    wireAnalyzer("/api/analytics/study-plan",   std::make_shared<StudyPlanAnalyzer>());

    // ---------- Combined dashboard ----------
    r.get("/api/dashboard", [](const HttpRequest&) {
        const Student& s = currentStudent();
        HealthScoreAnalyzer h; BunkPredictor b; AttendanceRiskAnalyzer risk;
        PerformanceAnalyzer perf; PressureAnalyzer press; GradePredictor gp;
        BayesianRiskScorer bayes; BurnoutDetector burnout;
        ExamCountdownAnalyzer exam; StudyPlanAnalyzer plan;
        AlertSystem::getInstance().refreshFrom(s);

        nlohmann::json out;
        out["student"]       = studentSummaryJson(s);
        out["subjects"]      = subjectsJson(s);
        out["events"]        = eventsJson(s);
        out["competitions"]  = competitionsJson(s);
        out["goals"]         = goalsJson(s);
        out["health"]        = h.analyze(s);
        out["bunk"]          = b.analyze(s);
        out["risk"]          = risk.analyze(s);
        out["performance"]   = perf.analyze(s);
        out["pressure"]      = press.analyze(s);
        out["predict"]       = gp.analyze(s);
        out["bayes"]         = bayes.analyze(s);
        out["burnout"]       = burnout.analyze(s);
        out["exams"]         = exam.analyze(s);
        out["studyPlan"]     = plan.analyze(s);
        out["alerts"]        = AlertSystem::getInstance().toJson();
        return HttpResponse::ok(out);
    });
}

}
