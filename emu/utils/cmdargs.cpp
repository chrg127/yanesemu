#include <emu/utils/cmdargs.hpp>

#include <cmath>
#include <utility>
#include <emu/utils/debug.hpp>

namespace Utils {

ArgFlags::ArgFlags(ArgFlags &&f)
{
    std::swap(bits, f.bits);
    std::swap(item, f.item);
    choices = f.choices;
    f.choices = nullptr;
}

ArgFlags &ArgFlags::operator=(ArgFlags &&f)
{
    std::swap(bits, f.bits);
    std::swap(item, f.item);
    choices = f.choices;
    f.choices = nullptr;
    return *this;
}

std::string &ArgFlags::get_choice(const int arg)
{
    int i = std::log2(arg);
    return choices[i];
}

int ArgParser::find_opt(char c)
{
    for (int i = 0; i < nargs; i++) {
        if (args[i].opt == c)
            return i;
    }
    return -1;
}

int ArgParser::find_opt(std::string s)
{
    for (int i = 0; i < nargs; i++) {
        if (args[i].long_opt == s)
            return i;
    }
    return -1;
}

bool ArgParser::get_choice(ArgFlags &flags, int i, std::string choice)
{
    bool found = false;

    if (!args[i].accept_any_choice) {
        for (auto s : args[i].choices) {
            if (choice == s) {
                found = true;
                break;
            }
        }
        if (!found)
            return false;
    }
    flags.choices[i] = choice;
    return true;
}

bool ArgParser::check_arg(ArgFlags &flags, const char *arg, const char *argnext)
{
    int i;
    bool valid = false, skip = false;
    
    i = (arg[1] == '-') ? find_opt(std::string(arg+2)) : find_opt(arg[1]);
    if (i == -1) {
        warning("invalid option: %s (will be ignored)\n", arg);
        return false;
    }

    flags.bits |= args[i].flagbit;
    if (args[i].has_choices) {
        skip = true;
        if (!argnext)
            warning("%s: must specify a choice\n", arg);
        else {
            valid = get_choice(flags, i, std::string(argnext));
            if (!valid)
                warning("invalid choice %s for option %s (will be ignored)\n", argnext, arg);
        }
    }

    return skip && valid;
}

// NOTE: public functions
ArgFlags ArgParser::parse_args(int argc, char *argv[])
{
    bool skip_arg = false;
    ArgFlags f(nargs);

    for (int i = 0; i < nargs; i++)
        f.choices[i] = "";
    while (++argv, --argc > 0) {
        if (*argv[0] == '-') {
            skip_arg = check_arg(f, *argv, argv[1]);
            if (skip_arg) {
                argv++;
                argc--;
            }
        } else {
            if (argc == 1)
                f.item = std::string(*argv);
            else
                warning("invalid option: %s (missing beginning -)\n", *argv);
        }
    }
    return f;
}

void ArgParser::print_usage()
{
    std::fprintf(stderr, "Usage: %s [args...] <ROM file>\n", progname);
    std::fprintf(stderr, "Valid arguments:\n");
    for (int i = 0; i < nargs; i++) {
        std::fprintf(stderr, "\t-%c, --%s\t\t%s\n",
                args[i].opt, args[i].long_opt.c_str(), args[i].desc.c_str());
    }
}

} // namespace CommandLine