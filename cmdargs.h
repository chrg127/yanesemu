#ifndef CMDLINE_H_INCLUDED
#define CMDLINE_H_INCLUDED

#include <vector>
#include <string>
#include <cstdint>

const int CMDFLAGS_NUM = 5;

enum Args : uint32_t {
    ARG_BREAK_ON_BRK = 0x01,
    ARG_LOG_FILE     = 0x02,
    ARG_DUMP_FILE    = 0x04,
    ARG_HELP         = 0x20000000,
    ARG_VERSION      = 0x40000000,
};

struct Flags {
    uint32_t bits;
    std::string choices[CMDFLAGS_NUM];
};

std::vector<char *> parse_args(Flags &flags, int argc, char *argv[]);
std::string get_arg_choices(Flags &flags, int arg);
void print_usage(const char *progname);
void print_version();

#endif

