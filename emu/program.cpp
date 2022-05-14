#include "program.hpp"

#include <filesystem>
#include <ctime>
#include <fmt/core.h>
#include <emu/version.hpp>
#include <emu/core/const.hpp>
#include <emu/core/emulator.hpp>

Program program;

void Program::start_video(std::string_view rom_name, cmdline::Result &flags)
{
    std::filesystem::path rompath{rom_name};
    auto basename = rompath.stem().c_str();
    auto title = fmt::format("{}{} - {}", progname, (flags.has('d') ? " (debugger)" : ""), basename);
    video = backend::create(
        flags.has('n') ? backend::Type::NoVideo : backend::Type::SDL_OpenGL,
        title,
        core::SCREEN_WIDTH,
        core::SCREEN_HEIGHT
    );
    screen = video->create_texture(core::SCREEN_WIDTH, core::SCREEN_HEIGHT, backend::TextureFormat::RGBA);
}

void Program::use_config(const conf::Data &conf)
{
    using namespace std::literals;
    using namespace input;
    for (auto p : { std::pair{"AKey"s,      Button::A},
                    std::pair{"BKey"s,      Button::B},
                    std::pair{"UpKey"s,     Button::Up},
                    std::pair{"DownKey"s,   Button::Down},
                    std::pair{"LeftKey"s,   Button::Left},
                    std::pair{"RightKey"s,  Button::Right},
                    std::pair{"StartKey"s,  Button::Start},
                    std::pair{"SelectKey"s, Button::Select} }) {
        auto entry = util::map_lookup(conf, p.first);
        auto s = entry.value().as<std::string>();
        s.erase(s.begin(), s.begin() + 4);
        video->map_key(s, p.second);
    }
}

void Program::set_window_scale(int size)
{
    video->resize(core::SCREEN_WIDTH*size, core::SCREEN_HEIGHT*size);
}

bool Program::poll_input(input::Button button)
{
    return video->is_pressed(button);
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
        video->poll();
        if (video->has_quit())
            stop();
        video->clear();
        on_pending([&]() { video->update_texture(screen, video_data); });
        video->draw_texture(screen, 0, 0);
        video->draw();
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

    // static unsigned frame_counter = 0;
    // static u64 prev = 0, curr;
    // frame_counter++;
    // curr = ::time(nullptr);
    // if (curr != prev) {
    //     prev = curr;
    //     fmt::print("{} fps\n", frame_counter);
    //     frame_counter = 0;
    // }
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
