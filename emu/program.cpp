#include "program.hpp"

#include <emu/version.hpp>
#include <emu/core/const.hpp>
#include <emu/core/emulator.hpp>

Program program;

void Program::start_video(bool debug_mode)
{
    video = platform::Video::create(platform::Type::SDL, core::SCREEN_WIDTH, core::SCREEN_HEIGHT);
    video.set_title(std::string(progname) + (debug_mode ? " (debugger)" : ""));
    screen = video.create_texture(core::SCREEN_WIDTH, core::SCREEN_HEIGHT);
}

void Program::set_window_scale(int size)
{
    video.resize(core::SCREEN_WIDTH*size, core::SCREEN_HEIGHT*size);
}

void Program::use_config(const conf::Configuration &conf)
{
    video.map_keys(conf);
}

void Program::start()
{
    render_loop();
}

bool Program::poll_input(input::Button button)
{
    return video.is_pressed(button);
}

void Program::render_loop()
{
    auto on_pending = [&](auto &&fn) {
        std::unique_lock<std::mutex> lock{frame_mutex};
        if (frame_pending == 1) {
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

bool Program::running()
{
    std::lock_guard<std::mutex> lock(running_mutex);
    return state == State::RUNNING;
}

void Program::stop()
{
    std::lock_guard<std::mutex> running_lock(running_mutex);
    std::lock_guard<std::mutex> frame_lock(frame_mutex);
    if (state != State::RUNNING)
        return;
    state = State::EXITING;
    wait_for_frame_update = false;
    frame_pending = 0;
    required_cond.notify_one();
}
