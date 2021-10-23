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

void Program::render_loop()
{
    while (running()) {
        video.poll();
        if (video.has_quit())
            stop();
        video.clear();
        core::emulator.run_frame();
        video.draw_texture(screen, 0, 0);
        video.swap();
    }
}

bool Program::poll_input(input::Button button)
{
    return video.is_pressed(button);
}

void Program::video_frame(u32 *data)
{
    video.update_texture(screen, data);
}
