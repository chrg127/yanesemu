#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/core/emulator.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/file.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/video/video.hpp>
#include <emu/util/debug.hpp>
#include <emu/version.hpp>

static const Util::ValidArgStruct cmdflags = {
    { 'b', "break-on-brk", "Stops emulation when BRK is encountered." },
    { 'l', "log-file",     "Log to this file. Pass stdout/stderr to print to stdout/stderr.",          Util::ParamType::MUST_HAVE },
    { 'd', "dump-file",    "Dump memory to this file. Pass stdout/stderr to print to stdout/stderr. ", Util::ParamType::MUST_HAVE },
    { 'h', "help",         "Print this help text and quit"            },
    { 'v', "version",      "Shows the program's version"              },
};
static Core::Emulator emu;
static Util::File logfile, dumpfile;
static Video::Context context;

void mainloop()
{
    Video::Canvas screen { context, Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT };
    bool running = true;
    SDL_Event ev;

    Util::seed();
    emu.power();
    emu.set_screen(&screen);
    fmt::print("{}\n", emu.rominfo());
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
        emu.run_frame(logfile);
        screen.update();
        context.draw();
    }
    emu.dump(dumpfile);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        Util::print_usage(progname, cmdflags);
        return 1;
    }

    // parse command line arguments
    Util::ArgResult flags = Util::parse(argc, argv, cmdflags);
    if (flags.found['h']) { Util::print_usage(progname, cmdflags); return 0; }
    if (flags.found['v']) { Util::print_version(progname, version); return 0; }
    if (flags.items.size() == 0) {
        error("ROM file not specified\n");
        return 1;
    } else if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");

    // open rom file
    try {
        emu.insert_rom(flags.items[0]);
    } catch (std::exception &e) {
        error("{}\n", e.what());
        return 1;
    }

    // initialize video subsystem
    if (!context.init(Video::Context::Type::OPENGL)) {
        error("can't initialize video\n");
        return 1;
    }

    // open log files -- these will be used to log emulator info and dump memory
    auto open_logfile = [&flags](Util::File &f, char flag) {
        if (!flags.found[flag] || flags.params[flag] == "")
            return;
        std::string_view s = flags.params[flag];
        if      (s == "stdout") f.assoc(stdout);
        else if (s == "stderr") f.assoc(stderr);
        else if (!f.open(s.data(), Util::File::Mode::WRITE))
            error("can't open {} for writing\n", s.data());
    };
    open_logfile(logfile, 'l');
    open_logfile(dumpfile, 'd');

    mainloop();

    return 0;
}

