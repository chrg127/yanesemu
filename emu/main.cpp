#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/version.hpp>
#include <emu/core/emulator.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/easyrandom.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/file.hpp>
#include <emu/video/video.hpp>

static const Util::ValidArgStruct cmdflags = {
    { 'b', "break-on-brk", "Stops emulation when BRK is encountered." },
    { 'l', "log-file",     "Log to this file. Pass stdout/stderr to print to stdout/stderr.",          Util::ParamType::MUST_HAVE },
    { 'd', "dump-file",    "Dump memory to this file. Pass stdout/stderr to print to stdout/stderr. ", Util::ParamType::MUST_HAVE },
    { 'h', "help",         "Print this help text and quit"            },
    { 'v', "version",      "Shows the program's version"              },
    { 'a', "debugger",     "Use command-line debugger"                },
};
static Core::Emulator emu;
static Util::File logfile, dumpfile;
static Video::Context context;
static Util::ArgResult flags;

void mainloop()
{
    Video::Canvas screen { context, Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT };
    bool running = true;
    SDL_Event ev;

    Util::seed();
    emu.set_screen(&screen);
    emu.power();
    if (logfile)
        logfile.print("{}\n", emu.rominfo());
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
        if (logfile)
            logfile.print(emu.status() + '\n');
        emu.run();
        screen.update();
        context.draw();
    }
    // emu.dump(dumpfile);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        Util::print_usage(progname, cmdflags);
        return 1;
    }

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

    // open log files -- these are used to log emulator info and dump memory
    auto open_logfile = [](Util::File &f, char flag) {
        if (!flags.has[flag] || flags.params[flag] == "")
            return;
        std::string_view s = flags.params[flag];
        if      (s == "stdout") f.assoc(stdout);
        else if (s == "stderr") f.assoc(stderr);
        else if (!f.open(s.data(), Util::File::Mode::WRITE))
            error("can't open {} for writing\n", s.data());
    };
    open_logfile(logfile, 'l');
    open_logfile(dumpfile, 'd');

    if (flags.has['a'])
        emu.enable_debugger([](Core::Debugger &db, Core::Debugger::Event &ev) { Core::clirepl(db, ev); });

    mainloop();

    return 0;
}

