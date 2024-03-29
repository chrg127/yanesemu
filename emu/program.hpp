#pragma once

#include <string_view>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <emu/backend/backend.hpp>
#include <emu/util/conf.hpp>
#include <emu/util/common.hpp>
#include <emu/util/locked.hpp>
#include <emu/util/cmdline.hpp>

class Program {
    std::unique_ptr<backend::Backend> video;
    u32 screen;
    std::thread emulator_thread;

    std::mutex frame_mutex;
    std::condition_variable required_cond;
    unsigned frame_pending = 0;
    bool wait_for_frame_update = true;
    std::span<const u8> video_data;

    enum State { Running, Exiting, };
    util::Locked<State> state;

    void render_loop();

public:
    void start_video(std::string_view rom_name, cmdline::Result &flags);
    void use_config(const conf::Data &conf);
    void set_window_scale(int size);
    void stop();

    void run(auto &&fn)             { emulator_thread = std::thread(fn); }
    void start()                    { render_loop(); }
    bool running()                  { return state.access<bool>([](auto s) { return s == State::Running; }); }

    input::Keys poll_input();
    void video_frame(std::span<const u8> data);
};

extern Program program;
