#include <stdexcept>
#include <fmt/core.h>
#include <emu/version.hpp>
#include <emu/program.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/core/emulator.hpp>
#include <emu/debugger/clidebugger.hpp>
#include <emu/util/cmdline.hpp>
#include <emu/util/debug.hpp>
#include <emu/util/mappedfile.hpp>
#include <emu/util/conf.hpp>
#include <emu/util/string.hpp>

static const std::vector<cmdline::Argument> cmdflags = {
    { 'h', "help",     "Print this help text and quit" },
    { 'v', "version",  "Shows the program's version"   },
    { 'd', "debugger", "Use command-line debugger"     },
    { 'n', "no-video", "Start without a window"        },
    { 's', "window-size", "Specify window size (1, 2, 3, 4)", cmdline::ParamType::Single, "2" },
};

static const conf::ValidConfig valid_conf = {
    { "AKey",      conf::Value("Key_z")     },
    { "BKey",      conf::Value("Key_x")     },
    { "UpKey",     conf::Value("Key_Up")    },
    { "DownKey",   conf::Value("Key_Down")  },
    { "LeftKey",   conf::Value("Key_Left")  },
    { "RightKey",  conf::Value("Key_Right") },
    { "StartKey",  conf::Value("Key_s")     },
    { "SelectKey", conf::Value("Key_a")     },
};

static conf::Data config;



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
    if (!core::emulator.insert_rom(cart.value()))
        throw std::runtime_error(fmt::format("mapper {} not supported", cart.value().mapper));
    return std::move(romfile.value());
}

void cli_interface(cmdline::Result &flags)
{
    if (flags.items.empty())
        throw std::runtime_error("ROM file not specified");
    if (flags.items.size() > 1)
        warning("multiple ROM files specified, first one will be used\n");

    auto name = flags.items[0];
    auto rom = open_rom(name);
    program.start_video(name, flags);

    int window_size = 2;
    if (flags.has('s')) {
        if (auto opt = str::to_num(flags.params['s']);
            opt && (opt.value() == 1 || opt.value() == 2 || opt.value() == 3 || opt.value() == 4)) {
                window_size = opt.value();
        } else
            throw std::runtime_error("Invalid value for viewport size (valid values: 1 2 3 4)");
    }
    program.set_window_scale(window_size);

    program.use_config(config);
    core::emulator.power();
    if (!flags.has('d')) {
        core::emulator.on_cpu_error([&](u8 id, u16 addr) {
            fmt::print(stderr, "The CPU has found an invalid instruction of ID ${:02X} at address ${:04X}. Stopping.\n", id, addr);
            core::emulator.stop();
            program.stop();
        });
        program.run([&]() {
            while (program.running())
                core::emulator.run_frame();
        });
    } else {
        program.run([&]() {
            debugger::CliDebugger debugger;
            debugger.print_instr();
            for (bool quit = false; !quit && program.running(); )
                quit = debugger.repl();
            program.stop();
        });
    }

    program.start();
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
        fmt::print(stderr, "Usage: {} [args...] romfile\n", progname);
        cmdline::print_args(cmdflags);
        return 1;
    }

    cmdline::Result flags = cmdline::parse(argc, argv, cmdflags);
    if (flags.has('h')) {
        fmt::print(stderr, "Usage: {} [args...] romfile\n", progname);
        cmdline::print_args(cmdflags);
        return 0;
    }

    if (flags.has('v')) {
        fmt::print(stderr, "{} version: {}\n", progname, version);
        return 0;
    }

    auto conf = conf::parse_or_create("yanesemu.conf", valid_conf, [](std::string_view msg) {
        fmt::print(stderr, "{}\n", msg);
    });
    if (!conf)
        return 1;
    config = std::move(conf.value());

    try {
        cli_interface(flags);
    } catch(const std::runtime_error &err) {
        fmt::print(stderr, "error: {}\n", err.what());
        return 1;
    }

    return 0;
}
