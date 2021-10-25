#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <emu/platform/video.hpp>
#include <emu/platform/input.hpp>
#include <emu/util/conf.hpp>
#include <emu/util/uint.hpp>

class Program {
    platform::Video video;
    platform::Texture screen;
    std::thread emulator_thread;
    std::mutex frame_mutex, running_mutex;
    std::condition_variable required_cond;
    unsigned frame_pending = 0;
    bool wait_for_frame_update = true;
    u32 *video_data = nullptr;

    enum State {
        RUNNING,
        EXITING,
    } state = State::RUNNING;

    void render_loop();

public:
    void start_video(bool debug_mode);
    void set_window_scale(int size);
    void use_config(const conf::Configuration &conf);
    void start();

    bool poll_input(input::Button button);
    void video_frame(u32 *data);

    bool running();
    void stop();

    void run(auto &&fn) { emulator_thread = std::thread(fn); }
};

extern Program program;
