/* This is a library for command line arguments. How to use:
 * First, define a ValidArgStruct like so:
 *
 *      const ValidArgStruct valid_arguments = {
 *          { 'b', "break" }, // default: ParamType::NONE, default_validator
 *          { 'f', "log-file", ParamType::MUST_HAVE },
 *          { 'C', "files", ParamType::OPTIONAL, some_custom_validator }
 *      };
 *
 * This describes all valid arguments. First are both the short
 * and long version of the argument, then what kind of parameters accepts.
 * NONE is the default.
 * You can also have custom parameter validators. To use one, declare it
 * as:
 *
 *      bool some_custom_validator(std::string_view param);
 *
 * and pass it to the valid_arguments struct.
 *
 * To actually parse the arguments, you just need to call parse:
 *
 *      auto result = parse(argc, argv, valid_arguments);
 *
 * result.found will list all arguments found; index into it using the
 * short version of the argument.
 * result.params lists the parameters found; use it like result.found.
 * result.items lists any argument without the usual dash (-).
 *
 * None that this library has the fixed behaviour of only accepting arguments
 * starting with - or -- as real arguments; for any other behaviour, use
 * something else.
 */

#ifndef CMDLINE_HPP_INCLUDED
#define CMDLINE_HPP_INCLUDED

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Util {

/* An Argument is a command line option starting with a dash (-).
 * It has always two versions: a short version that uses one dash and
 * one character and a longer version with two dashes.
 * An Argument can take a Parameter. Whether it takes a parameter is specified
 * by the ParamType field.
 * Parameters can also be validated at parsing. This is done by passing a
 * Validator function, of the prototype bool validator(std::string_view).
 * The Validator will be called during parsing.
 * Finally, an Item is any option that is not an Argument or a Parameter.
 * Items are collected automatically by parsing.
 *
 * When the parsing is done, it returns an ArgResult. The ArgResult struct
 * holds Arguments, Parameters and Items. To search for specific properties
 * of Arguments, 
 */

using ParamValidator = bool (*)(std::string_view);
enum class ParamType { NONE, OPTIONAL, MUST_HAVE };

bool default_validator(std::string_view);

struct Argument {
    char short_opt;
    std::string_view long_opt;
    std::string_view desc;
    ParamType paramt = ParamType::NONE;
    ParamValidator validator = default_validator;
};
using ValidArgStruct = std::vector<Argument>;

struct ArgResult {
    std::unordered_map<char, bool>             found;
    std::unordered_map<char, std::string_view> params;
    std::vector<std::string_view>               items;
};

ArgResult parse(int argc, char *argv[], const ValidArgStruct &valid_args);
void print_usage(std::string_view progname, const ValidArgStruct &valid_args);
void print_version(std::string_view progname, std::string_view version);

} // namespace Util

#endif

