#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/version.hpp>
#include <emu/core/cartridge.hpp>
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

bool open_rom(std::string_view pathname)
{
    auto opt_file = Util::File::open(pathname, Util::Access::READ);
    if (!opt_file) {
        std::perror("error");
        return false;
    }
    auto opt_cart = Core::parse_cartridge(opt_file.value());
    if (!opt_cart) {
        error("not a real NES ROM: {}\n", pathname);
        return false;
    }
    fmt::print("{}\n", opt_cart.value().to_string());
    emu.insert_rom(std::move(opt_cart.value()));
    return true;
}

bool open_rom_with_flags(const Util::ArgResult &flags)
{
    if (flags.items.empty()) {
        error("ROM file not specified\n");
        return false;
    }
    if (flags.items.size() > 1)
        warning("Multiple ROM files specified, only the first will be chosen\n");
    return open_rom(flags.items[0]);
}

int cli_interface(const Util::ArgResult &flags)
{
    if (!open_rom_with_flags(flags))
        return 1;

    // initialize video subsystem
    auto context = Video::Context::create(Video::Context::Type::OPENGL);
    if (!context) {
        error("can't initialize video\n");
        return 1;
    }

    Video::Canvas screen = context->create_canvas(Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT);
    bool running = true;
    SDL_Event ev;

    emu.set_screen(&screen);
    emu.power();
    // fmt::print("{}\n", emu.rominfo());
    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    context->resize(ev.window.data1, ev.window.data2);
            }
        }
        emu.run_frame();
        context->update_canvas(screen);
        context->use_texture(screen);
        context->draw();
        SDL_Delay(1000 / 60);
    }
    return 0;
}

int debugger_interface(const Util::ArgResult &flags)
{
    if (!open_rom_with_flags(flags))
        return 1;

    emu.power();
    Debugger::CliDebugger clidbg{&emu};
    clidbg.enter();
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

    Util::ArgResult flags;
    flags = Util::parse(argc, argv, cmdflags);
    if (flags.has['h']) {
        Util::print_usage(progname, cmdflags);
        return 0;
    }
    if (flags.has['v']) {
        Util::print_version(progname, version);
        return 0;
    }

    if (flags.has['d'])
        return debugger_interface(flags);
    return cli_interface(flags);


    Util::seed();
    if (flags.has['d'])
        return debugger_interface(flags);
    return cli_interface(flags);
}

