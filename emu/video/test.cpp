#include <algorithm>
#include <array>
#include <fmt/core.h>
#include <emu/util/uint.hpp>
#include <emu/video/video.hpp>

std::array<uint8, 640*480*4> buf;

int main()
{
    auto context = video::Context::create(video::Type::SDL);
    context->set_title("test");
    auto arrow = context->create_texture("arrow.png");
    auto screen = context->create_texture(640, 480);
    std::fill(buf.begin(), buf.end(), 0xff);
    context->update_texture(screen, buf);

    for (bool running = true; running; ) {
        context->poll();
        if (context->has_quit())
            running = false;
        context->clear();
        context->draw_texture(screen, 0, 0);
        context->draw_texture(arrow, 0, 0);
        context->swap();
    }
    return 0;
}
