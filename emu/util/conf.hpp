#pragma once

#include <cstdio>
#include <string>
#include <string_view>
#include <map>
#include <variant>
#include <optional>
#include <functional>

namespace conf {

enum class Type { Int, Float, Bool, String };

struct Value {
    std::variant<int, float, bool, std::string> value;

    Value() = default;
    explicit Value(bool v)               : value(v) {}
    explicit Value(int v)                : value(v) {}
    explicit Value(float v)              : value(v) {}
    explicit Value(const std::string &v) : value(v) {}
    explicit Value(const char *v)        : value(std::string(v)) {}
    explicit Value(char *v)              : value(std::string(v)) {}

    template <typename T> T as() const { return std::get<T>(value); }
    Type type() const { return static_cast<Type>(value.index()); }

    std::string to_string() const
    {
        switch (value.index()) {
        case 0: return std::to_string(as<int>());
        case 1: return std::to_string(as<float>()) + "f";
        case 2: return as<bool>() ? "true" : "false";
        case 3: return "\"" + as<std::string>() + "\"";
        default: return "";
        }
    }
};

using Data        = std::map<std::string, Value>;
using ValidConfig = std::map<std::string, Value>;

std::optional<Data> parse(std::string_view text, const ValidConfig &valid,
                          std::function<void(std::string_view)> display_error);
std::optional<Data> parse_or_create(std::string_view path, const ValidConfig &valid,
                                    std::function<void(std::string_view)> display_error);
void create(std::string_view pathname, const Data &conf, std::function<void(std::string_view)> display_error);

} // namespace conf
