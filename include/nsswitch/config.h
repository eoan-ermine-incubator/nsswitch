#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace nsswitch {

struct Criterion {
    bool inversion{false};

    enum class Status { SUCCESS, NOTFOUND, UNAVAIL, TRYAGAIN } status;

    enum class Action { RETURN, CONTINUE, MERGE } action;
};

struct Service {
    std::string name;
    std::vector<Criterion> criterions;
};

struct Entry {
    std::string database;
    std::vector<Service> services;
};

struct Config {
    std::vector<Entry> entries;
};

std::string to_string(Criterion::Status status) {
    switch (status) {
    case Criterion::Status::SUCCESS:
        return "SUCCESS";
    case Criterion::Status::NOTFOUND:
        return "NOTFOUND";
    case Criterion::Status::UNAVAIL:
        return "UNAVAIL";
    case Criterion::Status::TRYAGAIN:
        return "TRYAGAIN";
    }
    return "unknown";
}

std::string to_string(Criterion::Action action) {
    switch (action) {
    case Criterion::Action::RETURN:
        return "RETURN";
    case Criterion::Action::CONTINUE:
        return "CONTINUE";
    case Criterion::Action::MERGE:
        return "MERGE";
    }
    return "unknown";
}

}  // namespace nsswitch
