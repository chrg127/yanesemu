#include <thread>
#include <mutex>
#include <condition_variable>
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

std::mutex frame_mutex;
std::condition_variable required_cond;
unsigned frame_pending = 0;
bool thread_running = true;
bool wait_for_frame_update = true;
std::mutex running_mutex;

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

void end_emulator_thread()
{
    std::lock_guard<std::mutex> lock(running_mutex);
    std::lock_guard<std::mutex> lock2(frame_mutex);
    if (!thread_running)
        return;
    thread_running = false;
    wait_for_frame_update = false;
    frame_pending = 0;
    required_cond.notify_one();
}

bool emulator_running()
{
    std::lock_guard<std::mutex> lock(running_mutex);
    return thread_running;
}

void emulator_thread()
{
    while (emulator_running()) {
        // fmt::print("running for one frame\n");
        emu.run_frame();
        // fmt::print("finished running, update\n");
        std::unique_lock<std::mutex> lock{frame_mutex};
        // fmt::print("add to frame_pending: {} -> {}\n", frame_pending, frame_pending + 1);
        frame_pending += 1;
        do {
            // fmt::print("emulator: waiting on required_cond\n");
            if (wait_for_frame_update)
                required_cond.wait(lock);
            // fmt::print("finished waiting, frame_pending = {}\n", frame_pending);
        } while (frame_pending != 0 && wait_for_frame_update);
        // fmt::print("emulator: resuming\n");
        // fmt::print("emulator: checking running\n");
    }
    // fmt::print("emulation finished\n");
}



void rendering_thread(Video::Context &ctx, Video::Texture &screen)
{
    // return whether there are any new frames.
    const auto wait_frame_start = []() -> bool
    {
        // fmt::print("frame_pending = {}\n", frame_pending);
        if (frame_pending == 0)
            return false;
        frame_pending = 0;
        return true;
    };

    while (emulator_running()) {
        for (SDL_Event ev; SDL_PollEvent(&ev) != 0; ) {
            switch (ev.type) {
            case SDL_QUIT:
                end_emulator_thread();
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    ctx.resize(ev.window.data1, ev.window.data2);
            }
        }

        {
            std::unique_lock<std::mutex> lock{frame_mutex};
            if (wait_frame_start()) {
                ctx.update_texture(screen, emu.get_screen());
            }
            required_cond.notify_one();
        }

        ctx.update_texture(screen, emu.get_screen());
        ctx.use_texture(screen);
        ctx.draw();
    }
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

    Video::Texture tex = context->create_texture(Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT);

    emu.power();

    // run emulator and rendering in two separate threads
    std::thread emuthread{emulator_thread};

    /* note that we must run everything related to rendering in
     * the same thread where we created the context */
    rendering_thread(*context, tex);

    emuthread.join();

    return 0;
}

/*
int debugger_interface(const Util::ArgResult &flags)
{
    if (!open_rom_with_flags(flags))
        return 1;

    emu.power();
    Debugger::CliDebugger clidbg{&emu};
    clidbg.enter();
    return 0;
}
*/

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

    Util::seed();
    // if (flags.has['d'])
    //     return debugger_interface(flags);
    return cli_interface(flags);
}

