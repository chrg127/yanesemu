#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <fmt/core.h>
#include <SDL2/SDL.h>
#include <emu/version.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/emulator.hpp>
#include <emu/debugger/clidbg.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/mappedfile.hpp>
#include <emu/util/os.hpp>
#include <emu/platform/video.hpp>

static core::Emulator emu;

class MainThread {
    enum State {
        RUNNING,
        EXITING,
    } state = State::RUNNING;

    std::thread th;
    std::mutex frame_mutex;
    std::mutex running_mutex;
    std::condition_variable required_cond;
    unsigned frame_pending = 0;
    bool wait_for_frame_update = true;

public:
    void run(auto &&fn)
    {
        th = std::thread(fn);
    }

    bool running()
    {
        std::lock_guard<std::mutex> lock(running_mutex);
        return state == State::RUNNING;
    }

    void new_frame()
    {
        std::unique_lock<std::mutex> lock{frame_mutex};
        frame_pending += 1;
        do {
            if (wait_for_frame_update)
                required_cond.wait(lock);
        } while (frame_pending != 0 && wait_for_frame_update);
    }

    void end()
    {
        std::lock_guard<std::mutex> lock(running_mutex);
        std::lock_guard<std::mutex> lock2(frame_mutex);
        if (state != State::RUNNING)
            return;
        state = State::EXITING;
        wait_for_frame_update = false;
        frame_pending = 0;
        required_cond.notify_one();
    }

    void run_on_frame_pending(auto &&fn)
    {
        std::unique_lock<std::mutex> lock{frame_mutex};
        if (frame_pending != 0) {
            frame_pending = 0;
            fn();
        }
        required_cond.notify_one();
    }

    void join() { th.join(); }
};

[[nodiscard]]
io::MappedFile open_rom(std::string_view rompath)
{
    auto romfile = io::MappedFile::open(rompath);
    if (!romfile)
        throw std::runtime_error(fmt::format("couldn't open {}: {}", rompath, util::system_error_string()));
    auto cart = core::parse_cartridge(romfile.value());
    if (!cart)
        throw std::runtime_error(fmt::format("not a real NES ROM: {}", romfile.value().filename()));
    fmt::print("{}\n", cart.value().to_string());
    emu.insert_rom(std::move(cart.value()));
    return std::move(romfile.value());
}

void rendering_thread(MainThread &mainthread, platform::Video &ctx, platform::Texture &screen)
{
    while (mainthread.running()) {
        ctx.poll();
        if (ctx.has_quit())
            mainthread.end();
        mainthread.run_on_frame_pending([&]() {
            ctx.update_texture(screen, emu.get_screen());
        });
        ctx.clear();
        ctx.draw_texture(screen, 0, 0);
        ctx.swap();
    }
}

static const std::vector<cmdline::Argument> cmdflags = {
    { 'h', "help",     "Print this help text and quit" },
    { 'v', "version",  "Shows the program's version"   },
    { 'd', "debugger", "Use command-line debugger"     },
};

void cli_interface(cmdline::Result &flags)
{
    if (flags.items.size() < 1)
        throw std::runtime_error("ROM file not specified");
    if (flags.items.size() > 1)
        warning("multiple ROM files specified, first one will be used\n");

    auto rom = open_rom(flags.items[0]);
    auto context = platform::Video::create(platform::Type::SDL);
    platform::Texture tex = context.create_texture(core::SCREEN_WIDTH, core::SCREEN_HEIGHT);
    MainThread mainthread;

    if (!flags.has['d']) {
        emu.on_cpu_error([&](uint8 id, uint16 addr)
        {
            fmt::print(stderr, "The CPU has found an invalid instruction of ID ${:02X} at address ${:02X}. Stopping.\n", id, addr);
            mainthread.end();
        });

        mainthread.run([&]()
        {
            emu.power();
            while (mainthread.running()) {
                emu.run_frame();
                mainthread.new_frame();
            }
        });
    } else {
        mainthread.run([&]()
        {
            emu.power();
            Debugger::CliDebugger clidbg{&emu};
            clidbg.print_instr();
            for (bool quit = false; !quit && mainthread.running(); )
                quit = clidbg.repl();
            mainthread.end();
        });
    }

    rendering_thread(mainthread, context, tex);

    mainthread.join();
}

int main(int argc, char *argv[])
{
#ifdef PLATFORM_WINDOWS
    // windows doesn't have line buffering, and full buffering is just bad
    // for stdout and stderr, so set no buffering to these two.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
#endif

    if (argc < 2) {
        fmt::print(stderr, "Usage: {} [args...] romfile\n", progname);
        cmdline::print_args(cmdflags);
        return 1;
    }

    cmdline::Result flags = cmdline::argparse(argc, argv, cmdflags);
    if (flags.has['h']) {
        fmt::print(stderr, "Usage: {} [args...] romfile\n", progname);
        cmdline::print_args(cmdflags);
        return 0;
    }

    if (flags.has['v']) {
        fmt::print(stderr, "{} version: {}\n", progname, version);
        return 0;
    }

    try {
        cli_interface(flags);
    } catch(const std::runtime_error &err) {
        fmt::print(stderr, "error: {}\n", err.what());
        return 1;
    }

    return 0;
}
