#include "program.hpp"

#include <filesystem>
#include <fmt/core.h>
#include <emu/version.hpp>
#include <emu/core/const.hpp>
#include <emu/core/emulator.hpp>

Program program;

void Program::start_video(std::string_view rom_name, cmdline::Result &flags)
{
    video = platform::Video::create(
        flags.has['n'] ? platform::Type::NoVideo : platform::Type::SDL,
        core::SCREEN_WIDTH,
        core::SCREEN_HEIGHT
    );
    std::filesystem::path rompath{rom_name};
    auto basename = rompath.stem().c_str();
    auto title = fmt::format("{}{} - {}", progname, (flags.has['d'] ? " (debugger)" : ""), basename);
    video.set_title(title);
    screen = video.create_texture(core::SCREEN_WIDTH, core::SCREEN_HEIGHT);
}

void Program::use_config(const conf::Configuration &conf)
{
    video.map_keys(conf);
}

void Program::set_window_scale(int size)
{
    video.resize(core::SCREEN_WIDTH*size, core::SCREEN_HEIGHT*size);
}

void Program::hold_button(input::Button button, bool hold)
{
    video.hold_button(button, hold);
}

bool Program::poll_input(input::Button button)
{
    return video.is_pressed(button);
}

void Program::render_loop()
{
    auto on_pending = [&](auto &&fn) {
        std::unique_lock<std::mutex> lock{frame_mutex};
        if (frame_pending != 0) {
            frame_pending = 0;
            fn();
        }
        required_cond.notify_one();
    };

    while (running()) {
        video.poll();
        if (video.has_quit())
            stop();
        video.clear();
        on_pending([&]() { video.update_texture(screen, video_data); });
        video.draw_texture(screen, 0, 0);
        video.swap();
    }
    emulator_thread.join();
}

void Program::video_frame(u32 *data)
{
    std::unique_lock<std::mutex> lock{frame_mutex};
    frame_pending += 1;
    video_data = data;
    do {
        if (wait_for_frame_update)
            required_cond.wait(lock);
    } while (frame_pending != 0 && wait_for_frame_update);
}

void Program::stop()
{
    std::lock_guard<std::mutex> frame_lock(frame_mutex);
    state.access([&](State &s) {
        if (s == State::Running) {
            s = State::Exiting;
            wait_for_frame_update = false;
            frame_pending = 0;
            required_cond.notify_one();
        }
    });
}
