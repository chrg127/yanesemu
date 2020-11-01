#include <emu/utils/cmdargs.hpp>

#include <cmath>
#include <utility>
#include <emu/utils/debug.hpp>
#include <emu/utils/file.hpp>

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

std::string_view ArgFlags::get_choice(const int arg) const
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

int ArgParser::find_opt(std::string_view s)
{
    for (int i = 0; i < nargs; i++) {
        if (args[i].long_opt == s)
            return i;
    }
    return -1;
}

bool ArgParser::get_choice(ArgFlags &flags, int i, std::string_view choice)
{
    bool found = false;

    if (!args[i].accept_any_choice) {
        for (auto &s : args[i].choices) {
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

    i = (arg[1] == '-') ? find_opt(std::string_view(arg+2)) : find_opt(arg[1]);
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
            valid = get_choice(flags, i, std::string_view(argnext));
            if (!valid)
                warning("invalid choice %s for option %s (will be ignored)\n", argnext, arg);
        }
    }
    return skip && valid;
}

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
            if (f.item != "")
                warning("%s: item already specified", *argv);
            f.item = std::string_view(*argv);
        }
    }
    return f;
}

void ArgParser::print_usage() const
{
    Utils::File fout(stderr, Utils::File::Mode::WRITE);
    fout.printf("Usage: %s [args...] <ROM file>\n", progname.data());
    fout.printf("Valid arguments:\n");
    for (int i = 0; i < nargs; i++) {
        fout.printf("\t-%c, --%s\t\t%s\n",
                args[i].opt, args[i].long_opt.data(), args[i].desc.data());
    }
}

void ArgParser::print_version() const
{
    Utils::File fout(stderr, Utils::File::Mode::WRITE);
    fout.printf("%s: version %s\n", progname.data(), verstr.data());
}

} // namespace Utils
