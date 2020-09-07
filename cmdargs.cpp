#include "cmdargs.h"

#include <cmath>
#include "debug.h"

static struct {
    char opt;
    int flagbit;
    std::string long_opt;
    std::string desc;
    bool has_choices;
    bool accept_any_choice;
    std::vector<std::string> choices;
} cmdflags[CMDFLAGS_NUM] = {
    { 'b', ARG_BREAK_ON_BRK, "break-on-brk", "Stops emulation when BRK is encountered.", false, false, {} },
    { 'l', ARG_LOG_FILE,     "log-file",     "The file where to log instructions.",      true,  true,  {} },
    { 'd', ARG_DUMP_FILE,    "dump-file",    "The file where to dump memory.",           true,  true,  {} },
    { 'h', ARG_HELP,         "help",         "Print this help text and quit",            false, false, {} },
    { 'v', ARG_VERSION,      "version",      "Shows the program's version",              false, false, {} },
};

static std::string cmdflags_choice_values[CMDFLAGS_NUM] = { "", "", "" };

static int find_opt(char c)
{
    for (int i = 0; i < CMDFLAGS_NUM; i++) {
        if (cmdflags[i].opt == c)
            return i;
    }
    return -1;
}

static int find_opt(std::string s)
{
    for (int i = 0; i < CMDFLAGS_NUM; i++) {
        if (cmdflags[i].long_opt == s)
            return i;
    }
    return -1;
}

bool get_choice(Flags &flags, int i, std::string choice)
{
    bool found = false;

    if (!cmdflags[i].accept_any_choice) {
        for (auto s : cmdflags[i].choices) {
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

static bool check_arg(Flags &flags, const char *arg, const char *argnext)
{
    int i;
    bool valid_choice = false, skip = false;
    
    if (arg[1] == '-')
        i = find_opt(std::string(arg+2));
    else
        i = find_opt(arg[1]);

    if (i == -1) {
        warning("invalid option: %s (will be ignored)\n", arg);
        return false;
    }
    if (cmdflags[i].has_choices) {
        skip = true;
        valid_choice = get_choice(flags, i, std::string(argnext));
        if (!valid_choice)
            warning("invalid choice %s for option %s (will be ignored)\n", argnext, arg);
    }
    flags.bits |= cmdflags[i].flagbit;

    return skip && valid_choice;
}

std::vector<char *> parse_args(Flags &flags, int argc, char *argv[])
{
    bool skip_arg = false;
    std::vector<char *> v;
    
    flags.bits = 0;   
    while (++argv, --argc > 0) {
        if (*argv[0] == '-') {
            skip_arg = check_arg(flags, *argv, argv[1]);
            if (skip_arg)
                argv++;
        } else
            v.push_back(*argv);
    }
    return v;
}

std::string get_arg_choices(Flags &flags, int arg)
{
    int i = std::log2(arg);
    if (!cmdflags[i].has_choices)
        return "";
    else
        return flags.choices[i];
}

void print_usage(const char *progname)
{
    std::fprintf(stderr, "Usage: %s [args...] [ROM file]\n", progname);
    std::fprintf(stderr, "Valid arguments:\n");
    for (int i = 0; i < CMDFLAGS_NUM; i++) {
        std::fprintf(stderr, "\t-%c, --%s\t\t%s\n",
                cmdflags[i].opt, cmdflags[i].long_opt.c_str(), cmdflags[i].desc.c_str());
    }
}

