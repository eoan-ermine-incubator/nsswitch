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

}  // namespace nsswitch
