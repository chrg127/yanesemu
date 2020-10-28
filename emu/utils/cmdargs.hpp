#ifndef UTILS_CMDARGS_HPP_INCLUDED
#define UTILS_CMDARGS_HPP_INCLUDED

/* 
 * simple libray for command argument. arguments must be passed in the form:
 * <program name> <--options> <item>
 * that is, options must be present before the item.
 */

#include <vector>
#include <string_view>
#include <cstdint>

namespace Utils {

struct ArgOption {
    char opt;                   /* short option */
    int flagbit;                /* corresponding flag bit */
    std::string_view long_opt;  /* long option */
    std::string_view desc;      /* a description (can be used for usage text) */
    bool has_choices;           /* if the option accept choices */
    bool accept_any_choice;     /* if we can pick up any choice */
    std::vector<std::string_view> choices;   /* if we can't, what choices are ammissible */
};

struct ArgFlags {
    uint32_t bits = 0;                      /* what arguments have been found */
    std::string_view *choices = nullptr;    /* the choices found for any arg that accept them */ 
    std::string_view item = "";             /* the not-an-option item */

    ArgFlags() = default;
    ArgFlags(int n) : choices(new std::string_view[n])
    { }
    ArgFlags(const ArgFlags &f) = delete;
    ArgFlags(ArgFlags &&f);
    ArgFlags & operator=(const ArgFlags &f) = delete;
    ArgFlags & operator=(ArgFlags &&f);
    ~ArgFlags()
    {
        if (choices)
            delete[] choices;
    }

    std::string_view get_choice(const int arg) const;
    inline std::string_view get_item() const
    { return item; }

    friend class ArgParser;
};

class ArgParser {
    ArgOption *args;            /* array of arguments */
    const int nargs;                  /* size of array */
    std::string_view progname;  /* for print_usage */

    int find_opt(char c);
    int find_opt(std::string_view s);
    bool get_choice(ArgFlags &flags, int i, std::string_view choice);
    bool check_arg(ArgFlags &flags, const char *arg, const char *argnext);

public:
    ArgParser(std::string_view name, ArgOption *a, int n)
        : args(a), nargs(n), progname(name)
    { }

    ArgFlags parse_args(int argc, char *argv[]);
    void print_usage() const;
};

} // namespace Utils

#endif
