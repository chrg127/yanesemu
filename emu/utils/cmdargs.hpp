#ifndef UTILS_CMDARGS_HPP_INCLUDED
#define UTILS_CMDARGS_HPP_INCLUDED

/* 
 * simple libray for command argument. arguments must be passed in the form:
 * <program name> <--options> <item>
 * that is, options must be present before the item.
 */

#include <vector>
#include <string>
#include <cstdint>

namespace Utils {

struct ArgOption {
    char opt;               /* short option */
    int flagbit;            /* corresponding flag bit */
    std::string long_opt;   /* long option */
    std::string desc;       /* a description (can be used for usage text) */
    bool has_choices;       /* if the option accept choices */
    bool accept_any_choice; /* if we can pick up any choice */
    std::vector<std::string> choices;   /* if we can't, what choices are ammissible */
};

struct ArgFlags {
    uint32_t bits = 0;      /* what arguments have been found */
    std::string *choices;   /* the choices found for any arg that accept them */ 
    std::string item = "";  /* the not-an-option item */

    ArgFlags() : choices(nullptr)
    { }
    ArgFlags(int n) : choices(new std::string[n])
    { }

    ~ArgFlags()
    {
        if (choices)
            delete[] choices;
    }

    ArgFlags(ArgFlags &&f);
    ArgFlags &operator=(ArgFlags &&f);

    std::string &get_choice(const int arg);
};

class ArgParser {
    ArgOption *args;     /* array of arguments */
    int nargs;           /* size of array */
    const char *progname; /* for print_usage */

    int find_opt(char c);
    int find_opt(std::string s);
    bool get_choice(ArgFlags &flags, int i, std::string choice);
    bool check_arg(ArgFlags &flags, const char *arg, const char *argnext);

public:
    ArgParser(const char *name, ArgOption *a, int n)
        : args(a), nargs(n), progname(name)
    { }

    ArgFlags parse_args(int argc, char *argv[]);
    void print_usage();
};

} // namespace Utils

#endif
