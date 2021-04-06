#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/version.hpp>
#include <emu/core/emulator.hpp>
#include <emu/debugger/clidbg.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>
#include <emu/video/video.hpp>

static const Util::ValidArgStruct cmdflags = {
    { 'h',  "help",     "Print this help text and quit" },
    { 'v',  "version",  "Shows the program's version"   },
    { 'd',  "debugger", "Use command-line debugger"     },
};
static Core::Emulator emu;

int cli_interface()
{
    Video::Context context;
    // initialize video subsystem
    if (!context.init(Video::Context::Type::OPENGL)) {
        error("can't initialize video\n");
        return 1;
    }

    Video::Canvas screen { context, Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT };
    bool running = true;
    SDL_Event ev;

    emu.set_screen(&screen);
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
        emu.run_frame();
        screen.update();
        context.draw();
    }
    return 0;
}

int debugger_interface()
{
    emu.power();
    fmt::print("{}\n", emu.rominfo());
    Debugger::CliDebugger clidbg{&emu};
    clidbg.repl();
    return 0;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // windows doesn't have line buffering, and full buffering is just bad
    // for stdout and stderr, so set no buffering to these two.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif

    if (argc < 2) {
        Util::print_usage(progname, cmdflags);
        return 1;
    }

    // parse command line arguments
    Util::ArgResult flags;
    flags = Util::parse(argc, argv, cmdflags);
    if (flags.has['h']) { Util::print_usage(progname, cmdflags); return 0; }
    if (flags.has['v']) { Util::print_version(progname, version); return 0; }

    // open rom file
    if (flags.items.empty()) {
        error("ROM file not specified\n");
        return 1;
    } else if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");

    Util::File romfile{flags.items[0], Util::Access::READ};
    if (!romfile) {
        error("{}: ", flags.items[0]);
        std::perror("");
        return 1;
    }
    if (!emu.insert_rom(romfile)) {
        error("invalid ROM format\n");
        return 1;
    }
    romfile.close();

    Util::seed();
    if (flags.has['d'])
        return debugger_interface();
    return cli_interface();
}

