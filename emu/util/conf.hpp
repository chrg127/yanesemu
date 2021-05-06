#ifndef UTIL_SETTINGS_HPP_INCLUDED
#define UTIL_SETTINGS_HPP_INCLUDED

/* This is a settings library. The settings files it parses are key-value style
 * configurations, for example:
 *     key=value
 *     file=path
 *     flag=true
 * It supports three types for the values: string, bool and number (int).
 *
 * To parse a file, simply use parse_conf(). it needs a file and a structure
 * defined this way:
 *
 *      static const ValidConfValues example = {
 *          { "foo", ConfType::NUMBER, 0         },
 +          { "bar", ConfType::BOOL,   false     },
 *          { "baz", ConfType::STRING, "default" },
 *      };
 *
 * For each element, the first value is a key name, the second is the type of
 * the value, the third is a default value in case the key is missing or the
 * value is invalid. This structure is only used for validating the configuration file.
 */

#include <cstddef>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "file.hpp"

namespace Util {

using ConfValue = std::variant<bool, std::string, int>;

enum ConfType {
   ST_BOOL = 0,
   ST_STRING = 1,
   ST_NUMBER = 2,
};

struct ValidConfValue {
    std::string key;
    ConfType type;
    ConfValue def;
};
using ValidConfValues = std::vector<ValidConfValue>;
using ConfValueMap = std::unordered_map<std::string, ConfValue>;

// Helper functions for setting variant if one already knows the underlying type.
#define X(name, type) template <typename T> inline type get##name(T &v) { return std::get<type>(v); }
X(str, std::string)
X(bool, bool)
X(num, int)
#undef X

struct ConfValueError {
    std::string line;
    std::string key;
    std::size_t pos;
    enum class Kind {
        INVVAL, INVLINE,
    } kind;
};

struct ParseConfResult {
    ConfValueMap st_map;
    std::vector<ConfValueError> errors;
};

// Parse conf_file, return a setting map.
ParseConfResult parse_conf(File &conf_file, ValidConfValues valid_settings);

void print_conf_errors(const std::vector<ConfValueError> &errors);

/* Try finding the config file in the filesystem. If no file is found,
 * create a new one in a default directory for the current operating system.
 * @filename is used for the file's name. */
File open_conf_file(const std::string &dirname, const std::string &filename);

} // namespace Util

#endif
