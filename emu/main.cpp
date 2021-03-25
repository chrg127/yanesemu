#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/version.hpp>
#include <emu/core/emulator.hpp>
#include <emu/core/clidbg.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>
#include <emu/video/video.hpp>

static Core::Emulator emu;
static Video::Context context;
static Util::ArgResult flags;

void mainloop()
{
    Video::Canvas screen { context, Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT };
    bool running = true;
    SDL_Event ev;
    Core::CliDebugger clidbg;

    Util::seed();
    emu.set_screen(&screen);
    if (flags.has['d']) {
        emu.enable_debugger([&clidbg](Core::Debugger &db, Core::Debugger::Event &&ev) {
            clidbg.repl(db, std::move(ev));
        });
    }
    emu.power();
    fmt::print(stderr, "{}\n", emu.rominfo());
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
        if (emu.debugger_has_quit())
            running = false;
        emu.run();
        screen.update();
        context.draw();
    }
}

static const Util::ValidArgStruct cmdflags = {
    { 'h',  "help",     "Print this help text and quit" },
    { 'v',  "version",  "Shows the program's version"   },
    { 'd',  "debugger", "Use command-line debugger"     },
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        Util::print_usage(progname, cmdflags);
        return 1;
    }
#ifdef _WIN32
    // for some reason, running in mingw, the standard output shows itself
    // only at the program's exit. this is a work-around.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif
    // parse command line arguments
    flags = Util::parse(argc, argv, cmdflags);
    if (flags.has['h']) { Util::print_usage(progname, cmdflags); return 0; }
    if (flags.has['v']) { Util::print_version(progname, version); return 0; }

    // open rom file
    if (flags.items.size() == 0) {
        error("ROM file not specified\n");
        return 1;
    } else if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");
    Util::File romfile(flags.items[0], Util::File::Mode::READ);
    if (!romfile) {
        error("{}: {}\n", flags.items[0], romfile.error_str());
        return 1;
    }
    if (!emu.insert_rom(romfile)) {
        error("invalid ROM format\n");
        return 1;
    }
    romfile.close();

    // initialize video subsystem
    if (!context.init(Video::Context::Type::OPENGL)) {
        error("can't initialize video\n");
        return 1;
    }

    mainloop();

    return 0;
}

