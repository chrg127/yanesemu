#ifndef CMDLINE_H_INCLUDED
#define CMDLINE_H_INCLUDED

#include <vector>
#include <string>
#include <cstdint>

namespace Utils {

struct ArgOption {
    char opt;
    int flagbit;
    std::string long_opt;
    std::string desc;
    bool has_choices;
    bool accept_any_choice;
    std::vector<std::string> choices;
};

struct ArgFlags {
    uint32_t bits = 0;      /* what arguments have been found */                 
    std::string *choices;   /* the choices found for any arg that accept them */ 
    std::string item = "";  /* the not-an-option item */
    
    ArgFlags(int n) 
    {
        choices = new std::string[n];
    }

    ~ArgFlags()
    { 
        delete[] choices;
    }

    std::string &get_choice(int arg);
};

class ArgParser {
    ArgOption *args;     /* array of arguments */
    int nargs;          /* size of array */

    int find_opt(char c);
    int find_opt(std::string s);
    bool get_choice(ArgFlags &flags, int i, std::string choice);
    bool check_arg(ArgFlags &flags, const char *arg, const char *argnext);

public:
    ArgParser(ArgOption *a, int n)
        : args(a), nargs(n)
    { }

    void parse_args(ArgFlags &f, int argc, char *argv[]);
};

} // namespace CommandLine

#endif

