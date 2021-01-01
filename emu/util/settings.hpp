#ifndef UTIL_SETTINGS_HPP_INCLUDED
#define UTIL_SETTINGS_HPP_INCLUDED

/* this is a settings library. the settings files it parses are key-value style
 * configurations, for example:
 *     key=value
 *     file=path
 *     flag=true
 * it supports three types for the values: string, bool and number (int).
 *
 * to parse a file, simply use the parse_conf. it needs a file and a structure
 * defined this way:
 *
 *      static const ValidSettings example = {
 *          { "foo", SETTINGTYPE_NUMBER, 0         },
 +          { "bar", SETTINGTYPE_BOOL,   false     },
 *          { "baz", SETTINGTYPE_STRING, "default" },
 *      };
 * 
 * for each element, the first value is a key name, the second is the type of
 * the value, the third is a default value in case the key is missing or the
 * value is invalid.
 * this structure is only used for validating the configuration file.
 */

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "file.hpp"

namespace Util {

using Setting = std::variant<bool, std::string, int>;

/* this enum is only for verifying the setting. they should be in sync
 * with the variant type. */
enum SettingType {
   SETTINGTYPE_BOOL = 0,
   SETTINGTYPE_STRING = 1,
   SETTINGTYPE_NUMBER = 2
};

/* for building the valid settings array. */
struct ValidSetting {
    std::string key;
    SettingType type;
    Setting     def;
};
using ValidSettings = std::vector<ValidSetting>;

using SettingMap = std::unordered_map<std::string, Setting>;
SettingMap parse_conf(File &f, ValidSettings valid_settings);

/* std::get<T> is verbose, these 3 functions (getstr, getbool, getnum) cut down
 * on the verbosity. use them like so:
 *     getstr(setting_map["somekey"]) == "str_to_compare";
 *     if (getbool(setting_map["someboolkey"])) printf("true!");
 *     x += getnum(setting_map["anotherkey"]);
 */
#define X(name, type) template <typename T> inline type get##name(T &v) { return std::get<type>(v); }
X(str, std::string)
X(bool, bool)
X(num, int)
#undef X

} // namespace Util

#endif
