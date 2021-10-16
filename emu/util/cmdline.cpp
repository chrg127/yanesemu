#include "cmdline.hpp"

#include <iterator>
#include <type_traits>
#include <fmt/core.h>
#include <emu/util/debug.hpp>

namespace cmdline {

/* Returns an iterator inside of valid_args */
template <typename T>
static auto find_arg(const T arg, const std::vector<Argument> &args)
{
    static_assert(std::is_same<T,             char>::value
               || std::is_same<T, std::string_view>::value,
                  "T must be std::string_view, const char * or char");
    if constexpr(std::is_same<T, char>::value) {
        return std::find_if(args.begin(), args.end(), [arg](const auto &a) { return a.short_opt == arg; });
    } else if constexpr(std::is_same<T, std::string_view>::value) {
        return std::find_if(args.begin(), args.end(), [arg](const auto &a) { return a.long_opt == arg; });
    }
}

Result argparse(const std::vector<std::string_view> &args, const std::vector<Argument> &valid_args)
{
    Result res;

    for (const auto &arg : valid_args) {
        res.has[arg.short_opt] = false;
        res.params[arg.short_opt] = "";
    }

    for (auto it = args.begin()+1, it2 = args.begin()+2; it != args.end(); ++it, ++it2) {
        std::string_view curr = *it;
        std::string_view next = it2 >= args.end() ? "" : *it2;

        // is curr an arg or an item?
        if (curr[0] != '-') {
            res.items.push_back(curr);
            continue;
        }

        // find curr in valid_args
        auto argp = curr[1] != '-' && curr.size() == 2
                  ? find_arg(curr[1], valid_args)
                  : find_arg(curr.substr(2), valid_args);
        if (argp == valid_args.end()) {
            warning("invalid argument: {}\n", curr);
            continue;
        }

        auto arg = *argp;
        if (arg.ptype == ParamType::MustHave && (next == "" || next[0] == '-')) {
            warning("argument {} must have a parameter\n", curr);
            continue;
        }
        if (res.has[arg.short_opt]) {
            warning("argument {} was specified multiple times\n", curr);
            continue;
        }
        res.has[arg.short_opt] = true;

        // argument parameter handling
        if (arg.ptype != ParamType::None && next != "" && next[0] != '-') {
            if (!arg.validator(next)) {
                warning("invalid parameter {} for argument {}\n", next, curr);
                continue;
            }
            res.params[arg.short_opt] = next;
        }
    }

    return res;
}

void print_args(const std::vector<Argument> &args)
{
    const auto maxwidth = std::max_element(args.begin(), args.end(), [](const auto &p, const auto &q) {
        return p.long_opt.size() < q.long_opt.size();
    })->long_opt.size();

    fmt::print("Valid arguments:\n");
    for (const auto &arg : args) {
        fmt::print("    -{}, --{:{}}    {}\n", arg.short_opt, arg.long_opt, maxwidth, arg.desc);
    }
}

} // namespace Util

