#ifndef UTIL_CMDLINE_HPP_INCLUDED
#define UTIL_CMDLINE_HPP_INCLUDED

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <functional>
#include <optional>

namespace Util {

using ParamValidator = bool (*)(std::string_view);
enum class ParamType { NONE, OPTIONAL, MUST_HAVE };

bool default_validator(std::string_view);

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
};

enum class CmdParseError {
    INVALID_ARG,
    MULTIPLE_ARG,
    NO_PARAM,
    INVALID_PARAM,
    NO_ITEMS,
    NUM_ITEMS,
};
using ParseErrorFn = std::function<void(CmdParseError, std::string_view, std::string_view)>;

inline void def_parse_error_fn(CmdParseError, std::string_view, std::string_view) { }

std::optional<ArgResult> argparse(std::vector<std::string_view> args,
                const std::vector<Argument> &valid_args,
                ParseErrorFn errorfn = def_parse_error_fn,
                int num_items = 0);

inline std::optional<ArgResult> argparse(int argc, char **argv,
                       const std::vector<Argument> &valid_args,
                       ParseErrorFn errorfn = def_parse_error_fn,
                       int num_items = -1)
{
    return argparse(std::vector<std::string_view>(argv, argv + argc), valid_args, errorfn, num_items);
}

void print_usage(std::string_view progname, const std::vector<Argument> &valid_args);
void print_version(std::string_view progname, std::string_view version);

} // namespace Util

#endif
