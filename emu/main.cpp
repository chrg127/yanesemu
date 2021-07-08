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
std::mutex running_mutex;
std::condition_variable required_cond;
unsigned frame_pending = 0;
bool wait_for_frame_update = true;

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

    void join()
    {
        th.join();
    }
};

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

void rendering_thread(MainThread &mainthread, Video::Context &ctx, Video::Texture &screen)
{
    while (mainthread.running()) {
        for (SDL_Event ev; SDL_PollEvent(&ev) != 0; ) {
            switch (ev.type) {
            case SDL_QUIT:
                mainthread.end();
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    ctx.resize(ev.window.data1, ev.window.data2);
            }
        }

        mainthread.run_on_frame_pending([&]() {
            ctx.update_texture(screen, emu.get_screen());
        });

        ctx.use_texture(screen);
        ctx.draw();
    }
}

int cli_interface(const Util::ArgResult &flags)
{
    if (!open_rom_with_flags(flags))
        return 1;

    auto context = Video::Context::create(Video::Context::Type::OPENGL);
    if (!context) {
        error("can't initialize video\n");
        return 1;
    }

    Video::Texture tex = context->create_texture(Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT);

    emu.power();

    MainThread mainthread;
    mainthread.run([&]()
    {
        while (mainthread.running()) {
            emu.run_frame();
            mainthread.new_frame();
        }
    });

    rendering_thread(mainthread, context.value(), tex);

    mainthread.join();

    return 0;
}

int debugger_interface(const Util::ArgResult &flags)
{
    if (!open_rom_with_flags(flags))
        return 1;

    auto context = Video::Context::create(Video::Context::Type::OPENGL);
    if (!context) {
        error("can't initialize video\n");
        return 1;
    }

    Video::Texture tex = context->create_texture(Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT);

    emu.power();

    MainThread mainthread;
    Debugger::CliDebugger clidbg{&emu};

    mainthread.run([&]()
    {
        bool quit = false;

        clidbg.print_instr();
        while (!quit && mainthread.running())
            quit = clidbg.repl();
        mainthread.end();
    });
    rendering_thread(mainthread, context.value(), tex);

    mainthread.join();

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

    Util::seed();
    if (flags.has['d'])
        return debugger_interface(flags);
    return cli_interface(flags);
}

