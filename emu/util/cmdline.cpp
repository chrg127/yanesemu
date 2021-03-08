#include "cmdline.hpp"

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <cstdio>
#include <cstdlib> // exit()
#include <fmt/core.h>
#include <emu/util/debug.hpp>

namespace Util {

template <typename T>
static auto is_valid(const T arg, const ValidArgStruct &valid_args)
{
    static_assert(std::is_same<T, char>::value || std::is_same<T, std::string_view>::value, "T must be std::string_view, const char * or char");
    if constexpr(std::is_same<T, char>::value) {
        return std::find_if(valid_args.begin(), valid_args.end(),
                [arg](const Argument &valid_arg) { return valid_arg.short_opt == arg; });
    } else if constexpr(std::is_same<T, std::string_view>::value) {
        return std::find_if(valid_args.begin(), valid_args.end(),
                [arg](const Argument &valid_arg) { return valid_arg.long_opt == arg; });
    }
}

ArgResult parse(int argc, char *argv[], const ValidArgStruct &valid_args)
{
    ArgResult res;

    for (const auto &arg : valid_args) {
        res.has[arg.short_opt] = false;
        res.params[arg.short_opt] = "";
    }
    while (++argv, --argc > 0) {
        std::string_view currarg = argv[0];
        const char *nextarg = argv[1];
        // is currarg a real arg?
        if (currarg[0] != '-') {
            res.items.push_back(currarg);
            continue;
        }
        // is currarg a valid argument according to valid_args?
        auto argp = currarg[1] != '-' && currarg.size() == 2 ?
                        is_valid(currarg[1], valid_args)     :
                        is_valid(*(currarg.begin() + 2), valid_args);
        if (argp == valid_args.end()) {
            warning("%s: not a valid argument\n", currarg.data());
            continue;
        }
        auto arg = *argp;
        if (arg.paramt == ParamType::MUST_HAVE && (!nextarg || nextarg[0] == '-')) {
            warning("%s must have a parameter\n", currarg.data());
            continue;
        } else if (res.has[arg.short_opt]) {
            warning("%s specified multiple times\n", currarg.data());
            continue;
        }
        res.has[arg.short_opt] = true;
        // check for a parameter and validate and collect it
        if (arg.paramt != ParamType::NONE && nextarg && nextarg[0] != '-') {
            if (!arg.validator(nextarg)) {
                warning("%s: %s is not a valid parameter\n", currarg.data(), nextarg);
                continue;
            }
            res.params[arg.short_opt] = std::string_view(nextarg);
            // and advance to the next argument
            argv++;
            argc--;
        }
    }
    return res;
}

bool default_validator(std::string_view)
{
    return true;
}

void print_usage(std::string_view progname, const ValidArgStruct &args)
{
    fmt::print("Usage: {} [args...]\nValid arguments:\n", progname);
    const auto max = std::max_element(args.begin(), args.end(),
            [](const Argument &arg, const Argument &arg2) { return arg.long_opt.size() < arg2.long_opt.size(); });
    const std::size_t padding = max->long_opt.size() + 4;
    for (const auto &arg : args) {
        fmt::print("    -{}, --{}", arg.short_opt, arg.long_opt);
        for (std::size_t i = 0; i < padding - arg.long_opt.size(); i++)
            fmt::print(" ");
        fmt::print("{}\n", arg.desc);
    }
}

void print_version(std::string_view progname, std::string_view version)
{
    fmt::print("{}: version {}\n", progname, version);
}

} // namespace Util

