#include <chrono>

#include <SDL2/SDL.h>
#include <emu/core/emulator.hpp>
#include <emu/core/cartridge.hpp>
#include <emu/video/video.hpp>
#include <emu/util/file.hpp>

using namespace Util;

static Core::Emulator emu;

void run()
{
    namespace ch = std::chrono;
    auto t1 = ch::high_resolution_clock::now();
    emu.run_frame();
    auto t2 = ch::high_resolution_clock::now();
    auto ms = ch::duration_cast<ch::milliseconds>(t2-t1);
    fmt::print("{} ms\n", ms.count());
}

int main()
{
    auto file_opt = File::open("testrom/test.nes", Access::READ);
    if (!file_opt)
        return 1;
    auto cart_opt = Core::parse_cartridge(file_opt.value());
    if (!cart_opt)
        return 1;
    auto ctx = Video::Context::create(Video::Context::Type::OPENGL, "Window");
    if (!ctx)
        return 1;
    Video::Canvas screen = ctx->create_canvas(Core::SCREEN_WIDTH, Core::SCREEN_HEIGHT);
    emu.insert_rom(std::move(cart_opt.value()));
    emu.set_screen(&screen);
    emu.power();
    SDL_Event ev;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                    ctx->resize(ev.window.data1, ev.window.data2);
            }
        }
        run();
        ctx->update_canvas(screen);
        ctx->use_texture(screen);
        ctx->draw();
    }
    return 0;
}
