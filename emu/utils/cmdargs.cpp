#include <emu/utils/cmdargs.hpp>

#include <cmath>
#include <emu/utils/debug.hpp>

namespace Utils {

std::string &ArgFlags::get_choice(int arg)
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
void ArgParser::parse_args(ArgFlags &f, int argc, char *argv[])
{
    bool skip_arg = false;
    
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
}

} // namespace CommandLine
