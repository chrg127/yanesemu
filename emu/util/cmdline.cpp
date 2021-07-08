#include "cmdline.hpp"

#include <iterator>
#include <type_traits>
#include <fmt/core.h>
#include <emu/util/debug.hpp>

namespace Util {

/* Returns an iterator inside of valid_args */
template <typename T>
static auto find_arg(const T arg, const ValidArgStruct &valid_args)
{
    static_assert(std::is_same<T, char>::value || std::is_same<T, std::string_view>::value,
            "T must be std::string_view, const char * or char");
    if constexpr(std::is_same<T, char>::value) {
        return std::find_if(valid_args.begin(), valid_args.end(),
                [arg](const Argument &valid_arg) { return valid_arg.short_opt == arg; });
    } else if constexpr(std::is_same<T, std::string_view>::value) {
        return std::find_if(valid_args.begin(), valid_args.end(),
                [arg](const Argument &valid_arg) { return valid_arg.long_opt == arg; });
    }
}

bool default_validator(std::string_view)
{
    return true;
}

std::optional<ArgResult> argparse(std::vector<std::string_view> args,
                const std::vector<Argument> &valid_args,
                ParseErrorFn errorfn,
                int num_items)
{
    ArgResult res;

    for (const auto &arg : valid_args) {
        res.has[arg.short_opt] = false;
        res.params[arg.short_opt] = "";
    }

    for (auto it = args.begin()+1, it2 = args.begin()+2; it != args.end(); ++it, ++it2) {
        std::string_view curr = *it;
        std::string_view next = it2 >= args.end() ? "" : *it2;

        // is currarg a real arg?
        if (curr[0] != '-') {
            res.items.push_back(curr);
            continue;
        }

        // is currarg a valid argument according to valid_args?
        auto argp = curr[1] != '-' && curr.size() == 2
                  ? find_arg(curr[1], valid_args)
                  : find_arg(*(curr.begin() + 2), valid_args);
        if (argp == valid_args.end()) {
            errorfn(CmdParseError::INVALID_ARG, curr, "");
            continue;
        }
        auto arg = *argp;

        if (arg.ptype == ParamType::MUST_HAVE && (next == "" || next[0] == '-')) {
            errorfn(CmdParseError::NO_PARAM, curr, next);
            // warning("{} must have a parameter\n", currarg.data());
            continue;
        }
        if (res.has[arg.short_opt]) {
            errorfn(CmdParseError::MULTIPLE_ARG, curr, next);
            // warning("{} specified multiple times\n", currarg.data());
            continue;
        }
        res.has[arg.short_opt] = true;

        // argument parameter handling
        if (arg.ptype != ParamType::NONE && next != "" && next[0] != '-') {
            if (!arg.validator(next)) {
                errorfn(CmdParseError::INVALID_PARAM, curr, next);
                // warning("{}: {} is not a valid parameter\n", currarg.data(), nextarg);
                continue;
            }
            res.params[arg.short_opt] = next;
        }
    }

    if (num_items != -1 && res.items.size() < num_items) {
        errorfn(CmdParseError::NO_ITEMS, "", "");
        return std::nullopt;
    } else if (num_items != -1 && res.items.size() != num_items)
        errorfn(CmdParseError::NUM_ITEMS, "", "");

    return res;
}

void print_usage(std::string_view progname, const ValidArgStruct &args)
{
    const auto max = std::max_element(args.begin(), args.end(), [](const auto &p, const auto &q) {
        return p.long_opt.size() < q.long_opt.size();
    });

    fmt::print("Usage: {} [args...]\nValid arguments:\n", progname);
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

