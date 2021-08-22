#pragma once

#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <variant>
#include <vector>

namespace conf {

enum class Type {
    INT,
    BOOL,
    STRING
};

struct Value {
    std::variant<int, bool, std::string> value;

    Value() = default;
    Value(bool v) : value(v) {}
    Value(int v) : value(v) {}
    Value(const std::string &v) : value(v) {}
    Value(const char *v) : value(std::string(v)) {}
    Value(char *v) : value(std::string(v)) {}

    template <typename T>
    T as() const { return std::get<T>(value); }

    std::string to_str() const
    {
        switch (value.index()) {
        case 0: return std::to_string(as<int>());
        case 1: return as<bool>() ? "true" : "false";
        case 2: return as<std::string>();
        default: return "";
        }
    }
};

using Configuration = std::map<std::string, Value>;

struct ValidVal {
    Type type;
    Value default_value;
};
using ValidConf = std::map<std::string, ValidVal>;

struct Error {
    int line;
    std::string msg;
};

Configuration parse(std::string_view pathname, const ValidConf &valid);
void create(
    std::string_view pathname,
    const Configuration &conf,
    const std::unordered_map<std::string, std::string> comments = {}
);
std::vector<Error> errors();

} // namespace Util
