#ifndef UTIL_CMDLINE_HPP_INCLUDED
#define UTIL_CMDLINE_HPP_INCLUDED

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Util {

using ParamValidator = bool (*)(std::string_view);
enum class ParamType { NONE, OPTIONAL, MUST_HAVE };

inline bool default_validator(std::string_view) { return true; }

/* Represents a command line argument. They come in two forms:
 * a short version starting with - and a long version starting with --.
 * An argument can take a (single) parameter. Whether it does is described
 * by ptype, and if parameter, if any, is validated by validator. */
struct Argument {
    char short_opt;
    std::string_view long_opt;
    std::string_view desc;
    ParamType ptype = ParamType::NONE;
    ParamValidator validator = default_validator;
};

/* A type that contains all defined arguments. It is read-only
 * by parse() and company. One should define a ValidArgStruct at
 * the start of the program like so:
 *  static const ValidArgStruct args = {
 *      { ...first arg... }
 *      { ... other args ... }
 *  }; */
using ValidArgStruct = std::vector<Argument>;

/* Result of parse(). Use the short option to search for arguments.
 * has[] contains what arguments were found. params[] contains
 * the parameters for each argument. Items are any string found
 * that weren't arguments, ordered by when they were found. */
struct ArgResult {
    std::unordered_map<char, bool>             has;
    std::unordered_map<char, std::string_view> params;
    std::vector<std::string_view>              items;
    bool item_error = false;
};

ArgResult argparse(const std::vector<std::string_view> &args, const std::vector<Argument> &valid_args);

inline ArgResult argparse(int argc, char **argv, const std::vector<Argument> &valid_args)
{
    return argparse(std::vector<std::string_view>(argv, argv + argc), valid_args);
}

void print_args(const std::vector<Argument> &valid_args);

} // namespace Util

#endif
