#include <thread>
#include <mutex>
#include <unistd.h>
#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/emulator.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/file.hpp>
#include <emu/util/easyrandom.hpp>
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
static bool global_done = false;
static std::mutex done_mutex;

bool check_done()
{
    std::lock_guard<std::mutex> lock(done_mutex);
    return global_done;
}

void set_done()
{
    std::lock_guard<std::mutex> lock(done_mutex);
    global_done = true;
}

void event_thread()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("yanesemu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          640, 480, SDL_WINDOW_SHOWN);
    // if (!window) {
    //     error("can't create window\n");
    //     return 1; // ???????
    // }

    SDL_Renderer *rnd = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    static SDL_Event ev;

    while (!check_done()) {
        if (SDL_WaitEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                fmt::print("quitting");
                set_done();
            }
        }
        SDL_SetRenderDrawColor(rnd, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(rnd);
        SDL_RenderPresent(rnd);
    }
    SDL_DestroyRenderer(rnd);
    SDL_DestroyWindow(window);
}

static Util::File logfile, dumpfile;

void emulator_thread()
{
    while (!check_done()) {
        emu.log(logfile);
        emu.run();
        // usleep(1);
    }
}

int main(int argc, char *argv[])
{
    // Video::Video v;

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

    // initialize two threads and join
    std::thread event_th(event_thread);
    std::thread emu_th(emulator_thread);
    event_th.join();
    emu_th.join();
    return 0;
}

