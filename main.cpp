#include <cstdio>
#include <cstdlib>
#include <vector>
#include "cmdargs.h"
#include "bus.h"
#include "cpu.h"
#include "nesrom.h"
#define DEBUG
#include "debug.h"

static Flags flags;
static const char *version_str = "0.1";
// static const char *def_log  = "other/output.log";
static const char *def_dump = "other/memdump.log";

int main(int argc, char *argv[])
{
    nesrom::ROM rom;
    Processor::Bus bus;
    Processor::CPU cpu(bus);
    bool done = false;
    FILE *logfile, *dumpfile;
    char *romname = nullptr;
    int counter;

    if (argc < 2) {
        print_usage(*argv);
        return 1;
    }

    auto v = parse_args(flags, argc, argv);
    
    if (flags.bits & ARG_HELP) {
        print_usage(*argv);
        return 0;
    }
    if (flags.bits & ARG_VERSION) {
        std::printf("%s\n", version_str);
        return 0;
    }

    if (v.size() != 1) {
        if (v.size() == 0)
            error("ROM file not specified\n");
        else
            error("Too many files specified\n");
        return 1;
    }
    romname = v[0];

    if (rom.open(romname) != 0) {
        error("%s\n", rom.get_errormsg());
        return 1;
    }
    
    if (flags.bits & ARG_LOG_FILE) {
        auto s = get_arg_choices(flags, ARG_LOG_FILE);
        if (s == "stdout")
            logfile = stdout;
        else if (s == "stderr")
            logfile = stderr;
        else {
            logfile = fopen(s.c_str(), "r");
            if (!logfile) {
                error("%s: no such file or directory\n", s.c_str());
                return 1;
            }
        }
    } else
        logfile = stdout;

    if (flags.bits & ARG_DUMP_FILE) {
        auto s = get_arg_choices(flags, ARG_DUMP_FILE);
        if (s == "stdout")
            dumpfile = stdout;
        else if (s == "stderr")
            dumpfile = stderr;
        else {
            dumpfile = fopen(s.c_str(), "r");
            if (!dumpfile) {
                error("%s: no such file or directory\n", s.c_str());
                return 1;
            }
        }
    } else
        dumpfile = fopen(def_dump, "r");



    rom.printinfo(logfile);
    cpu.power(rom.get_prgrom(), rom.get_prgrom_size());
    counter = 20;
    while (!done) {
        cpu.main();
        cpu.printinfo(logfile);
        if (flags.bits & ARG_BREAK_ON_BRK && cpu.peek_opcode() == 0) {
            DBGPRINT("got BRK, stopping emulation\n");
            bus.memdump(dumpfile);
            done = true;
        }
        if (--counter < 0) {
            cpu.reset();
            bus.memdump(dumpfile);
            done = true;
        }
    }
    puts("");
    fclose(dumpfile);

    return 0;
}

