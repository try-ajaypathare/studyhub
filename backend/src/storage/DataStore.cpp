#include "storage/DataStore.hpp"
#include "utils/Exceptions.hpp"
#include "utils/Logger.hpp"
#include "third_party/json.hpp"

#include <fstream>
#include <sstream>

namespace ssaas {

DataStore& DataStore::getInstance() {
    static DataStore instance;
    return instance;
}

DataStore::DataStore()
    : studentRepo(new Repository<Student>(
        [](const Student& s) { return s.getRollNumber(); })) {}

namespace {

Subject parseSubject(const nlohmann::json& j) {
    Subject s(
        j.at("code").get<std::string>(),
        j.at("name").get<std::string>(),
        j.at("credits").get<int>(),
        j.value("professor", ""),
        j.value("schedule",  "")
    );
    if (j.contains("attendance")) {
        const auto& att = j["attendance"];
        s.setAttendance(att.value("total", 0), att.value("attended", 0));
    }
    return s;
}

MarksRecord parseMark(const std::string& subject, const nlohmann::json& j) {
    return MarksRecord(
        subject,
        parseAssessmentType(j.at("type").get<std::string>()),
        j.value("title", ""),
        j.at("obtained").get<double>(),
        j.at("max").get<double>(),
        Date(j.at("date").get<std::string>())
    );
}

Exam parseExam(const std::string& subject, const nlohmann::json& j) {
    return Exam(
        subject,
        parseAssessmentType(j.at("type").get<std::string>()),
        Date(j.at("date").get<std::string>()),
        j.value("time",  ""),
        j.value("venue", ""),
        j.value("max", 100.0),
        j.value("preparation", 0)
    );
}

Event parseEvent(int id, const nlohmann::json& j) {
    return Event(
        id,
        j.at("title").get<std::string>(),
        parseEventType(j.value("type", "other")),
        Date(j.at("date").get<std::string>()),
        j.value("time", ""),
        j.value("location", ""),
        j.value("description", ""),
        j.value("registered", false)
    );
}

Competition parseCompetition(int id, const nlohmann::json& j) {
    return Competition(
        id,
        j.at("title").get<std::string>(),
        j.value("organizer", ""),
        j.value("category", "Other"),
        Date(j.at("deadline").get<std::string>()),
        j.value("prize", ""),
        j.value("description", ""),
        parseCompetitionStatus(j.value("status", "open"))
    );
}

Goal parseGoal(int id, const nlohmann::json& j) {
    return Goal(
        id,
        parseGoalType(j.value("type", "custom")),
        j.at("title").get<std::string>(),
        j.at("target").get<double>(),
        j.value("current", 0.0),
        j.value("deadline", ""),
        j.value("note", "")
    );
}

}

void DataStore::bootstrap(const std::string& path) {
    Logger::getInstance().info("DataStore: loading seed from " + path);
    std::ifstream in(path);
    if (!in) throw StorageException("Could not open seed file: " + path);
    std::stringstream ss;
    ss << in.rdbuf();

    nlohmann::json root;
    try { root = nlohmann::json::parse(ss.str()); }
    catch (const std::exception& e) {
        throw StorageException(std::string("Seed parse error: ") + e.what());
    }

    for (const auto& sj : root.at("students")) {
        Student stu(
            sj.at("name").get<std::string>(),
            sj.at("email").get<std::string>(),
            sj.at("rollNumber").get<std::string>(),
            sj.at("program").get<std::string>(),
            sj.at("semester").get<int>()
        );
        for (const auto& sub : sj.value("subjects", nlohmann::json::array()))
            stu.addSubject(parseSubject(sub));
        for (const auto& mk : sj.value("marks", nlohmann::json::array()))
            stu.addMarks(parseMark(mk.at("subject").get<std::string>(), mk));
        for (const auto& ex : sj.value("exams", nlohmann::json::array()))
            stu.addExam(parseExam(ex.at("subject").get<std::string>(), ex));
        for (const auto& ev : sj.value("events", nlohmann::json::array()))
            stu.addEvent(parseEvent(stu.nextEventId(), ev));
        for (const auto& cp : sj.value("competitions", nlohmann::json::array()))
            stu.addCompetition(parseCompetition(stu.nextCompetitionId(), cp));
        for (const auto& g  : sj.value("goals", nlohmann::json::array()))
            stu.addGoal(parseGoal(stu.nextGoalId(), g));
        studentRepo->add(stu);
    }

    if (!studentRepo->all().empty())
        activeRollNumber = studentRepo->all().front().getRollNumber();

    Logger::getInstance().info("DataStore: " +
        std::to_string(studentRepo->size()) + " student(s) loaded; active = " +
        activeRollNumber);
}

Student& DataStore::activeStudent() {
    if (activeRollNumber.empty())
        throw StorageException("No active student set");
    return studentRepo->get(activeRollNumber);
}

const Student& DataStore::activeStudent() const {
    if (activeRollNumber.empty())
        throw StorageException("No active student set");
    return studentRepo->get(activeRollNumber);
}

void DataStore::setActiveStudent(const std::string& roll) {
    if (!studentRepo->exists(roll))
        throw NotFoundException("Student not in store: " + roll);
    activeRollNumber = roll;
}

void DataStore::persist(const std::string& path) const {
    nlohmann::json root;
    root["students"] = nlohmann::json::array();
    for (const auto& s : studentRepo->all()) {
        nlohmann::json sj;
        sj["name"]       = s.getName();
        sj["email"]      = s.getEmail();
        sj["rollNumber"] = s.getRollNumber();
        sj["program"]    = s.getProgram();
        sj["semester"]   = s.getSemester();
        // ... persistence is best-effort; full serialisation skipped here.
        root["students"].push_back(sj);
    }
    std::ofstream out(path);
    if (!out) throw StorageException("Cannot write persist file: " + path);
    out << root.dump(2);
}

}
