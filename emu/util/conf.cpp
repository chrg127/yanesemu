#include "conf.hpp"

#include <optional>
#include <fmt/core.h>
#include <emu/util/utility.hpp>
#include <emu/util/file.hpp>
#include <emu/util/string.hpp>

namespace conf {

static std::vector<Error> collected_errors;

static void add_error(int line, std::string &&msg)
{
    collected_errors.push_back({ .line = line, .msg = std::move(msg) });
}

static std::optional<Value> check(const std::string &value, Type type)
{
    switch (type) {
    case Type::Int:
        if (auto r = str::conv(value); r)
            return r.value();
        return std::nullopt;
    case Type::Bool:
        if (value == "true")  return true;
        if (value == "false") return false;
        return std::nullopt;
    case Type::String:
        if (value == "true")  return std::nullopt;
        if (value == "false") return std::nullopt;
        return Value(value);
    }
    return std::nullopt;
}

static Configuration parse_file(std::string_view pathname, const ValidConf &valid)
{
    std::optional<io::File> file = io::File::open(pathname, io::Access::READ);
    if (!file)
        return {};

    Configuration conf;
    int linenum = 1;
    for (std::string line; file->getline(line); linenum++) {
        // skip comments
        if (line == "" || line[0] == '#')
            continue;

        auto keyval = str::split(line, '=');
        if (keyval.size() != 2) {
            add_error(linenum, fmt::format("invalid line: {}", line));
            continue;
        }

        auto key   = str::trim(keyval[0]);
        auto value = str::trim(keyval[1]);
        auto entry = util::map_lookup(valid, key);
        if (!entry) {
            add_error(linenum, fmt::format("invalid key: {}", key));
            continue;
        }

        if (auto cv = check(value, entry->type); cv)
            conf[key] = cv.value();
        else
            add_error(linenum, fmt::format("invalid value '{}' for key '{}' (default will be used)", value, key));
    }
    return conf;
}

static Configuration add_missing(Configuration &&conf, const ValidConf &valid)
{
    for (const auto &entry : valid) {
        if (auto r = util::map_lookup(conf, entry.first); !r) {
            add_error(-1, fmt::format("missing key: {}", entry.first));
            conf[entry.first] = entry.second.default_value;
        }
    }
    return conf;
}

Configuration parse(std::string_view pathname, const ValidConf &valid)
{
    return add_missing(parse_file(pathname, valid), valid);
}

void create(std::string_view pathname, const Configuration &conf, const std::unordered_map<std::string, std::string> &comments)
{
    auto file = io::File::open(pathname, io::Access::WRITE);
    if (!file) {
        fmt::print(stderr, "error: couldn't create file {}\n", pathname);
        return;
    }

    auto max = std::max_element(conf.begin(), conf.end(), [](const auto &e1, const auto &e2) {
        return e1.first.size() < e2.first.size();
    });
    std::size_t width = max->first.size();

    for (auto &entry : conf) {
        if (auto c = util::map_lookup(comments, entry.first); c) {
            auto lines = str::split_lines(c.value(), 80);
            for (auto &line : lines)
                fmt::print(file->data(), "# {}\n", line);
        }
        fmt::print(file->data(), "{:{}} = {}\n", entry.first, width, entry.second.to_str());
    }
}

std::vector<Error> errors()
{
    auto tmp = collected_errors;
    collected_errors.clear();
    return tmp;
}

} // namespace Util
