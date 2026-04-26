#ifndef SSAAS_STORAGE_DATA_STORE_HPP
#define SSAAS_STORAGE_DATA_STORE_HPP

#include "core/Student.hpp"
#include "storage/Repository.hpp"

#include <memory>
#include <string>

namespace ssaas {

// Singleton aggregate root for the running app. Holds a Repository<Student>
// and exposes the "active student" for the current frontend session.
//
// Sample/seed data lives in data/sample_data.json and is loaded on bootstrap().
class DataStore {
public:
    static DataStore& getInstance();
    DataStore(const DataStore&) = delete;
    DataStore& operator=(const DataStore&) = delete;

    // Load seed data and set the active student. Called once at startup.
    void bootstrap(const std::string& sampleJsonPath);

    Repository<Student>&       students()       { return *studentRepo; }
    const Repository<Student>& students() const { return *studentRepo; }

    Student&       activeStudent();
    const Student& activeStudent() const;

    void setActiveStudent(const std::string& rollNumber);

    // Persist the active student snapshot back to JSON (single-file, simple).
    void persist(const std::string& path) const;

private:
    DataStore();

    std::unique_ptr<Repository<Student>> studentRepo;
    std::string                          activeRollNumber;
};

}

#endif
