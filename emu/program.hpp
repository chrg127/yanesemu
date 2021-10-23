#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <emu/platform/video.hpp>
#include <emu/platform/input.hpp>
#include <emu/util/conf.hpp>
#include <emu/util/uint.hpp>

class Program {
    // std::thread emulator_thread;
    platform::Video video;
    platform::Texture screen;

    enum State {
        RUNNING,
        EXITING,
    } state = State::RUNNING;

public:
    void start_video(bool debug_mode);
    void set_window_scale(int size);
    void use_config(const conf::Configuration &conf);
    void render_loop();

    bool poll_input(input::Button button);
    void video_frame(u32 *data);

    bool running() { return state == State::RUNNING; }
    void stop() { state = State::EXITING; }
};

extern Program program;
