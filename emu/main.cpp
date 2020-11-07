#include <emu/emulator.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/utils/cmdargs.hpp>
#include <emu/utils/file.hpp>
#include <emu/video/video.hpp>
#define DEBUG
#include <emu/utils/debug.hpp>

using Utils::File;

enum {
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

static Utils::ArgParser parser("yanesemu", "0.1", cmdflags, 5);
static Emulator emu;

int main(int argc, char *argv[])
{
    File logfile, dumpfile;
    Core::Cartridge cart;
    Video::Video v;

    if (argc < 2) {
        parser.print_usage();
        return 1;
    }

    auto logopen = [](File &f, Utils::ArgFlags &flags, uint32_t arg) {
        if ((flags.bits & arg) == 0)
            return;
        std::string_view s = flags.get_choice(arg);
        if      (s == "stdout") f.assoc(stdout, File::Mode::WRITE);
        else if (s == "stderr") f.assoc(stderr, File::Mode::WRITE);
        else if (s == "")       return;
        else {
            if (!f.open(s, File::Mode::WRITE))
                error("can't open %s for writing\n", s.data());
        }
    };

    Utils::ArgFlags flags = parser.parse_args(argc, argv);
    logopen(logfile, flags, ARG_LOG_FILE);
    logopen(dumpfile, flags, ARG_DUMP_FILE);

    if (flags.bits & ARG_HELP) {
        parser.print_usage();
        return 0;
    } else if (flags.bits & ARG_VERSION) {
        parser.print_version();
        return 0;
    } else if (flags.get_item() == "") {
        error("ROM file not specified\n");
        return 1;
    } else if (!cart.open(flags.get_item())) {
        error("can't open rom file\n");
        return 1;
    }
    // else if (!v.create()) {
    //     error("can't initialize video subsytem\n");
    //     return 1;
    // }

    emu.init(cart);
    cart.printinfo(logfile);
    while (!v.closed()) {
        v.poll();
        v.render();
        emu.log(logfile);
        emu.run();
    }
    emu.dump(dumpfile);
    return 0;
}

