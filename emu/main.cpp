#include <cstdio>
#include <cstdlib>
#include <vector>
#include <emu/utils/cmdargs.h>
#include <emu/core/bus.h>
#include <emu/core/cpu.h>
#include <emu/nesrom.h>
#define DEBUG
#include <emu/utils/debug.h>

enum Args : uint32_t {
    ARG_BREAK_ON_BRK = 0x01,
    ARG_LOG_FILE     = 0x02,
    ARG_DUMP_FILE    = 0x04,
    ARG_HELP         = 0x20000000,
    ARG_VERSION      = 0x40000000,
};

const int NUM_FLAGS = 5;
static CommandLine::ArgOption cmdflags[] = {
    { 'b', ARG_BREAK_ON_BRK, "break-on-brk", "Stops emulation when BRK is encountered.", false, false, {} },
    { 'l', ARG_LOG_FILE,     "log-file",     "The file where to log instructions. "
                                             "Pass \"stdout\" to print to stdout, "
                                             "\"stderr\" to print to stderr.",           true,  true,  {} },
    { 'd', ARG_DUMP_FILE,    "dump-file",    "The file where to dump memory."
                                             "Pass \"stdout\" to print to stdout, "
                                             "\"stderr\" to print to stderr.",           true,  true,  {} },
    { 'h', ARG_HELP,         "help",         "Print this help text and quit",            false, false, {} },
    { 'v', ARG_VERSION,      "version",      "Shows the program's version",              false, false, {} },
};
static CommandLine::ArgFlags flags(NUM_FLAGS);
static char *progname;
static const char *version_str = "0.1";

void print_usage();
FILE *logopen(uint32_t arg);

void print_usage()
{
    std::fprintf(stderr, "Usage: %s [args...] <ROM file>\n", progname);
    std::fprintf(stderr, "Valid arguments:\n");
    for (int i = 0; i < NUM_FLAGS; i++) {
        std::fprintf(stderr, "\t-%c, --%s\t\t%s\n",
                cmdflags[i].opt, cmdflags[i].long_opt.c_str(), cmdflags[i].desc.c_str());
    }
}

// static const char *def_log  = "other/output.log";
// static const char *def_dump = "other/memdump.log";

FILE *logopen(uint32_t arg)
{
    FILE *f;
    
    if ((flags.bits & arg) == 0)
        return nullptr;

    std::string &s = flags.get_choice(arg);
    if (s == "stdout")
        f  = stdout;
    else if (s == "stderr")
        f  = stderr;
    else if (s == "")
        f = nullptr;
    else {
        const char *cs = s.c_str();
        f = fopen(cs, "w");
        if (!f)
            error("can't open %s for writing\n", cs);
    }
    return f;
}

int main(int argc, char *argv[])
{
    CommandLine::ArgParser parser(cmdflags, NUM_FLAGS);
    FILE *logfile, *dumpfile;
    nesrom::ROM rom;
    Processor::Bus bus;
    Processor::CPU cpu(&bus);
    bool done = false;
    int counter;
    
    progname = *argv;
    if (argc < 2) {
        print_usage();
        return 1;
    }

    parser.parse_args(flags, argc, argv);
    if (flags.bits & ARG_HELP) {
        print_usage();
        return 0;
    }
    if (flags.bits & ARG_VERSION) {
        std::printf("%s\n", version_str);
        return 0;
    }

    if (flags.item == "") {
        error("ROM file not specified\n");
        return 1;
    } else if (!rom.open(flags.item)) {
        error("%s\n", rom.geterr().c_str());
        return 1;
    }
    
    logfile = logopen(ARG_LOG_FILE);
    dumpfile = logopen(ARG_DUMP_FILE);
    
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
    if (logfile)
        std::fputs("", logfile);

    if (logfile && logfile != stdout && logfile != stderr)
        fclose(logfile);
    if (dumpfile && dumpfile != stdout && dumpfile != stderr)
        fclose(dumpfile);
    return 0;
}

