#include <fmt/core.h>
#include <emu/emulator.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/file.hpp>
#include <emu/video/video.hpp>
#define DEBUG
#include <emu/util/debug.hpp>

static const Util::ValidArgStruct cmdflags = {
    { 'b', "break-on-brk", "Stops emulation when BRK is encountered." },
    { 'l', "log-file",     "The file where to log instructions. "
                           "Pass \"stdout\" to print to stdout, "
                           "\"stderr\" to print to stderr.",          Util::ParamType::MUST_HAVE },
    { 'd', "dump-file",    "The file where to dump memory."
                           "Pass \"stdout\" to print to stdout, "
                           "\"stderr\" to print to stderr.",          Util::ParamType::MUST_HAVE },
    { 'h', "help",         "Print this help text and quit"            },
    { 'v', "version",      "Shows the program's version"              },
};
static constexpr std::string_view progname = "yanesemu";
static constexpr std::string_view version  = "0.1";
static Emulator emu;

int main(int argc, char *argv[])
{
    Util::File logfile, dumpfile;
    Video::Video v;

    // parse command line arguments
    if (argc < 2) {
        Util::print_usage(progname, cmdflags, false);
        return 1;
    }
    Util::ArgResult flags = Util::parse(argc, argv, cmdflags);
    if (flags.found['h']) { Util::print_usage(progname, cmdflags); return 0; }
    if (flags.found['v']) { Util::print_version(progname, version; return 0; }
    if (flags.items.size() == 0) {
        error("ROM file not specified\n");
        return 1;
    } else if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");
    if (!emu.init(flags.items[0]))
        return 1;

    // open log files -- these will be used to log emulator info and dump memory
    // at the end
    auto logopen = [](Util::File &f, char flag, Util::ArgResult &args) {
        if (!args.found[flag] || args.params[flag] == "")
            return;
        std::string_view s = args.params[flag];
        if      (s == "stdout") f.assoc(stdout, Util::File::Mode::WRITE);
        else if (s == "stderr") f.assoc(stderr, Util::File::Mode::WRITE);
        else if (!f.open(s, Util::File::Mode::WRITE))
            error("can't open %s for writing\n", s.data());
    };
    logopen(logfile, 'l', flags);
    logopen(dumpfile, 'd', flags);
    if (logfile.isopen()) {
        logfile.putstr(emu.rominfo());
        logfile.putc('\n');
    }

    return 0;
}

