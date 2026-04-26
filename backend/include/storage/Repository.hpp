#ifndef SSAAS_STORAGE_REPOSITORY_HPP
#define SSAAS_STORAGE_REPOSITORY_HPP

#include "utils/Exceptions.hpp"

#include <vector>
#include <functional>
#include <algorithm>
#include <string>

namespace ssaas {

// Template-based generic repository.
// T must expose a string identifier reachable via the KeyExtractor functor.
//
// Usage:
//   Repository<Subject> subjects([](const Subject& s){ return s.getCode(); });
template <typename T>
class Repository {
public:
    using KeyExtractor = std::function<std::string(const T&)>;

    explicit Repository(KeyExtractor key) : keyOf(std::move(key)) {}

    void add(const T& item) {
        if (exists(keyOf(item)))
            throw ValidationException("Duplicate key in repository: " + keyOf(item));
        items.push_back(item);
    }

    bool exists(const std::string& id) const {
        return std::any_of(items.begin(), items.end(),
            [&](const T& it) { return keyOf(it) == id; });
    }

    T& get(const std::string& id) {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const T& x) { return keyOf(x) == id; });
        if (it == items.end()) throw NotFoundException("Repo missing key: " + id);
        return *it;
    }

    const T& get(const std::string& id) const {
        auto it = std::find_if(items.begin(), items.end(),
            [&](const T& x) { return keyOf(x) == id; });
        if (it == items.end()) throw NotFoundException("Repo missing key: " + id);
        return *it;
    }

    void remove(const std::string& id) {
        auto it = std::remove_if(items.begin(), items.end(),
            [&](const T& x) { return keyOf(x) == id; });
        if (it == items.end()) throw NotFoundException("Cannot remove missing: " + id);
        items.erase(it, items.end());
    }

    size_t size() const { return items.size(); }

    const std::vector<T>& all() const { return items; }
    std::vector<T>&       all()       { return items; }

private:
    std::vector<T>  items;
    KeyExtractor    keyOf;
};

}

#endif
