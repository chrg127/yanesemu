#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace cmdline {

using ParamValidator = bool (*)(std::string_view);
enum class ParamType { NONE, OPTIONAL, MUST_HAVE };

inline bool default_validator(std::string_view) { return true; }

/*
 * Represents a command line argument. They come in two forms:
 * a short version starting with - and a long version starting with --.
 * An argument can take a (single) parameter: whether it does is described
 * by ptype. The eventual parameter is validated by validator.
 */
struct Argument {
    char short_opt;
    std::string_view long_opt;
    std::string_view desc;
    ParamType ptype = ParamType::NONE;
    ParamValidator validator = default_validator;
};

/*
 * Result of argparse(). Use the short option to search for arguments.
 * has[] contains what arguments were found. params[] contains
 * the parameters for each argument. Items are any string found
 * that weren't arguments, ordered by when they were found.
 */
struct Result {
    std::unordered_map<char, bool>             has;
    std::unordered_map<char, std::string_view> params;
    std::vector<std::string_view>              items;
    bool item_error = false;
};

/*
 * To correctly use argparse(), one should define a structure like so:
 * std::vector<Argument> arguments = {
 *     { 'f', "firstarg", "my first argument" },
 *     { 'h', "help", "help argument" },
 *     // ...
 * };
 */
Result argparse(const std::vector<std::string_view> &args, const std::vector<Argument> &valid_args);

inline Result argparse(int argc, char **argv, const std::vector<Argument> &valid_args)
{
    return argparse(std::vector<std::string_view>(argv, argv + argc), valid_args);
}

void print_args(const std::vector<Argument> &valid_args);

} // namespace Util
