#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <emu/platform/video.hpp>
#include <emu/platform/input.hpp>
#include <emu/util/conf.hpp>
#include <emu/util/uint.hpp>
#include <emu/util/locked.hpp>
#include <emu/util/cmdline.hpp>

class Program {
    platform::Video video;
    platform::Texture screen;
    std::thread emulator_thread;

    std::mutex frame_mutex;
    std::condition_variable required_cond;
    unsigned frame_pending = 0;
    bool wait_for_frame_update = true;
    u32 *video_data = nullptr;

    enum State { Running, Exiting, };
    util::Locked<State> state;

    void render_loop();

public:
    void start_video(cmdline::Result &flags);
    void use_config(const conf::Configuration &conf);
    void set_window_scale(int size);
    void hold_button(input::Button button, bool hold);
    void stop();

    void run(auto &&fn)             { emulator_thread = std::thread(fn); }
    void start()                    { render_loop(); }
    bool running()                  { return state.access<bool>([](auto s) { return s == State::Running; }); }

    bool poll_input(input::Button button);
    void video_frame(u32 *data);
};

extern Program program;
