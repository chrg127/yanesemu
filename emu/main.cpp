#include <cstdio>
#include <cstdlib>
#include <emu/utils/cmdargs.hpp>
#include <emu/core/bus.hpp>
#include <emu/core/cpu.hpp>
#include <emu/file/filebuf.hpp>
#include <emu/file/nesrom.hpp>
#define DEBUG
#include <emu/utils/debug.hpp>

static const char *version_str = "0.1";
static const int NUM_FLAGS = 5;
static Utils::ArgFlags flags(NUM_FLAGS);

enum Args : uint32_t {
    ARG_BREAK_ON_BRK = 0x01,
    ARG_LOG_FILE     = 0x02,
    ARG_DUMP_FILE    = 0x04,
    ARG_HELP         = 0x20000000,
    ARG_VERSION      = 0x40000000,
};

static Utils::ArgOption cmdflags[] = {
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

void print_usage(const char *progname);
void logopen(File::FileBuf &f, uint32_t arg);

void print_usage(const char *progname)
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

void logopen(File::FileBuf &f, uint32_t arg)
{
    if ((flags.bits & arg) == 0)
        return;

    std::string &s = flags.get_choice(arg);
    if (s == "stdout")
        f.assoc(stdout, File::Mode::WRITE);
    else if (s == "stderr")
        f.assoc(stderr, File::Mode::WRITE);
    else if (s == "")
        return;
    else {
        if (!f.open(s, File::Mode::WRITE))
            error("can't open %s for writing\n", s.c_str());
    }
}

int main(int argc, char *argv[])
{
    Utils::ArgParser parser(cmdflags, NUM_FLAGS);
    File::FileBuf logfile, dumpfile;
    File::ROM rom;
    Core::Bus bus;
    Core::CPU cpu(&bus);
    bool done = false;
    int counter;
    
    if (argc < 2) {
        print_usage(*argv);
        return 1;
    }

    parser.parse_args(flags, argc, argv);
    if (flags.bits & ARG_HELP) {
        print_usage(*argv);
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

    logopen(logfile, ARG_LOG_FILE);
    logopen(dumpfile, ARG_DUMP_FILE);
    
    rom.printinfo(logfile);
    cpu.power(rom.get_prgrom(), rom.get_prgrom_size());
    counter = 20;
    while (!done) {
        cpu.main();
        cpu.printinfo(logfile);
        cpu.disassemble(logfile);
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

    return 0;
}

