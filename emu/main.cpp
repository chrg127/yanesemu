#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/emulator.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/file.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/video/video.hpp>
#include <emu/util/debug.hpp>
#include <emu/version.hpp>

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
static Emulator emu;
static Util::File logfile, dumpfile;

int main(int argc, char *argv[])
{
    // parse command line arguments
    if (argc < 2) {
        Util::print_usage(progname, cmdflags);
        return 1;
    }

    Util::ArgResult flags = Util::parse(argc, argv, cmdflags);
    if (flags.found['h']) { Util::print_usage(progname, cmdflags); return 0; }
    if (flags.found['v']) { Util::print_version(progname, version); return 0; }
    if (flags.items.size() == 0) {
        error("ROM file not specified\n");
        return 1;
    } else if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");

    try {
        emu.insert_rom(flags.items[0]);
    } catch (std::exception &e) {
        error("%s\n", e.what());
        return 1;
    }
    Video::Context context;
    if (!context.init(Video::Context::Type::OPENGL)) {
        error("can't initialize video\n");
        return 1;
    }
    Util::seed();
    emu.power();

    // open log files -- these will be used to log emulator info and dump memory
    auto logopen = [&flags](Util::File &f, char flag) {
        if (!flags.found[flag] || flags.params[flag] == "")
            return;
        std::string_view s = flags.params[flag];
        if      (s == "stdout") f.assoc(stdout, Util::File::Mode::WRITE);
        else if (s == "stderr") f.assoc(stderr, Util::File::Mode::WRITE);
        else if (!f.open(s, Util::File::Mode::WRITE))
            error("can't open %s for writing\n", s.data());
    };
    logopen(logfile, 'l');
    logopen(dumpfile, 'd');
    if (logfile.isopen()) {
        logfile.putstr(emu.rominfo());
        logfile.putc('\n');
    }

    bool running = true;
    SDL_Event ev;
    Video::Canvas screen { context, 256, 224 };
    emu.set_screen(&screen);
    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    context.resize(ev.window.data1, ev.window.data2);
            }
        }
        emu.log(logfile);
        emu.wait_nmi();
        screen.update();
        context.draw();
    }
    return 0;
}

