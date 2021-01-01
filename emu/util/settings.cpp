#include "settings.hpp"

#include "stringops.hpp"
#define DEBUG
#include "debug.hpp"

namespace Util {

static bool is_int(const std::string &s)
{
    if (s.empty() || !isdigit(s[0]) && s[0] != '+' && s[0] != '-')
        return false;
    // use strtol to avoid exceptions for such a simple thing
    char *endptr;
    std::strtol(s.c_str(), &endptr, 0);
    return (*endptr == 0);
}

/* string to variant */
static Setting parse_value(const std::string &s)
{
    // return (s == "true")  ? true         :
    //        (s == "false") ? false        :
    //        (is_int(s))    ? std::stoi(s) :
    //                         s;
    if (s == "true") return true;
    else if (s == "false") return false;
    else if (is_int(s)) return std::stoi(s);
    else return s;
}

/* parse a file into a setting data structure, ignoring errors */
SettingMap parse_conf(File &f, ValidSettings valid_settings)
{
    SettingMap smap;
    std::unordered_map<std::string, std::string> kvmap;
    for (std::string line; f.getline(line); ) {
        if (line[0] == '#' || line == "")
            continue;
        auto vs = strsplit(line, '=');
        if (vs.size() != 2) {
            warning("not a key value pair: %s\n", line.c_str());
            continue;
        }
        kvmap[vs[0]] = vs[1];
    }
    for (const auto &setting : valid_settings) {
        auto it = kvmap.find(setting.key);
        if (it == kvmap.end()) {
            warning("key %s not found, using default\n", setting.key.c_str());
            smap[setting.key] = setting.def;
            continue;
        }
        Setting value = parse_value(it->second);
        if (value.index() != setting.type) {
            warning("invalid type for key %s, using default\n", setting.key.c_str());
            smap[setting.key] = setting.def;
            continue;
        }
        smap[setting.key] = value;
    }
    return smap;
}

} // namespace Util

